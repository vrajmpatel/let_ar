import type { CalibrationData, Vector3Data } from "./CalibrationManager";

export interface QuaternionData {
    w: number;
    x: number;
    y: number;
    z: number;
}

export interface RecordingEventV1 {
    tMs: number;
    timestamp: string;
    type: "system" | "data" | "error";
    message: string;
    quaternion?: QuaternionData;
    linearAccel?: Vector3Data;
    magnetometer?: Vector3Data;
}

export interface IMURecordingV1 {
    schemaVersion: 1;
    recordedAt: string;
    deviceName?: string | null;
    connectedAt?: string | null;
    disconnectedAt?: string | null;
    calibration?: CalibrationData | null;
    events: RecordingEventV1[];
}

function isNumber(value: unknown): value is number {
    return typeof value === "number" && Number.isFinite(value);
}

function isVector3(value: unknown): value is { x: number; y: number; z: number } {
    if (!value || typeof value !== "object") return false;
    const v = value as { x?: unknown; y?: unknown; z?: unknown };
    return isNumber(v.x) && isNumber(v.y) && isNumber(v.z);
}

function isQuaternion(value: unknown): value is QuaternionData {
    if (!value || typeof value !== "object") return false;
    const q = value as { w?: unknown; x?: unknown; y?: unknown; z?: unknown };
    return isNumber(q.w) && isNumber(q.x) && isNumber(q.y) && isNumber(q.z);
}

export function isIMURecordingV1(value: unknown): value is IMURecordingV1 {
    if (!value || typeof value !== "object") return false;
    const v = value as Partial<IMURecordingV1>;
    if (v.schemaVersion !== 1) return false;
    if (typeof v.recordedAt !== "string") return false;
    if (!Array.isArray(v.events)) return false;
    for (const event of v.events) {
        if (!event || typeof event !== "object") return false;
        const e = event as Partial<RecordingEventV1>;
        if (!isNumber(e.tMs)) return false;
        if (typeof e.timestamp !== "string") return false;
        if (e.type !== "system" && e.type !== "data" && e.type !== "error") return false;
        if (typeof e.message !== "string") return false;
        if (e.quaternion !== undefined && !isQuaternion(e.quaternion)) return false;
        if (e.linearAccel !== undefined && !isVector3(e.linearAccel)) return false;
        if (e.magnetometer !== undefined && !isVector3(e.magnetometer)) return false;
    }
    return true;
}

