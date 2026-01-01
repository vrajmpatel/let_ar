'use client';

import { useRef } from "react";
import { Canvas, useFrame, useThree } from "@react-three/fiber";
import * as THREE from "three";
import { Text } from "@react-three/drei";

interface AxisArrowProps {
    direction: [number, number, number];
    color: string;
    label: string;
}

function AxisArrow({ direction, color, label }: AxisArrowProps) {
    const length = 0.6;
    const headLength = 0.12;
    const headWidth = 0.06;

    // Calculate arrow end position
    const endPos: [number, number, number] = [
        direction[0] * length,
        direction[1] * length,
        direction[2] * length
    ];

    // Label position (slightly beyond arrow tip)
    const labelPos: [number, number, number] = [
        direction[0] * (length + 0.18),
        direction[1] * (length + 0.18),
        direction[2] * (length + 0.18)
    ];

    // Calculate rotation for cylinder and cone
    const getRotation = (): [number, number, number] => {
        if (direction[1] === 1) return [0, 0, 0];
        if (direction[1] === -1) return [Math.PI, 0, 0];
        if (direction[0] === 1) return [0, 0, -Math.PI / 2];
        if (direction[0] === -1) return [0, 0, Math.PI / 2];
        if (direction[2] === 1) return [Math.PI / 2, 0, 0];
        if (direction[2] === -1) return [-Math.PI / 2, 0, 0];
        return [0, 0, 0];
    };

    const rotation = getRotation();

    return (
        <group>
            {/* Arrow shaft */}
            <mesh
                position={[
                    direction[0] * (length / 2),
                    direction[1] * (length / 2),
                    direction[2] * (length / 2)
                ]}
                rotation={rotation}
            >
                <cylinderGeometry args={[0.018, 0.018, length, 8]} />
                <meshBasicMaterial color={color} />
            </mesh>

            {/* Arrow head */}
            <mesh position={endPos} rotation={rotation}>
                <coneGeometry args={[headWidth, headLength, 8]} />
                <meshBasicMaterial color={color} />
            </mesh>

            {/* Axis label */}
            <Text
                position={labelPos}
                fontSize={0.15}
                color={color}
                anchorX="center"
                anchorY="middle"
                fontWeight="bold"
            >
                {label}
            </Text>
        </group>
    );
}

/**
 * Inner UCS gizmo that syncs with the hand quaternion
 */
function UCSGizmoInner({
    handQuatRef
}: {
    handQuatRef: React.MutableRefObject<THREE.Quaternion>
}) {
    const gizmoGroupRef = useRef<THREE.Group>(null);

    useFrame(() => {
        if (gizmoGroupRef.current) {
            // Apply hand rotation directly to show the hand's current orientation
            gizmoGroupRef.current.quaternion.copy(handQuatRef.current);
        }
    });

    return (
        <>
            <group ref={gizmoGroupRef}>
                {/* Origin sphere */}
                <mesh>
                    <sphereGeometry args={[0.05, 16, 16]} />
                    <meshBasicMaterial color="#ffffff" />
                </mesh>

                {/* X Axis - Red */}
                <AxisArrow direction={[1, 0, 0]} color="#ef4444" label="X" />

                {/* Y Axis - Green */}
                <AxisArrow direction={[0, 1, 0]} color="#22c55e" label="Y" />

                {/* Z Axis - Blue */}
                <AxisArrow direction={[0, 0, 1]} color="#3b82f6" label="Z" />

                {/* Plane indicators */}
                <mesh position={[0.2, 0.2, 0]}>
                    <planeGeometry args={[0.25, 0.25]} />
                    <meshBasicMaterial color="#fbbf24" transparent opacity={0.25} side={THREE.DoubleSide} />
                </mesh>

                <mesh position={[0.2, 0, 0.2]} rotation={[Math.PI / 2, 0, 0]}>
                    <planeGeometry args={[0.25, 0.25]} />
                    <meshBasicMaterial color="#a855f7" transparent opacity={0.25} side={THREE.DoubleSide} />
                </mesh>

                <mesh position={[0, 0.2, 0.2]} rotation={[0, Math.PI / 2, 0]}>
                    <planeGeometry args={[0.25, 0.25]} />
                    <meshBasicMaterial color="#06b6d4" transparent opacity={0.25} side={THREE.DoubleSide} />
                </mesh>
            </group>

            {/* UCS Label */}
            <Text
                position={[0, -0.85, 0]}
                fontSize={0.12}
                color="#71717a"
                anchorX="center"
                anchorY="middle"
            >
                UCS
            </Text>
        </>
    );
}

/**
 * Component to place inside the main Canvas that broadcasts camera quaternion
 */
export function UCSCameraBridge({
    quatRef
}: {
    quatRef: React.MutableRefObject<THREE.Quaternion>
}) {
    const { camera } = useThree();

    useFrame(() => {
        quatRef.current.copy(camera.quaternion);
    });

    return null;
}

/**
 * Standalone UCS gizmo overlay with its own canvas
 */
export function UCSGizmoOverlay({
    handQuatRef
}: {
    handQuatRef: React.MutableRefObject<THREE.Quaternion>
}) {
    return (
        <div
            className="absolute bottom-4 right-4 w-[140px] h-[140px] pointer-events-none z-10"
            style={{
                background: 'radial-gradient(circle at center, rgba(0,0,0,0.5) 0%, rgba(0,0,0,0.25) 60%, transparent 100%)',
                borderRadius: '12px',
                border: '1px solid rgba(255,255,255,0.1)'
            }}
        >
            <Canvas
                camera={{
                    position: [0, 0, 2.5],
                    fov: 50,
                    near: 0.1,
                    far: 100
                }}
                dpr={[1, 2]}
                gl={{ antialias: true, alpha: true }}
                frameloop="always"
            >
                <UCSGizmoInner handQuatRef={handQuatRef} />
            </Canvas>
        </div>
    );
}

/**
 * Origin marker placed at the starting position (0, 0, 0)
 * Shows where the hand's initial/base position is
 */
export function OriginMarker() {
    const markerRef = useRef<THREE.Group>(null);
    const ringRef = useRef<THREE.Mesh>(null);
    const pulseRef = useRef(0);

    useFrame((state) => {
        pulseRef.current = Math.sin(state.clock.elapsedTime * 2) * 0.5 + 0.5;

        if (ringRef.current) {
            const scale = 1 + pulseRef.current * 0.15;
            ringRef.current.scale.set(scale, scale, scale);
            (ringRef.current.material as THREE.MeshBasicMaterial).opacity = 0.3 + pulseRef.current * 0.2;
        }
    });

    return (
        <group ref={markerRef} position={[0, 0, 0]}>
            {/* Center point */}
            <mesh>
                <sphereGeometry args={[0.08, 16, 16]} />
                <meshBasicMaterial color="#f97316" />
            </mesh>

            {/* Outer glow ring */}
            <mesh ref={ringRef} rotation={[Math.PI / 2, 0, 0]}>
                <torusGeometry args={[0.25, 0.02, 8, 32]} />
                <meshBasicMaterial color="#f97316" transparent opacity={0.4} />
            </mesh>

            {/* Ground plane indicator */}
            <mesh rotation={[Math.PI / 2, 0, 0]} position={[0, -0.01, 0]}>
                <ringGeometry args={[0.12, 0.35, 32]} />
                <meshBasicMaterial color="#f97316" transparent opacity={0.15} side={THREE.DoubleSide} />
            </mesh>

            {/* Cross-hair lines on ground */}
            <mesh rotation={[Math.PI / 2, 0, 0]}>
                <planeGeometry args={[0.8, 0.015]} />
                <meshBasicMaterial color="#ef4444" transparent opacity={0.4} side={THREE.DoubleSide} />
            </mesh>

            <mesh rotation={[Math.PI / 2, Math.PI / 2, 0]}>
                <planeGeometry args={[0.8, 0.015]} />
                <meshBasicMaterial color="#3b82f6" transparent opacity={0.4} side={THREE.DoubleSide} />
            </mesh>

            {/* Vertical line (Y axis) */}
            <mesh>
                <cylinderGeometry args={[0.008, 0.008, 0.4, 8]} />
                <meshBasicMaterial color="#22c55e" transparent opacity={0.6} />
            </mesh>

            {/* "ORIGIN" label */}
            <Text
                position={[0, -0.35, 0]}
                rotation={[Math.PI / 2, 0, 0]}
                fontSize={0.1}
                color="#a1a1aa"
                anchorX="center"
                anchorY="middle"
            >
                ORIGIN
            </Text>
        </group>
    );
}
