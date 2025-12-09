'use client';

import { useState, useCallback, useRef, useEffect } from "react";
import { Navbar } from "@/components/layout/Navbar";
import { Footer } from "@/components/layout/Footer";
import { SceneContainer } from "@/components/scene/SceneContainer";
import { Terminal } from "@/components/terminal/Terminal";
import { Quaternion, LinearAccel } from "@/components/scene/HandModel";
import { EKFTracker } from "@/lib/EKFTracker";

export default function Home() {
  const [quaternion, setQuaternion] = useState<Quaternion | null>(null);
  const [linearAccel, setLinearAccel] = useState<LinearAccel | null>(null);
  const [position, setPosition] = useState<{ x: number, y: number, z: number }>({ x: 0, y: 0, z: 0 });

  const ekfTrackerRef = useRef<EKFTracker | null>(null);
  const lastQuaternionRef = useRef<Quaternion | null>(null);

  // Initialize EKF tracker
  useEffect(() => {
    ekfTrackerRef.current = new EKFTracker();
  }, []);

  const handleQuaternion = useCallback((q: Quaternion) => {
    setQuaternion(q);
    lastQuaternionRef.current = q;
  }, []);

  const handleLinearAccel = useCallback((a: LinearAccel) => {
    setLinearAccel(a);

    // Update EKF tracker if we have orientation data
    if (ekfTrackerRef.current && lastQuaternionRef.current) {
      const newPos = ekfTrackerRef.current.predict(a, lastQuaternionRef.current);
      setPosition(newPos);
    }
  }, []);

  const handleMagnetometer = useCallback((m: { x: number; y: number; z: number }) => {
    // Feed magnetometer data to EKF for heading correction
    if (ekfTrackerRef.current) {
      ekfTrackerRef.current.updateMagnetometer(m);
    }
  }, []);

  const handleDisconnect = useCallback(() => {
    if (ekfTrackerRef.current) {
      ekfTrackerRef.current.reset();
      setPosition({ x: 0, y: 0, z: 0 });
    }
  }, []);

  return (
    <main className="flex h-full w-full bg-background text-foreground overflow-hidden">
      {/* 3D Viewport - Takes up majority of space */}
      <section className="relative flex-1 h-full min-w-0">
        <Navbar />
        <SceneContainer quaternion={quaternion} linearAccel={linearAccel} position={position} />
        <Footer />
      </section>

      {/* Terminal Sidebar - Fixed width on desktop, hidden/collapsible on mobile (future) */}
      <aside className="w-80 h-full border-l border-white/5 bg-zinc-950 shadow-2xl z-40 hidden md:block">
        <Terminal
          onQuaternion={handleQuaternion}
          onLinearAccel={handleLinearAccel}
          onMagnetometer={handleMagnetometer}
          onDisconnect={handleDisconnect}
        />
      </aside>
    </main>
  );
}

