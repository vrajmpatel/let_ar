'use client';

import { useRef } from "react";
import { useFrame } from "@react-three/fiber";
import * as THREE from "three";
import { Text } from "@react-three/drei";

interface AxisArrowProps {
    direction: [number, number, number];
    color: string;
    label: string;
}

function AxisArrow({ direction, color, label }: AxisArrowProps) {
    const length = 0.8;
    const headLength = 0.15;
    const headWidth = 0.08;

    // Calculate arrow end position
    const endPos: [number, number, number] = [
        direction[0] * length,
        direction[1] * length,
        direction[2] * length
    ];

    // Label position (slightly beyond arrow tip)
    const labelPos: [number, number, number] = [
        direction[0] * (length + 0.2),
        direction[1] * (length + 0.2),
        direction[2] * (length + 0.2)
    ];

    return (
        <group>
            {/* Arrow shaft using a thin cylinder */}
            <mesh
                position={[
                    direction[0] * (length / 2),
                    direction[1] * (length / 2),
                    direction[2] * (length / 2)
                ]}
                rotation={[
                    direction[1] === 1 ? 0 : direction[1] === -1 ? Math.PI : direction[2] === 1 ? Math.PI / 2 : direction[2] === -1 ? -Math.PI / 2 : Math.PI / 2,
                    0,
                    direction[0] === 1 ? -Math.PI / 2 : direction[0] === -1 ? Math.PI / 2 : 0
                ]}
            >
                <cylinderGeometry args={[0.02, 0.02, length, 8]} />
                <meshBasicMaterial color={color} />
            </mesh>

            {/* Arrow head (cone) */}
            <mesh
                position={endPos}
                rotation={[
                    direction[1] === 1 ? 0 : direction[1] === -1 ? Math.PI : direction[2] === 1 ? Math.PI / 2 : direction[2] === -1 ? -Math.PI / 2 : Math.PI / 2,
                    0,
                    direction[0] === 1 ? -Math.PI / 2 : direction[0] === -1 ? Math.PI / 2 : 0
                ]}
            >
                <coneGeometry args={[headWidth, headLength, 8]} />
                <meshBasicMaterial color={color} />
            </mesh>

            {/* Axis label */}
            <Text
                position={labelPos}
                fontSize={0.18}
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

export function CoordinateAxes() {
    const groupRef = useRef<THREE.Group>(null);

    // Keep the axes oriented towards camera for better visibility
    useFrame(({ camera }) => {
        if (groupRef.current) {
            // Make the UCS legend face the camera (billboard effect on Y axis only)
            groupRef.current.quaternion.copy(camera.quaternion);
        }
    });

    return (
        <group position={[-3.5, -2.5, 0]}>
            {/* Static axes group - doesn't rotate with camera */}
            <group ref={groupRef}>
                {/* Origin sphere */}
                <mesh>
                    <sphereGeometry args={[0.06, 16, 16]} />
                    <meshBasicMaterial color="#ffffff" />
                </mesh>
            </group>

            {/* Axes remain in world orientation */}
            <group>
                {/* X Axis - Red */}
                <AxisArrow
                    direction={[1, 0, 0]}
                    color="#ef4444"
                    label="X"
                />

                {/* Y Axis - Green */}
                <AxisArrow
                    direction={[0, 1, 0]}
                    color="#22c55e"
                    label="Y"
                />

                {/* Z Axis - Blue */}
                <AxisArrow
                    direction={[0, 0, 1]}
                    color="#3b82f6"
                    label="Z"
                />
            </group>

            {/* UCS Label */}
            <Text
                position={[0, -0.5, 0]}
                fontSize={0.14}
                color="#a1a1aa"
                anchorX="center"
                anchorY="middle"
            >
                UCS
            </Text>

            {/* Plane indicators */}
            {/* XY Plane indicator */}
            <mesh position={[0.3, 0.3, 0]} rotation={[0, 0, 0]}>
                <planeGeometry args={[0.4, 0.4]} />
                <meshBasicMaterial color="#fbbf24" transparent opacity={0.2} side={THREE.DoubleSide} />
            </mesh>

            {/* XZ Plane indicator */}
            <mesh position={[0.3, 0, 0.3]} rotation={[Math.PI / 2, 0, 0]}>
                <planeGeometry args={[0.4, 0.4]} />
                <meshBasicMaterial color="#a855f7" transparent opacity={0.2} side={THREE.DoubleSide} />
            </mesh>

            {/* YZ Plane indicator */}
            <mesh position={[0, 0.3, 0.3]} rotation={[0, Math.PI / 2, 0]}>
                <planeGeometry args={[0.4, 0.4]} />
                <meshBasicMaterial color="#06b6d4" transparent opacity={0.2} side={THREE.DoubleSide} />
            </mesh>
        </group>
    );
}
