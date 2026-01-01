'use client';

import { useState, useCallback, useRef, useEffect } from "react";
import { Navbar } from "@/components/layout/Navbar";
import { Footer } from "@/components/layout/Footer";
import { SceneContainer } from "@/components/scene/SceneContainer";
import { Terminal } from "@/components/terminal/Terminal";
import { Quaternion, LinearAccel } from "@/components/scene/HandModel";
import { EKFTracker } from "@/lib/EKFTracker";
import { useCalibration } from "@/hooks/useCalibration";
import type { ReplaySessionV1 } from "@/lib/replay";

export default function Home() {
  const [quaternion, setQuaternion] = useState<Quaternion | null>(null);
  const [linearAccel, setLinearAccel] = useState<LinearAccel | null>(null);
  const [position, setPosition] = useState<{ x: number, y: number, z: number }>({ x: 0, y: 0, z: 0 });
  const [isPositionLocked, setIsPositionLocked] = useState(true);
  const [replay, setReplay] = useState<ReplaySessionV1 | null>(null);
  const [isReplaying, setIsReplaying] = useState(false);
  const [replayProgress, setReplayProgress] = useState<{ currentFrame: number; totalFrames: number } | null>(null);
  const [battery, setBattery] = useState<{ percent: number; milliVolts: number } | null>(null);
  const [isDeviceConnected, setIsDeviceConnected] = useState(false);

  const ekfTrackerRef = useRef<EKFTracker | null>(null);
  const lastQuaternionRef = useRef<Quaternion | null>(null);

  // Initialize EKF tracker
  useEffect(() => {
    ekfTrackerRef.current = new EKFTracker();
  }, []);

  // Initialize calibration with terminal message callback
  const {
    isCalibrating,
    hasCalibration,
    currentStep,
    startCalibration,
    cancelCalibration,
    clearCalibration,
    addAccelSample,
    transformAcceleration,
    getStepProgress,
  } = useCalibration({
    onCalibrationMessage: useCallback((message: string, type: 'system' | 'data' | 'error') => {
      // Calibration messages are logged via useCalibration's internal callbacks
      // The terminal will display them through the useBluetooth entries
      console.log(`[Calibration ${type}]:`, message);
    }, []),
  });

  const handleQuaternion = useCallback((q: Quaternion) => {
    if (isReplaying) return;
    setQuaternion(q);
    lastQuaternionRef.current = q;
  }, [isReplaying]);

  const handleLinearAccel = useCallback((a: LinearAccel) => {
    // If calibrating, feed samples to calibration manager
    if (isCalibrating) {
      addAccelSample(a);
      return; // Don't update position during calibration
    }

    if (isReplaying) {
      return;
    }

    // Skip all processing when locked (record-only mode)
    if (isPositionLocked) {
      return;
    }

    // Apply calibration transform if available
    const calibratedAccel = transformAcceleration(a);
    setLinearAccel(calibratedAccel);

    // Update EKF tracker if we have orientation data
    if (ekfTrackerRef.current && lastQuaternionRef.current) {
      const newPos = ekfTrackerRef.current.predict(calibratedAccel, lastQuaternionRef.current);
      setPosition(newPos);
    }
  }, [isPositionLocked, isReplaying, isCalibrating, addAccelSample, transformAcceleration]);

  const handleMagnetometer = useCallback((m: { x: number; y: number; z: number }) => {
    // Skip magnetometer updates when locked or calibrating
    if (isReplaying || isPositionLocked || isCalibrating) {
      return;
    }

    // Feed magnetometer data to EKF for heading correction
    if (ekfTrackerRef.current) {
      ekfTrackerRef.current.updateMagnetometer(m);
    }
  }, [isReplaying, isPositionLocked, isCalibrating]);

  const handleBattery = useCallback((b: { percent: number; milliVolts: number }) => {
    setBattery(b);
    // Mark as connected when we receive battery data
    setIsDeviceConnected(true);
  }, []);

  const handleConnect = useCallback(() => {
    setIsDeviceConnected(true);
  }, []);

  const handleDisconnect = useCallback(() => {
    if (ekfTrackerRef.current) {
      ekfTrackerRef.current.reset();
      setPosition({ x: 0, y: 0, z: 0 });
    }
    // Cancel any in-progress calibration on disconnect
    if (isCalibrating) {
      cancelCalibration();
    }
    // Clear battery and connection state on disconnect
    setBattery(null);
    setIsDeviceConnected(false);
    setIsReplaying(false);
    setReplay(null);
  }, [isCalibrating, cancelCalibration]);

  const handleTogglePositionLock = useCallback(() => {
    setIsPositionLocked(prev => !prev);
  }, []);

  const handleReplayLoaded = useCallback((session: ReplaySessionV1) => {
    setReplay(session);
    setIsReplaying(true);
    setReplayProgress({ currentFrame: 1, totalFrames: session.frames.length });
    setQuaternion(null);
    setLinearAccel(null);
    setPosition({ x: 0, y: 0, z: 0 });
    if (ekfTrackerRef.current) {
      ekfTrackerRef.current.reset();
    }
  }, []);

  const handleStopReplay = useCallback(() => {
    setIsReplaying(false);
    setReplay(null);
    setReplayProgress(null);
    setQuaternion(null);
    setLinearAccel(null);
    setPosition({ x: 0, y: 0, z: 0 });
  }, []);

  // Get calibration progress for display
  const calibrationProgress = getStepProgress();

  return (
    <main className="flex h-full w-full bg-background text-foreground overflow-hidden">
      {/* 3D Viewport - Takes up majority of space */}
      <section className="relative flex-1 h-full min-w-0">
        <Navbar
          playback={{
            isActive: isReplaying,
            currentFrame: replayProgress?.currentFrame ?? 0,
            totalFrames: replayProgress?.totalFrames ?? 0,
          }}
          battery={battery}
          isConnected={isDeviceConnected}
        />
        <SceneContainer
          quaternion={quaternion}
          linearAccel={linearAccel}
          position={position}
          isCalibrated={hasCalibration}
          replay={replay && isReplaying ? { frames: replay.frames, isPlaying: true, onEnded: handleStopReplay } : null}
          onReplayProgress={setReplayProgress}
        />
        <Footer />
      </section>

      {/* Terminal Sidebar - Fixed width on desktop, hidden/collapsible on mobile (future) */}
      <aside className="w-80 h-full border-l border-white/5 bg-zinc-950 shadow-2xl z-40 hidden md:block">
        <Terminal
          onQuaternion={handleQuaternion}
          onLinearAccel={handleLinearAccel}
          onMagnetometer={handleMagnetometer}
          onBattery={handleBattery}
          onConnect={handleConnect}
          onDisconnect={handleDisconnect}
          isPositionLocked={isPositionLocked}
          onTogglePositionLock={handleTogglePositionLock}
          isCalibrating={isCalibrating}
          hasCalibration={hasCalibration}
          onStartCalibration={startCalibration}
          onCancelCalibration={cancelCalibration}
          onClearCalibration={clearCalibration}
          calibrationProgress={calibrationProgress}
          onReplayLoaded={handleReplayLoaded}
          isReplaying={isReplaying}
          onStopReplay={handleStopReplay}
        />
      </aside>
    </main>
  );
}
