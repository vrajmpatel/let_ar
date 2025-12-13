import { Maximize2, Play } from "lucide-react";
import { cn } from "@/lib/utils";

export function Navbar({
    playback,
}: {
    playback?: {
        isActive: boolean;
        currentFrame: number;
        totalFrames: number;
    };
}) {
    const playbackIsAvailable = (playback?.totalFrames ?? 0) > 0;
    const playbackIsActive = playbackIsAvailable && Boolean(playback?.isActive);
    const playbackText = playbackIsAvailable
        ? `${Math.min(playback?.currentFrame ?? 0, playback?.totalFrames ?? 0)}/${playback?.totalFrames ?? 0}`
        : "--/--";

    return (
        <header className="absolute top-0 left-0 right-0 z-50 p-4">
            <div className={cn(
                "bg-zinc-950/20 backdrop-blur-md border border-white/5 rounded-2xl px-6 py-4 flex items-center justify-between shadow-2xl safe-area"
            )}>
                <div className="flex items-center gap-3">
                    <div className="p-2 bg-blue-500/10 rounded-lg border border-blue-500/20">
                        <Maximize2 className="w-5 h-5 text-blue-500" />
                    </div>
                    <h1 className="text-lg font-medium tracking-tight bg-gradient-to-br from-white to-white/60 bg-clip-text text-transparent">
                        Low-Cost Motion Capture
                    </h1>
                </div>

                <div className="flex items-center gap-4">
                    <div
                        className={cn(
                            "flex items-center gap-2 px-3 py-1.5 rounded-full border",
                            playbackIsActive
                                ? "bg-purple-500/5 border-purple-500/10"
                                : "bg-zinc-500/5 border-zinc-500/10 opacity-60"
                        )}
                        title={playbackIsAvailable ? "Playback in progress" : "Upload a JSON recording to start playback"}
                    >
                        <div
                            className={cn(
                                "w-2 h-2 rounded-full",
                                playbackIsActive ? "bg-purple-400 animate-pulse" : "bg-zinc-500"
                            )}
                        />
                        <Play className={cn("w-3.5 h-3.5", playbackIsActive ? "text-purple-400" : "text-zinc-500")} />
                        <span className={cn("text-xs font-mono", playbackIsActive ? "text-purple-400/80" : "text-zinc-400/70")}>
                            Playback {playbackText}
                        </span>
                    </div>

                    <div className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-green-500/5 border border-green-500/10">
                        <div className="w-2 h-2 rounded-full bg-green-500 animate-pulse" />
                        <span className="text-xs font-mono text-green-500/80">System Online</span>
                    </div>
                </div>
            </div>
        </header>
    );
}
