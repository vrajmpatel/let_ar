'use client';

import { useState, useCallback } from "react";
import { Navbar } from "@/components/layout/Navbar";
import { Footer } from "@/components/layout/Footer";
import { SceneContainer } from "@/components/scene/SceneContainer";
import { Terminal } from "@/components/terminal/Terminal";
import { Quaternion, LinearAccel } from "@/components/scene/HandModel";

export default function Home() {
  const [quaternion, setQuaternion] = useState<Quaternion | null>(null);
  const [linearAccel, setLinearAccel] = useState<LinearAccel | null>(null);

  const handleQuaternion = useCallback((q: Quaternion) => {
    setQuaternion(q);
  }, []);

  const handleLinearAccel = useCallback((a: LinearAccel) => {
    setLinearAccel(a);
  }, []);

  return (
    <main className="flex h-full w-full bg-background text-foreground overflow-hidden">
      {/* 3D Viewport - Takes up majority of space */}
      <section className="relative flex-1 h-full min-w-0">
        <Navbar />
        <SceneContainer quaternion={quaternion} linearAccel={linearAccel} />
        <Footer />
      </section>

      {/* Terminal Sidebar - Fixed width on desktop, hidden/collapsible on mobile (future) */}
      <aside className="w-80 h-full border-l border-white/5 bg-zinc-950 shadow-2xl z-40 hidden md:block">
        <Terminal onQuaternion={handleQuaternion} onLinearAccel={handleLinearAccel} />
      </aside>
    </main>
  );
}
