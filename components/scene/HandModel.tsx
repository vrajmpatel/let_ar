import { useRef, useEffect } from "react";
import { useFrame } from "@react-three/fiber";
import * as THREE from "three";

export interface Quaternion {
    w: number;
    x: number;
    y: number;
    z: number;
}

export interface LinearAccel {
    x: number;
    y: number;
    z: number;
}

interface HandModelProps {
    position?: [number, number, number];
    scale?: number | [number, number, number];
    quaternion?: Quaternion | null;
    linearAccel?: LinearAccel | null;
}

function Finger({ position, scale }: { position: [number, number, number], scale: [number, number, number] }) {
    return (
        <mesh position={position} castShadow receiveShadow>
            <boxGeometry args={scale} />
            <meshStandardMaterial
                color="#e4e4e7"
                roughness={0.3}
                metalness={0.8}
            />
        </mesh>
    );
}

function Joint({ position }: { position: [number, number, number] }) {
    return (
        <mesh position={position} castShadow receiveShadow>
            <sphereGeometry args={[0.35, 32, 32]} />
            <meshStandardMaterial
                color="#3b82f6"
                roughness={0.2}
                metalness={0.9}
                emissive="#1d4ed8"
                emissiveIntensity={0.2}
            />
        </mesh>
    );
}

export function HandModel({ quaternion, linearAccel, ...props }: HandModelProps) {
    const groupRef = useRef<THREE.Group>(null);
    const targetQuaternion = useRef(new THREE.Quaternion());
    const hasQuaternion = quaternion !== null && quaternion !== undefined;

    // Update target quaternion when prop changes
    useEffect(() => {
        if (quaternion) {
            // Convert from sensor quaternion to Three.js quaternion
            // Note: You may need to adjust the axis mapping depending on sensor orientation
            targetQuaternion.current.set(
                quaternion.x,
                quaternion.y,
                quaternion.z,
                quaternion.w
            );
        }
    }, [quaternion]);



    useFrame((state) => {
        if (groupRef.current) {
            if (hasQuaternion) {
                // Smoothly interpolate to the target quaternion from the sensor
                groupRef.current.quaternion.slerp(targetQuaternion.current, 0.15);
            } else {
                // Gentle floating animation when no quaternion data
                groupRef.current.position.y = Math.sin(state.clock.elapsedTime * 0.5) * 0.1;
                groupRef.current.rotation.y = Math.sin(state.clock.elapsedTime * 0.2) * 0.1;
            }
        }
    });

    return (
        <group ref={groupRef} {...props}>
            {/* Palm */}
            <mesh position={[0, 0, 0]} castShadow receiveShadow>
                <boxGeometry args={[2, 2.4, 0.5]} />
                <meshStandardMaterial
                    color="#a1a1aa"
                    roughness={0.3}
                    metalness={0.8}
                />
            </mesh>

            {/* Wrist Joint */}
            <Joint position={[0, -1.5, 0]} />

            {/* Fingers (Thumb, Index, Middle, Ring, Pinky) */}

            {/* Thumb */}
            <group position={[1.2, -0.5, 0]} rotation={[0, 0, -0.5]}>
                <Joint position={[-0.2, 0, 0]} />
                <Finger position={[0.3, 0.5, 0]} scale={[0.6, 1.2, 0.5]} />
                <Joint position={[0.3, 1.1, 0]} />
                <Finger position={[0.3, 1.6, 0]} scale={[0.5, 0.8, 0.5]} />
            </group>

            {/* Index */}
            <group position={[0.8, 1.2, 0]}>
                <Joint position={[0, 0, 0]} />
                <Finger position={[0, 0.6, 0]} scale={[0.5, 1.2, 0.5]} />
                <Joint position={[0, 1.2, 0]} />
                <Finger position={[0, 1.7, 0]} scale={[0.45, 0.9, 0.5]} />
            </group>

            {/* Middle */}
            <group position={[0.25, 1.2, 0]}>
                <Joint position={[0, 0, 0]} />
                <Finger position={[0, 0.7, 0]} scale={[0.5, 1.4, 0.5]} />
                <Joint position={[0, 1.4, 0]} />
                <Finger position={[0, 2.0, 0]} scale={[0.45, 1.0, 0.5]} />
            </group>

            {/* Ring */}
            <group position={[-0.3, 1.2, 0]}>
                <Joint position={[0, 0, 0]} />
                <Finger position={[0, 0.65, 0]} scale={[0.5, 1.3, 0.5]} />
                <Joint position={[0, 1.3, 0]} />
                <Finger position={[0, 1.9, 0]} scale={[0.45, 1.0, 0.5]} />
            </group>

            {/* Pinky */}
            <group position={[-0.85, 1.1, 0]}>
                <Joint position={[0, 0, 0]} />
                <Finger position={[0, 0.5, 0]} scale={[0.45, 1.0, 0.5]} />
                <Joint position={[0, 1.0, 0]} />
                <Finger position={[0, 1.5, 0]} scale={[0.4, 0.8, 0.5]} />
            </group>

        </group>
    );
}
