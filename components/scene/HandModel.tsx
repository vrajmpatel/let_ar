import { useRef } from "react";
import { useFrame } from "@react-three/fiber";
import * as THREE from "three";

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

export function HandModel(props: JSX.IntrinsicElements['group']) {
    const groupRef = useRef<THREE.Group>(null);

    useFrame((state) => {
        if (groupRef.current) {
            // Gentle floating animation
            groupRef.current.position.y = Math.sin(state.clock.elapsedTime * 0.5) * 0.1;
            groupRef.current.rotation.y = Math.sin(state.clock.elapsedTime * 0.2) * 0.1;
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
