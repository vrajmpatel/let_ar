'use client';

import { Terminal as TerminalIcon, Bluetooth, BluetoothOff, Trash2, Lock, Unlock, Crosshair, X, Download, Upload, Square } from "lucide-react";
import { cn } from "@/lib/utils";
import { useBluetooth, TerminalEntry } from "@/hooks/useBluetooth";
import { useEffect, useRef, useState } from "react";
import { downloadJson } from "@/lib/downloadJson";
import { CALIBRATION_STORAGE_KEY } from "@/lib/CalibrationManager";
import { isIMURecordingV1 } from "@/lib/recording";
import { preprocessRecordingToReplay } from "@/lib/preprocessRecording";
import type { ReplaySessionV1 } from "@/lib/replay";

function EntryIcon({ type }: { type: TerminalEntry['type'] }) {
    switch (type) {
        case 'system':
            return <span className="text-blue-400">▸</span>;
        case 'data':
            return <span className="text-emerald-400">◆</span>;
        case 'error':
            return <span className="text-red-400">✖</span>;
        default:
            return <span className="text-zinc-500">▸</span>;
    }
}

interface TerminalProps {
    onQuaternion?: (q: { w: number; x: number; y: number; z: number }) => void;
    onLinearAccel?: (a: { x: number; y: number; z: number }) => void;
    onMagnetometer?: (m: { x: number; y: number; z: number }) => void;
    onDisconnect?: () => void;
    isPositionLocked?: boolean;
    onTogglePositionLock?: () => void;
    // Calibration props
    isCalibrating?: boolean;
    hasCalibration?: boolean;
    onStartCalibration?: () => void;
    onCancelCalibration?: () => void;
    onClearCalibration?: () => void;
    calibrationProgress?: { current: number; total: number; label: string } | null;
    onAddEntry?: (type: 'system' | 'data' | 'error', message: string) => void;
    onReplayLoaded?: (replay: ReplaySessionV1) => void;
    isReplaying?: boolean;
    onStopReplay?: () => void;
}

export function Terminal({
    onQuaternion,
    onLinearAccel,
    onMagnetometer,
    onDisconnect,
    isPositionLocked = false,
    onTogglePositionLock,
    isCalibrating = false,
    hasCalibration = false,
    onStartCalibration,
    onCancelCalibration,
    onClearCalibration,
    calibrationProgress,
    onReplayLoaded,
    isReplaying = false,
    onStopReplay,
}: TerminalProps) {
    const {
        isConnected,
        isConnecting,
        deviceName,
        entries,
        packetCount,
        connect,
        disconnect,
        clearEntries,
        addEntry,
        exportRecording,
    } = useBluetooth({ onQuaternion, onLinearAccel, onMagnetometer, onDisconnect });

    // Track if we've shown calibration status message (ref avoids setState-in-effect lint)
    const hasShownCalibrationStatusRef = useRef(false);

    // Show calibration status when connected
    useEffect(() => {
        if (isConnected && !hasShownCalibrationStatusRef.current) {
            if (hasCalibration) {
                addEntry('system', '✓ Calibration data loaded from storage. Device is ready.');
            } else {
                addEntry('system', '⚠ No calibration data found. Click "Calibrate" to align IMU axes.');
            }
            hasShownCalibrationStatusRef.current = true;
        }
        if (!isConnected) {
            hasShownCalibrationStatusRef.current = false;
        }
    }, [isConnected, hasCalibration, addEntry]);


    const scrollRef = useRef<HTMLDivElement>(null);
    const uploadInputRef = useRef<HTMLInputElement>(null);

    const getSafeDeviceName = (name: string | null | undefined) => {
        const base = (name || "device").trim();
        return base.length > 0 ? base.replaceAll(/[^\w.-]+/g, "_") : "device";
    };

    const handleDownload = () => {
        const recording = exportRecording();
        if (!recording) {
            addEntry("error", "No recording available yet. Connect to the device first.");
            return;
        }

        let calibration = null;
        try {
            const stored = localStorage.getItem(CALIBRATION_STORAGE_KEY);
            if (stored) calibration = JSON.parse(stored);
        } catch {
            calibration = null;
        }

        const exportData = { ...recording, calibration };
        const stamp = new Date().toISOString().replaceAll(":", "-");
        const filename = `imu_recording_${getSafeDeviceName(recording.deviceName || deviceName)}_${stamp}.json`;
        downloadJson(filename, exportData);
        addEntry("system", `Downloaded recording (${exportData.events.length} events).`);
    };

    const handleUploadClick = () => {
        uploadInputRef.current?.click();
    };

    const handleUploadFile = async (file: File) => {
        const text = await file.text();
        const parsed = JSON.parse(text) as unknown;
        if (!isIMURecordingV1(parsed)) {
            throw new Error("Unsupported JSON format (expected schemaVersion=1 recording).");
        }
        const replay = preprocessRecordingToReplay(parsed, { sourceFileName: file.name });
        onReplayLoaded?.(replay);
        addEntry("system", `Loaded replay from ${file.name} (${replay.frames.length} frames).`);
    };

    // Auto-scroll to bottom when new entries arrive
    useEffect(() => {
        if (scrollRef.current) {
            scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
        }
    }, [entries]);

    return (
        <div className={cn(
            "h-full w-full flex flex-col overflow-hidden",
            "bg-zinc-950/60 backdrop-blur-xl border-l border-white/5 shadow-2xl"
        )}>
            {/* Header */}
            <div className="flex items-center justify-between px-4 py-3 border-b border-white/5 bg-zinc-950/40">
                <div className="flex items-center gap-3">
                    <TerminalIcon className="w-4 h-4 text-muted-foreground" />
                    <span className="text-sm font-medium text-muted-foreground tracking-tight">
                        Terminal Output
                    </span>
                </div>

                {/* Connection status indicator */}
                <div className="flex items-center gap-2">
                    <div className={cn(
                        "w-2 h-2 rounded-full",
                        isConnected ? "bg-emerald-400 shadow-lg shadow-emerald-400/50" :
                            isConnecting ? "bg-amber-400 animate-pulse" : "bg-zinc-600"
                    )} />
                    <span className="text-xs text-muted-foreground">
                        {isConnected ? deviceName : isConnecting ? "Connecting..." : "Disconnected"}
                    </span>
                </div>
            </div>

            {/* Control buttons */}
            <div className="flex items-center gap-2 px-4 py-2 border-b border-white/10 bg-zinc-900/50 flex-wrap">
                {!isConnected ? (
                    <button
                        onClick={connect}
                        disabled={isConnecting}
                        className={cn(
                            "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all",
                            "bg-blue-600/20 border border-blue-500/30 text-blue-400",
                            "hover:bg-blue-600/30 hover:border-blue-500/50",
                            "disabled:opacity-50 disabled:cursor-not-allowed"
                        )}
                    >
                        <Bluetooth className="w-3.5 h-3.5" />
                        {isConnecting ? "Connecting..." : "Connect"}
                    </button>
                ) : (
                    <button
                        onClick={disconnect}
                        className={cn(
                            "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all",
                            "bg-red-600/20 border border-red-500/30 text-red-400",
                            "hover:bg-red-600/30 hover:border-red-500/50"
                        )}
                    >
                        <BluetoothOff className="w-3.5 h-3.5" />
                        Disconnect
                    </button>
                )}

                <button
                    onClick={clearEntries}
                    className={cn(
                        "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all",
                        "bg-zinc-800/50 border border-zinc-700/50 text-zinc-400",
                        "hover:bg-zinc-700/50 hover:text-zinc-300"
                    )}
                >
                    <Trash2 className="w-3.5 h-3.5" />
                    Clear
                </button>

                <button
                    onClick={handleDownload}
                    className={cn(
                        "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all",
                        "bg-zinc-800/50 border border-zinc-700/50 text-zinc-400",
                        "hover:bg-zinc-700/50 hover:text-zinc-300"
                    )}
                    title="Download all recorded events as JSON"
                >
                    <Download className="w-3.5 h-3.5" />
                    Download
                </button>

                <input
                    ref={uploadInputRef}
                    type="file"
                    accept="application/json"
                    className="hidden"
                    onChange={async (e) => {
                        const file = e.target.files?.[0];
                        e.target.value = "";
                        if (!file) return;
                        try {
                            await handleUploadFile(file);
                        } catch (err) {
                            const message = err instanceof Error ? err.message : "Failed to load recording JSON.";
                            addEntry("error", message);
                        }
                    }}
                />

                {!isReplaying ? (
                    <button
                        onClick={handleUploadClick}
                        className={cn(
                            "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all",
                            "bg-zinc-800/50 border border-zinc-700/50 text-zinc-400",
                            "hover:bg-zinc-700/50 hover:text-zinc-300"
                        )}
                        title="Upload a recording JSON to replay"
                    >
                        <Upload className="w-3.5 h-3.5" />
                        Upload
                    </button>
                ) : (
                    <button
                        onClick={onStopReplay}
                        className={cn(
                            "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all",
                            "bg-amber-600/20 border border-amber-500/30 text-amber-400",
                            "hover:bg-amber-600/30 hover:border-amber-500/50"
                        )}
                        title="Stop replay and return to live view"
                    >
                        <Square className="w-3.5 h-3.5" />
                        Stop
                    </button>
                )}

                {/* Calibration Button */}
                {isConnected && (
                    isCalibrating ? (
                        <button
                            onClick={onCancelCalibration}
                            className={cn(
                                "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all",
                                "bg-orange-600/20 border border-orange-500/30 text-orange-400",
                                "hover:bg-orange-600/30 hover:border-orange-500/50 animate-pulse"
                            )}
                            title={calibrationProgress ? `Step ${calibrationProgress.current}/${calibrationProgress.total}: ${calibrationProgress.label}` : "Calibrating..."}
                        >
                            <X className="w-3.5 h-3.5" />
                            Cancel ({calibrationProgress?.label || "..."})
                        </button>
                    ) : (
                        <button
                            onClick={onStartCalibration}
                            className={cn(
                                "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all",
                                hasCalibration
                                    ? "bg-emerald-600/20 border border-emerald-500/30 text-emerald-400 hover:bg-emerald-600/30"
                                    : "bg-purple-600/20 border border-purple-500/30 text-purple-400 hover:bg-purple-600/30"
                            )}
                            title={hasCalibration ? "Recalibrate device" : "Calibrate device axes"}
                        >
                            <Crosshair className="w-3.5 h-3.5" />
                            {hasCalibration ? "Recalibrate" : "Calibrate"}
                        </button>
                    )
                )}

                {/* Position Lock Toggle */}
                <button
                    onClick={onTogglePositionLock}
                    className={cn(
                        "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all ml-auto",
                        isPositionLocked
                            ? "bg-amber-600/20 border border-amber-500/30 text-amber-400 hover:bg-amber-600/30"
                            : "bg-zinc-800/50 border border-zinc-700/50 text-zinc-400 hover:bg-zinc-700/50 hover:text-zinc-300"
                    )}
                    title={isPositionLocked ? "Position locked (orientation only)" : "Position tracking active"}
                >
                    {isPositionLocked ? <Lock className="w-3.5 h-3.5" /> : <Unlock className="w-3.5 h-3.5" />}
                    {isPositionLocked ? "Locked" : "Unlocked"}
                </button>
            </div>

            {/* Terminal output */}
            <div
                ref={scrollRef}
                className="flex-1 p-4 font-mono text-xs space-y-1 overflow-y-auto text-muted-foreground/80 scrollbar-thin scrollbar-thumb-white/10 scrollbar-track-transparent"
            >
                {entries.length === 0 ? (
                    <div className="flex items-center gap-2 text-zinc-600">
                        <span className="animate-pulse">▸</span>
                        <span>Waiting for input...</span>
                    </div>
                ) : (
                    entries.map((entry) => (
                        <div
                            key={entry.id}
                            className={cn(
                                "flex gap-2 leading-relaxed",
                                entry.type === 'error' && "text-red-400",
                                entry.type === 'data' && "text-emerald-300/90",
                                entry.type === 'system' && "text-zinc-400"
                            )}
                        >
                            <EntryIcon type={entry.type} />
                            <span className="flex-1 break-all">{entry.message}</span>
                        </div>
                    ))
                )}
            </div>

            {/* Footer with stats */}
            {isConnected && (
                <div className="px-4 py-2 border-t border-white/5 bg-zinc-900/30 text-xs text-zinc-500 flex items-center justify-between">
                    <span>Packets: {packetCount}</span>
                    <span className="flex items-center gap-1">
                        <span className="w-1.5 h-1.5 rounded-full bg-emerald-400 animate-pulse" />
                        Live
                    </span>
                </div>
            )}
        </div>
    );
}
