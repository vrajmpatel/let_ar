import { Hand } from "lucide-react";
import { cn } from "@/lib/utils";

export function Navbar() {
    return (
        <header className="absolute top-0 left-0 right-0 z-50 p-4">
            <div className={cn(
                "bg-zinc-950/20 backdrop-blur-md border border-white/5 rounded-2xl px-6 py-4 flex items-center justify-between shadow-2xl safe-area"
            )}>
                <div className="flex items-center gap-3">
                    <div className="p-2 bg-blue-500/10 rounded-lg border border-blue-500/20">
                        <Hand className="w-5 h-5 text-blue-500" />
                    </div>
                    <h1 className="text-lg font-medium tracking-tight bg-gradient-to-br from-white to-white/60 bg-clip-text text-transparent">
                        Hand Control
                    </h1>
                </div>

                <div className="flex items-center gap-4">
                    <div className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-green-500/5 border border-green-500/10">
                        <div className="w-2 h-2 rounded-full bg-green-500 animate-pulse" />
                        <span className="text-xs font-mono text-green-500/80">System Online</span>
                    </div>
                </div>
            </div>
        </header>
    );
}
