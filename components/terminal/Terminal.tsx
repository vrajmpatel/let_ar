import { Terminal as TerminalIcon } from "lucide-react";
import { cn } from "@/lib/utils";

export function Terminal() {
    return (
        <div className={cn(
            "h-full w-full flex flex-col overflow-hidden",
            "bg-zinc-950/60 backdrop-blur-xl border-l border-white/5 shadow-2xl"
        )}>
            <div className="flex items-center gap-3 px-6 py-4 border-b border-white/5 bg-zinc-950/20">
                <TerminalIcon className="w-4 h-4 text-muted-foreground" />
                <span className="text-sm font-medium text-muted-foreground tracking-tight">Terminal Output</span>
            </div>

            <div className="flex-1 p-6 font-mono text-xs space-y-2 overflow-y-auto text-muted-foreground/80 scrollbar-thin scrollbar-thumb-white/10 scrollbar-track-transparent">
                <div className="flex gap-2">
                    <span className="text-green-500">➜</span>
                    <span>Initializing system...</span>
                </div>
                <div className="flex gap-2">
                    <span className="text-blue-500">➜</span>
                    <span>Loading 3D environment...</span>
                </div>
                <div className="flex gap-2">
                    <span className="text-zinc-600">➜</span>
                    <span className="animate-pulse">Waiting for input...</span>
                </div>
            </div>
        </div>
    );
}
