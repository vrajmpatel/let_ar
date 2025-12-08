'use client';

import { Terminal as TerminalIcon, Bluetooth, BluetoothOff, Trash2 } from "lucide-react";
import { cn } from "@/lib/utils";
import { useBluetooth, TerminalEntry } from "@/hooks/useBluetooth";
import { useEffect, useRef } from "react";

function EntryIcon({ type }: { type: TerminalEntry['type'] }) {
    switch (type) {
        case 'system':
            return <span className="text-blue-400">▸</span>;
        case 'data':
            return <span className="text-emerald-400">◆</span>;
        case 'error':
            return <span className="text-red-400">✖</span>;
        default:
            return <span className="text-zinc-500">▸</span>;
    }
}

export function Terminal() {
    const {
        isConnected,
        isConnecting,
        deviceName,
        entries,
        connect,
        disconnect,
        clearEntries,
    } = useBluetooth();

    const scrollRef = useRef<HTMLDivElement>(null);

    // Auto-scroll to bottom when new entries arrive
    useEffect(() => {
        if (scrollRef.current) {
            scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
        }
    }, [entries]);

    return (
        <div className={cn(
            "h-full w-full flex flex-col overflow-hidden",
            "bg-zinc-950/60 backdrop-blur-xl border-l border-white/5 shadow-2xl"
        )}>
            {/* Header */}
            <div className="flex items-center justify-between px-4 py-3 border-b border-white/5 bg-zinc-950/40">
                <div className="flex items-center gap-3">
                    <TerminalIcon className="w-4 h-4 text-muted-foreground" />
                    <span className="text-sm font-medium text-muted-foreground tracking-tight">
                        Terminal Output
                    </span>
                </div>

                {/* Connection status indicator */}
                <div className="flex items-center gap-2">
                    <div className={cn(
                        "w-2 h-2 rounded-full",
                        isConnected ? "bg-emerald-400 shadow-lg shadow-emerald-400/50" :
                            isConnecting ? "bg-amber-400 animate-pulse" : "bg-zinc-600"
                    )} />
                    <span className="text-xs text-muted-foreground">
                        {isConnected ? deviceName : isConnecting ? "Connecting..." : "Disconnected"}
                    </span>
                </div>
            </div>

            {/* Control buttons */}
            <div className="flex items-center gap-2 px-4 py-2 border-b border-white/10 bg-zinc-900/50">
                {!isConnected ? (
                    <button
                        onClick={connect}
                        disabled={isConnecting}
                        className={cn(
                            "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all",
                            "bg-blue-600/20 border border-blue-500/30 text-blue-400",
                            "hover:bg-blue-600/30 hover:border-blue-500/50",
                            "disabled:opacity-50 disabled:cursor-not-allowed"
                        )}
                    >
                        <Bluetooth className="w-3.5 h-3.5" />
                        {isConnecting ? "Connecting..." : "Connect"}
                    </button>
                ) : (
                    <button
                        onClick={disconnect}
                        className={cn(
                            "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all",
                            "bg-red-600/20 border border-red-500/30 text-red-400",
                            "hover:bg-red-600/30 hover:border-red-500/50"
                        )}
                    >
                        <BluetoothOff className="w-3.5 h-3.5" />
                        Disconnect
                    </button>
                )}

                <button
                    onClick={clearEntries}
                    className={cn(
                        "flex items-center gap-2 px-3 py-1.5 rounded-md text-xs font-medium transition-all",
                        "bg-zinc-800/50 border border-zinc-700/50 text-zinc-400",
                        "hover:bg-zinc-700/50 hover:text-zinc-300"
                    )}
                >
                    <Trash2 className="w-3.5 h-3.5" />
                    Clear
                </button>
            </div>

            {/* Terminal output */}
            <div
                ref={scrollRef}
                className="flex-1 p-4 font-mono text-xs space-y-1 overflow-y-auto text-muted-foreground/80 scrollbar-thin scrollbar-thumb-white/10 scrollbar-track-transparent"
            >
                {entries.length === 0 ? (
                    <div className="flex items-center gap-2 text-zinc-600">
                        <span className="animate-pulse">▸</span>
                        <span>Waiting for input...</span>
                    </div>
                ) : (
                    entries.map((entry) => (
                        <div
                            key={entry.id}
                            className={cn(
                                "flex gap-2 leading-relaxed",
                                entry.type === 'error' && "text-red-400",
                                entry.type === 'data' && "text-emerald-300/90",
                                entry.type === 'system' && "text-zinc-400"
                            )}
                        >
                            <EntryIcon type={entry.type} />
                            <span className="flex-1 break-all">{entry.message}</span>
                        </div>
                    ))
                )}
            </div>

            {/* Footer with stats */}
            {isConnected && (
                <div className="px-4 py-2 border-t border-white/5 bg-zinc-900/30 text-xs text-zinc-500 flex items-center justify-between">
                    <span>Packets: {entries.filter(e => e.type === 'data').length}</span>
                    <span className="flex items-center gap-1">
                        <span className="w-1.5 h-1.5 rounded-full bg-emerald-400 animate-pulse" />
                        Live
                    </span>
                </div>
            )}
        </div>
    );
}
