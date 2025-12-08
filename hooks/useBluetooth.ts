'use client';

import { useState, useCallback, useRef } from 'react';

// Nordic UART Service UUIDs
const UART_SERVICE_UUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const UART_TX_CHARACTERISTIC_UUID = '6e400003-b5a3-f393-e0a9-e50e24dcca9e'; // TX from device (notifications)

export interface TerminalEntry {
    id: number;
    timestamp: Date;
    type: 'system' | 'data' | 'error';
    message: string;
    quaternion?: { w: number; x: number; y: number; z: number };
}

export interface BluetoothState {
    isConnected: boolean;
    isConnecting: boolean;
    deviceName: string | null;
    error: string | null;
}

/**
 * Parse a 20-byte quaternion packet from the QuatStream device
 * Packet format:
 *   Byte 0:     '!'  (start marker)
 *   Byte 1:     'Q'  (quaternion identifier)
 *   Bytes 2-5:  w (float, 4 bytes, little-endian)
 *   Bytes 6-9:  x (float, 4 bytes, little-endian)
 *   Bytes 10-13: y (float, 4 bytes, little-endian)
 *   Bytes 14-17: z (float, 4 bytes, little-endian)
 *   Byte 18:    checksum (~sum of bytes 0-17)
 *   Byte 19:    newline '\n'
 */
function parseQuaternionPacket(data: DataView): { w: number; x: number; y: number; z: number } | null {
    if (data.byteLength < 20) {
        return null;
    }

    // Verify start marker and identifier
    const startMarker = data.getUint8(0);
    const identifier = data.getUint8(1);

    if (startMarker !== 0x21 || identifier !== 0x51) { // '!' and 'Q'
        return null;
    }

    // Verify checksum
    let checksum = 0;
    for (let i = 0; i < 18; i++) {
        checksum += data.getUint8(i);
    }
    checksum = (~checksum) & 0xFF;

    if (data.getUint8(18) !== checksum) {
        return null;
    }

    // Parse quaternion floats (little-endian)
    const w = data.getFloat32(2, true);
    const x = data.getFloat32(6, true);
    const y = data.getFloat32(10, true);
    const z = data.getFloat32(14, true);

    return { w, x, y, z };
}

/**
 * Parse a 16-byte magnetometer packet from the QuatStream device
 * Packet format:
 *   Byte 0:     '!'  (start marker)
 *   Byte 1:     'M'  (magnetometer identifier)
 *   Bytes 2-5:  mag_x (float, 4 bytes, little-endian) - micro Tesla (µT)
 *   Bytes 6-9:  mag_y (float, 4 bytes, little-endian) - micro Tesla (µT)
 *   Bytes 10-13: mag_z (float, 4 bytes, little-endian) - micro Tesla (µT)
 *   Byte 14:    checksum (~sum of bytes 0-13)
 *   Byte 15:    newline '\n'
 */
function parseMagnetometerPacket(data: DataView): { x: number; y: number; z: number } | null {
    if (data.byteLength < 16) {
        return null;
    }

    // Verify start marker and identifier
    const startMarker = data.getUint8(0);
    const identifier = data.getUint8(1);

    if (startMarker !== 0x21 || identifier !== 0x4D) { // '!' and 'M'
        return null;
    }

    // Verify checksum
    let checksum = 0;
    for (let i = 0; i < 14; i++) {
        checksum += data.getUint8(i);
    }
    checksum = (~checksum) & 0xFF;

    if (data.getUint8(14) !== checksum) {
        return null;
    }

    // Parse magnetometer floats (little-endian) - values in µT
    const x = data.getFloat32(2, true);
    const y = data.getFloat32(6, true);
    const z = data.getFloat32(10, true);

    return { x, y, z };
}

export interface UseBluetoothOptions {
    onQuaternion?: (q: { w: number; x: number; y: number; z: number }) => void;
}

export function useBluetooth(options: UseBluetoothOptions = {}) {
    const { onQuaternion } = options;

    const [state, setState] = useState<BluetoothState>({
        isConnected: false,
        isConnecting: false,
        deviceName: null,
        error: null,
    });

    const [entries, setEntries] = useState<TerminalEntry[]>([
        { id: 0, timestamp: new Date(), type: 'system', message: 'Initializing system...' },
        { id: 1, timestamp: new Date(), type: 'system', message: 'Ready to connect to QuatStream device.' },
    ]);

    const deviceRef = useRef<BluetoothDevice | null>(null);
    const characteristicRef = useRef<BluetoothRemoteGATTCharacteristic | null>(null);
    const entryIdRef = useRef(2);

    const addEntry = useCallback((type: TerminalEntry['type'], message: string, quaternion?: TerminalEntry['quaternion']) => {
        const entry: TerminalEntry = {
            id: entryIdRef.current++,
            timestamp: new Date(),
            type,
            message,
            quaternion,
        };
        setEntries(prev => {
            // Keep only last 100 entries to prevent memory issues
            const newEntries = [...prev, entry];
            if (newEntries.length > 100) {
                return newEntries.slice(-100);
            }
            return newEntries;
        });
    }, []);

    const handleNotification = useCallback((event: Event) => {
        const characteristic = event.target as BluetoothRemoteGATTCharacteristic;
        const value = characteristic.value;

        if (!value) return;

        // BLE UART may combine multiple packets into a single notification
        // We need to scan through the buffer and parse each packet
        const buffer = new Uint8Array(value.buffer, value.byteOffset, value.byteLength);
        let offset = 0;

        while (offset < buffer.length) {
            // Look for start marker '!'
            if (buffer[offset] !== 0x21) {
                offset++;
                continue;
            }

            // Check if we have enough bytes for the identifier
            if (offset + 1 >= buffer.length) break;

            const identifier = buffer[offset + 1];

            // Try quaternion packet ('Q' = 0x51, 20 bytes)
            if (identifier === 0x51 && offset + 20 <= buffer.length) {
                const packetView = new DataView(buffer.buffer, buffer.byteOffset + offset, 20);
                const quaternion = parseQuaternionPacket(packetView);

                if (quaternion) {
                    const message = `Q: w=${quaternion.w.toFixed(4)} x=${quaternion.x.toFixed(4)} y=${quaternion.y.toFixed(4)} z=${quaternion.z.toFixed(4)}`;
                    addEntry('data', message, quaternion);

                    if (onQuaternion) {
                        onQuaternion(quaternion);
                    }
                    offset += 20;
                    continue;
                }
            }

            // Try magnetometer packet ('M' = 0x4D, 16 bytes)
            if (identifier === 0x4D && offset + 16 <= buffer.length) {
                const packetView = new DataView(buffer.buffer, buffer.byteOffset + offset, 16);
                const magnetometer = parseMagnetometerPacket(packetView);

                if (magnetometer) {
                    const message = `M: x=${magnetometer.x.toFixed(2)} y=${magnetometer.y.toFixed(2)} z=${magnetometer.z.toFixed(2)} µT`;
                    addEntry('data', message);
                    offset += 16;
                    continue;
                }
            }

            // Unknown packet type or parsing failed, skip this byte
            offset++;
        }
    }, [addEntry, onQuaternion]);

    const connect = useCallback(async () => {
        // Check if Web Bluetooth is supported
        if (!navigator.bluetooth) {
            setState(prev => ({ ...prev, error: 'Web Bluetooth is not supported in this browser' }));
            addEntry('error', 'Web Bluetooth is not supported. Please use Chrome or Edge.');
            return;
        }

        setState(prev => ({ ...prev, isConnecting: true, error: null }));
        addEntry('system', 'Scanning for QuatStream device...');

        try {
            // Request the device
            const device = await navigator.bluetooth.requestDevice({
                filters: [{ namePrefix: 'QuatStream' }],
                optionalServices: [UART_SERVICE_UUID],
            });

            deviceRef.current = device;
            addEntry('system', `Found device: ${device.name || 'Unknown'}`);

            // Handle disconnection
            device.addEventListener('gattserverdisconnected', () => {
                setState(prev => ({ ...prev, isConnected: false, deviceName: null }));
                addEntry('system', 'Device disconnected.');
                characteristicRef.current = null;
            });

            // Connect to GATT server
            addEntry('system', 'Connecting to GATT server...');
            const server = await device.gatt?.connect();

            if (!server) {
                throw new Error('Failed to connect to GATT server');
            }

            // Get the UART service
            addEntry('system', 'Getting UART service...');
            const service = await server.getPrimaryService(UART_SERVICE_UUID);

            // Get the TX characteristic (we receive notifications on this)
            addEntry('system', 'Setting up notifications...');
            const txCharacteristic = await service.getCharacteristic(UART_TX_CHARACTERISTIC_UUID);
            characteristicRef.current = txCharacteristic;

            // Start notifications
            await txCharacteristic.startNotifications();
            txCharacteristic.addEventListener('characteristicvaluechanged', handleNotification);

            setState({
                isConnected: true,
                isConnecting: false,
                deviceName: device.name || 'QuatStream',
                error: null,
            });

            addEntry('system', `Connected to ${device.name}! Receiving quaternion data...`);

        } catch (error) {
            const message = error instanceof Error ? error.message : 'Failed to connect';
            setState(prev => ({ ...prev, isConnecting: false, error: message }));
            addEntry('error', `Connection failed: ${message}`);
        }
    }, [addEntry, handleNotification]);

    const disconnect = useCallback(async () => {
        if (characteristicRef.current) {
            try {
                await characteristicRef.current.stopNotifications();
                characteristicRef.current.removeEventListener('characteristicvaluechanged', handleNotification);
            } catch {
                // Ignore errors during cleanup
            }
        }

        if (deviceRef.current?.gatt?.connected) {
            deviceRef.current.gatt.disconnect();
        }

        characteristicRef.current = null;
        deviceRef.current = null;

        setState({
            isConnected: false,
            isConnecting: false,
            deviceName: null,
            error: null,
        });

        addEntry('system', 'Disconnected from device.');
    }, [addEntry, handleNotification]);

    const clearEntries = useCallback(() => {
        setEntries([]);
        entryIdRef.current = 0;
    }, []);

    return {
        ...state,
        entries,
        connect,
        disconnect,
        clearEntries,
    };
}
