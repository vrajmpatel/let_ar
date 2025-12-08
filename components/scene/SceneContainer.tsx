'use client';

import { Canvas } from "@react-three/fiber";
import { Environment, OrbitControls } from "@react-three/drei";
import { HandModel } from "./HandModel";
import { Suspense } from "react";

export function SceneContainer() {
    return (
        <div className="w-full h-full bg-gradient-to-b from-zinc-900 to-black relative">
            <div className="absolute inset-0 bg-[radial-gradient(circle_at_center,_var(--tw-gradient-stops))] from-blue-500/5 via-transparent to-transparent opacity-40" />

            <Canvas
                camera={{ position: [0, 2, 5], fov: 50 }}
                dpr={[1, 2]}
                gl={{ antialias: true, alpha: true }}
            >
                <Suspense fallback={null}>
                    <ambientLight intensity={0.5} />
                    <spotLight position={[10, 10, 10]} angle={0.15} penumbra={1} intensity={1} castShadow />
                    <pointLight position={[-10, -10, -10]} intensity={0.5} />

                    <OrbitControls
                        enablePan={false}
                        minPolarAngle={Math.PI / 4}
                        maxPolarAngle={Math.PI / 1.5}
                    />
                    <Environment preset="city" />

                    <HandModel position={[0, 0, 0]} scale={0.8} />
                </Suspense>
            </Canvas>
        </div>
    );
}
