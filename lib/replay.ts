import type { QuaternionData } from "./recording";

export interface Vector3Data {
    x: number;
    y: number;
    z: number;
}

export interface ReplayFrameV1 {
    tMs: number;
    position: Vector3Data;
    quaternion: QuaternionData;
}

export interface ReplaySessionV1 {
    schemaVersion: 1;
    sourceFileName?: string;
    deviceName?: string | null;
    durationMs: number;
    frames: ReplayFrameV1[];
}

