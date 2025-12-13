'use client';

import { Canvas, useFrame, useThree } from "@react-three/fiber";
import { Environment, OrbitControls } from "@react-three/drei";
import { HandModel, Quaternion, LinearAccel } from "./HandModel";
import { CoordinateAxes } from "./CoordinateAxes";
import { Suspense, useRef, useEffect } from "react";
import * as THREE from "three";
import type { ReplayFrameV1 } from "@/lib/replay";

const CALIBRATED_NEUTRAL_ROTATION = new THREE.Quaternion(Math.SQRT1_2, Math.SQRT1_2, 0, 0);

interface SceneContainerProps {
    quaternion?: Quaternion | null;
    linearAccel?: LinearAccel | null;
    position?: { x: number; y: number; z: number };
    isCalibrated?: boolean;
    onReplayProgress?: (progress: { currentFrame: number; totalFrames: number }) => void;
    replay?: {
        frames: ReplayFrameV1[];
        isPlaying: boolean;
        speed?: number;
        loop?: boolean;
        onEnded?: () => void;
    } | null;
}

interface AdaptiveCameraProps {
    handPosition: { x: number; y: number; z: number };
    handPositionRef?: React.RefObject<{ x: number; y: number; z: number }>;
    orbitControlsRef: React.RefObject<typeof OrbitControls>;
}

/**
 * Adaptive camera component that automatically adjusts zoom to keep the hand visible.
 * Monitors the hand's position and smoothly zooms out when the hand approaches viewport edges.
 */
function AdaptiveCamera({ handPosition, handPositionRef, orbitControlsRef }: AdaptiveCameraProps) {
    const { camera, gl } = useThree();
    const targetDistance = useRef(5);
    const currentDistance = useRef(5);

    // Configuration
    const MARGIN = 0.15; // 15% margin from edges before triggering zoom
    const MIN_DISTANCE = 3;
    const MAX_DISTANCE = 25;
    const ZOOM_OUT_SPEED = 0.08; // Speed of zooming out (faster response)
    const ZOOM_IN_SPEED = 0.02; // Speed of zooming back in (slower, smoother)
    const DISTANCE_PER_UNIT = 2.0; // How much to increase distance per unit outside bounds

    useFrame(() => {
        const pos = handPositionRef?.current ?? handPosition;
        // Get the hand's world position
        const handWorldPos = new THREE.Vector3(pos.x, pos.y, pos.z);

        // Project hand position to normalized device coordinates (NDC)
        const ndcPos = handWorldPos.clone().project(camera);

        // NDC ranges from -1 to 1, convert to 0-1 for easier margin calculation
        const screenX = (ndcPos.x + 1) / 2;
        const screenY = (ndcPos.y + 1) / 2;

        // Calculate how far outside the safe zone the hand is
        let outsideAmount = 0;

        // Check horizontal bounds
        if (screenX < MARGIN) {
            outsideAmount = Math.max(outsideAmount, (MARGIN - screenX) / MARGIN);
        } else if (screenX > 1 - MARGIN) {
            outsideAmount = Math.max(outsideAmount, (screenX - (1 - MARGIN)) / MARGIN);
        }

        // Check vertical bounds
        if (screenY < MARGIN) {
            outsideAmount = Math.max(outsideAmount, (MARGIN - screenY) / MARGIN);
        } else if (screenY > 1 - MARGIN) {
            outsideAmount = Math.max(outsideAmount, (screenY - (1 - MARGIN)) / MARGIN);
        }

        // Also check if hand is behind camera (z > 1 in NDC means behind)
        if (ndcPos.z > 1) {
            outsideAmount = Math.max(outsideAmount, 1);
        }

        // Calculate distance from origin to determine if hand has moved far
        const distanceFromOrigin = handWorldPos.length();

        // Determine target distance based on hand position
        if (outsideAmount > 0) {
            // Hand is near or outside edges - zoom out proportionally
            const additionalDistance = outsideAmount * DISTANCE_PER_UNIT + distanceFromOrigin * 0.5;
            targetDistance.current = Math.min(MAX_DISTANCE, currentDistance.current + additionalDistance);
        } else {
            // Hand is comfortably in view - calculate ideal distance based on position
            const idealDistance = Math.max(MIN_DISTANCE, 5 + distanceFromOrigin * 0.3);
            targetDistance.current = Math.min(MAX_DISTANCE, idealDistance);
        }

        // Smoothly interpolate current distance to target
        const lerpSpeed = targetDistance.current > currentDistance.current ? ZOOM_OUT_SPEED : ZOOM_IN_SPEED;
        currentDistance.current = THREE.MathUtils.lerp(currentDistance.current, targetDistance.current, lerpSpeed);

        // Apply the distance to camera only if OrbitControls is available
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        const controls = orbitControlsRef.current as any;
        if (controls) {
            // Get current camera direction
            const direction = new THREE.Vector3();
            camera.getWorldDirection(direction);

            // Update the OrbitControls target and camera position to maintain view direction
            const targetPos = controls.target as THREE.Vector3;
            const offsetDir = camera.position.clone().sub(targetPos).normalize();
            const newCamPos = targetPos.clone().add(offsetDir.multiplyScalar(currentDistance.current));

            camera.position.lerp(newCamPos, lerpSpeed);

            // Update the controls
            controls.update();
        }
    });

    return null;
}

function ReplayController({
    frames,
    isPlaying,
    speed = 1,
    loop = false,
    onEnded,
    onProgress,
    isCalibrated = false,
    handRef,
    handPositionRef,
}: {
    frames: ReplayFrameV1[];
    isPlaying: boolean;
    speed?: number;
    loop?: boolean;
    onEnded?: () => void;
    onProgress?: (progress: { currentFrame: number; totalFrames: number }) => void;
    isCalibrated?: boolean;
    handRef: React.RefObject<THREE.Group>;
    handPositionRef: React.RefObject<{ x: number; y: number; z: number }>;
}) {
    const startPerfRef = useRef<number | null>(null);
    const indexRef = useRef(0);
    const endedRef = useRef(false);
    const onEndedRef = useRef(onEnded);
    const onProgressRef = useRef(onProgress);
    const lastProgressPerfRef = useRef(0);
    const lastProgressIndexRef = useRef(-1);

    useEffect(() => {
        onEndedRef.current = onEnded;
    }, [onEnded]);

    useEffect(() => {
        onProgressRef.current = onProgress;
    }, [onProgress]);

    useEffect(() => {
        if (isPlaying && frames.length > 0) {
            startPerfRef.current = performance.now();
            indexRef.current = 0;
            endedRef.current = false;
            lastProgressPerfRef.current = 0;
            lastProgressIndexRef.current = -1;
            onProgressRef.current?.({ currentFrame: 1, totalFrames: frames.length });
        }
    }, [isPlaying, frames]);

    const tmpQ0 = useRef(new THREE.Quaternion());
    const tmpQ1 = useRef(new THREE.Quaternion());

    useFrame(() => {
        const hand = handRef.current;
        if (!hand || !isPlaying || frames.length === 0 || startPerfRef.current === null) return;

        const durationMs = frames[frames.length - 1].tMs;
        const now = performance.now();
        let tMs = (now - startPerfRef.current) * speed;

        if (tMs >= durationMs) {
            if (loop) {
                startPerfRef.current = performance.now();
                indexRef.current = 0;
                tMs = 0;
            } else {
                tMs = durationMs;
                if (!endedRef.current) {
                    endedRef.current = true;
                    onEndedRef.current?.();
                }
            }
        }

        while (indexRef.current + 1 < frames.length && frames[indexRef.current + 1].tMs <= tMs) {
            indexRef.current++;
        }

        if (
            onProgressRef.current &&
            indexRef.current !== lastProgressIndexRef.current &&
            now - lastProgressPerfRef.current >= 80
        ) {
            lastProgressIndexRef.current = indexRef.current;
            lastProgressPerfRef.current = now;
            onProgressRef.current({ currentFrame: indexRef.current + 1, totalFrames: frames.length });
        }

        const a = frames[indexRef.current];
        const b = frames[Math.min(indexRef.current + 1, frames.length - 1)];
        const denom = b.tMs - a.tMs;
        const alpha = denom <= 0 ? 0 : Math.min(1, Math.max(0, (tMs - a.tMs) / denom));

        const x = a.position.x + (b.position.x - a.position.x) * alpha;
        const y = a.position.y + (b.position.y - a.position.y) * alpha;
        const z = a.position.z + (b.position.z - a.position.z) * alpha;

        hand.position.set(x, y, z);
        handPositionRef.current.x = x;
        handPositionRef.current.y = y;
        handPositionRef.current.z = z;

        tmpQ0.current.set(a.quaternion.x, a.quaternion.y, a.quaternion.z, a.quaternion.w);
        tmpQ1.current.set(b.quaternion.x, b.quaternion.y, b.quaternion.z, b.quaternion.w);
        if (isCalibrated) {
            tmpQ0.current.multiply(CALIBRATED_NEUTRAL_ROTATION);
            tmpQ1.current.multiply(CALIBRATED_NEUTRAL_ROTATION);
        }
        hand.quaternion.copy(tmpQ0.current).slerp(tmpQ1.current, alpha);
    });

    return null;
}

export function SceneContainer({ quaternion, linearAccel, position, replay, onReplayProgress, isCalibrated }: SceneContainerProps) {
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    const orbitControlsRef = useRef<any>(null);
    const handPos = position || { x: 0, y: 0, z: 0 };
    const handRef = useRef<THREE.Group>(null!);
    const replayHandPosRef = useRef({ x: 0, y: 0, z: 0 });

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
                        ref={orbitControlsRef}
                        enablePan={false}
                        minPolarAngle={Math.PI / 4}
                        maxPolarAngle={Math.PI / 1.5}
                        minDistance={3}
                        maxDistance={25}
                    />
                    <Environment preset="city" />

                    {/* Adaptive camera that automatically zooms to keep hand visible */}
                    <AdaptiveCamera
                        handPosition={handPos}
                        handPositionRef={replay?.isPlaying ? replayHandPosRef : undefined}
                        orbitControlsRef={orbitControlsRef}
                    />

                    {replay?.frames && replay.frames.length > 0 && (
                        <ReplayController
                            frames={replay.frames}
                            isPlaying={replay.isPlaying}
                            speed={replay.speed}
                            loop={replay.loop}
                            onEnded={replay.onEnded}
                            onProgress={onReplayProgress}
                            isCalibrated={Boolean(isCalibrated)}
                            handRef={handRef}
                            handPositionRef={replayHandPosRef}
                        />
                    )}

                    <HandModel
                        ref={handRef}
                        position={position ? [position.x, position.y, position.z] : [0, 0, 0]}
                        scale={0.8}
                        quaternion={replay?.isPlaying ? null : quaternion}
                        linearAccel={linearAccel}
                        isCalibrated={!replay?.isPlaying && Boolean(isCalibrated)}
                        mode={replay?.isPlaying ? "replay" : "live"}
                    />

                    {/* User Coordinate System Legend - bottom left corner */}
                    <CoordinateAxes />
                </Suspense>
            </Canvas>
        </div>
    );
}
