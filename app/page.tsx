import { Navbar } from "@/components/layout/Navbar";
import { Footer } from "@/components/layout/Footer";
import { SceneContainer } from "@/components/scene/SceneContainer";
import { Terminal } from "@/components/terminal/Terminal";

export default function Home() {
  return (
    <main className="flex h-full w-full bg-background text-foreground overflow-hidden">
      {/* 3D Viewport - Takes up majority of space */}
      <section className="relative flex-1 h-full min-w-0">
        <Navbar />
        <SceneContainer />
        <Footer />
      </section>

      {/* Terminal Sidebar - Fixed width on desktop, hidden/collapsible on mobile (future) */}
      <aside className="w-80 h-full border-l border-white/5 bg-zinc-950 shadow-2xl z-40 hidden md:block">
        <Terminal />
      </aside>
    </main>
  );
}
