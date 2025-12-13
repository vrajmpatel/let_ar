import { EKFTracker } from "./EKFTracker";
import { transformAccelerationWithCalibration } from "./CalibrationManager";
import type { IMURecordingV1, QuaternionData, RecordingEventV1 } from "./recording";
import type { ReplayFrameV1, ReplaySessionV1, Vector3Data } from "./replay";

function clamp01(value: number): number {
    if (value < 0) return 0;
    if (value > 1) return 1;
    return value;
}

function normalizeQuaternion(q: QuaternionData): QuaternionData {
    const len = Math.sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    if (!Number.isFinite(len) || len === 0) {
        return { w: 1, x: 0, y: 0, z: 0 };
    }
    return { w: q.w / len, x: q.x / len, y: q.y / len, z: q.z / len };
}

function slerpQuaternion(aIn: QuaternionData, bIn: QuaternionData, t: number): QuaternionData {
    const a = normalizeQuaternion(aIn);
    let b = normalizeQuaternion(bIn);

    let dot = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
    if (dot < 0) {
        dot = -dot;
        b = { w: -b.w, x: -b.x, y: -b.y, z: -b.z };
    }

    if (dot > 0.9995) {
        const lerped = {
            w: a.w + t * (b.w - a.w),
            x: a.x + t * (b.x - a.x),
            y: a.y + t * (b.y - a.y),
            z: a.z + t * (b.z - a.z),
        };
        return normalizeQuaternion(lerped);
    }

    const theta0 = Math.acos(clamp01(dot));
    const sinTheta0 = Math.sin(theta0);
    if (sinTheta0 === 0) return a;

    const theta = theta0 * t;
    const sinTheta = Math.sin(theta);
    const s0 = Math.cos(theta) - dot * (sinTheta / sinTheta0);
    const s1 = sinTheta / sinTheta0;

    return normalizeQuaternion({
        w: s0 * a.w + s1 * b.w,
        x: s0 * a.x + s1 * b.x,
        y: s0 * a.y + s1 * b.y,
        z: s0 * a.z + s1 * b.z,
    });
}

function estimateAccelDtSeconds(events: RecordingEventV1[]): number {
    const accelTimes: number[] = [];
    for (const e of events) {
        if (e.linearAccel) accelTimes.push(e.tMs);
        if (accelTimes.length >= 12) break;
    }
    if (accelTimes.length < 2) return 1 / 60;
    let sum = 0;
    let count = 0;
    for (let i = 1; i < accelTimes.length; i++) {
        const dt = (accelTimes[i] - accelTimes[i - 1]) / 1000;
        if (dt > 0 && dt < 0.2) {
            sum += dt;
            count++;
        }
    }
    return count > 0 ? sum / count : 1 / 60;
}

function lerp(a: number, b: number, t: number): number {
    return a + (b - a) * t;
}

function lerpVec3(a: Vector3Data, b: Vector3Data, t: number): Vector3Data {
    return { x: lerp(a.x, b.x, t), y: lerp(a.y, b.y, t), z: lerp(a.z, b.z, t) };
}

export function preprocessRecordingToReplay(
    recording: IMURecordingV1,
    options?: { frameRate?: number; sourceFileName?: string }
): ReplaySessionV1 {
    const events = [...recording.events].sort((a, b) => a.tMs - b.tMs);

    const quaternionKeyframes: Array<{ tMs: number; quaternion: QuaternionData }> = [];
    const positionKeyframes: Array<{ tMs: number; position: Vector3Data }> = [];

    const tracker = new EKFTracker();
    const defaultDt = estimateAccelDtSeconds(events);

    let lastQuat: QuaternionData | null = null;
    let lastAccelTMs: number | null = null;

    for (const event of events) {
        if (event.quaternion) {
            lastQuat = normalizeQuaternion(event.quaternion);
            quaternionKeyframes.push({ tMs: event.tMs, quaternion: lastQuat });
        }

        if (event.linearAccel && lastQuat) {
            const accel = transformAccelerationWithCalibration(recording.calibration ?? null, event.linearAccel);
            const dt = lastAccelTMs === null ? defaultDt : (event.tMs - lastAccelTMs) / 1000;
            lastAccelTMs = event.tMs;
            const position = tracker.predictWithDt(accel, lastQuat, dt);
            positionKeyframes.push({ tMs: event.tMs, position });
        }

        if (event.magnetometer) {
            tracker.updateMagnetometer(event.magnetometer);
        }
    }

    const lastQuatT = quaternionKeyframes.length > 0 ? quaternionKeyframes[quaternionKeyframes.length - 1].tMs : 0;
    const lastPosT = positionKeyframes.length > 0 ? positionKeyframes[positionKeyframes.length - 1].tMs : 0;
    const durationMs = Math.max(lastQuatT, lastPosT, events.length > 0 ? events[events.length - 1].tMs : 0);

    const frameRate = options?.frameRate ?? 60;
    const frameIntervalMs = 1000 / frameRate;
    const frames: ReplayFrameV1[] = [];

    let qIndex = 0;
    let pIndex = 0;

    for (let tMs = 0; tMs <= durationMs; tMs += frameIntervalMs) {
        while (qIndex + 1 < quaternionKeyframes.length && quaternionKeyframes[qIndex + 1].tMs <= tMs) qIndex++;
        while (pIndex + 1 < positionKeyframes.length && positionKeyframes[pIndex + 1].tMs <= tMs) pIndex++;

        const q0 = quaternionKeyframes[qIndex]?.quaternion ?? { w: 1, x: 0, y: 0, z: 0 };
        const q1 = quaternionKeyframes[Math.min(qIndex + 1, quaternionKeyframes.length - 1)]?.quaternion ?? q0;
        const q0t = quaternionKeyframes[qIndex]?.tMs ?? 0;
        const q1t = quaternionKeyframes[Math.min(qIndex + 1, quaternionKeyframes.length - 1)]?.tMs ?? q0t;
        const qAlpha = q1t === q0t ? 0 : clamp01((tMs - q0t) / (q1t - q0t));
        const quaternion = slerpQuaternion(q0, q1, qAlpha);

        const p0 = positionKeyframes[pIndex]?.position ?? { x: 0, y: 0, z: 0 };
        const p1 = positionKeyframes[Math.min(pIndex + 1, positionKeyframes.length - 1)]?.position ?? p0;
        const p0t = positionKeyframes[pIndex]?.tMs ?? 0;
        const p1t = positionKeyframes[Math.min(pIndex + 1, positionKeyframes.length - 1)]?.tMs ?? p0t;
        const pAlpha = p1t === p0t ? 0 : clamp01((tMs - p0t) / (p1t - p0t));
        const position = lerpVec3(p0, p1, pAlpha);

        frames.push({ tMs, position, quaternion });
    }

    return {
        schemaVersion: 1,
        sourceFileName: options?.sourceFileName,
        deviceName: recording.deviceName ?? null,
        durationMs,
        frames,
    };
}

