import { Vector3, Quaternion } from 'three';

export class TrackingModel {
    private position: Vector3;
    private velocity: Vector3;
    private lastUpdateTime: number;

    // ZUPT (Zero Velocity Update) parameters
    private readonly ACCEL_THRESHOLD = 0.5; // m/s^2 - Threshold to consider "stationary"
    private readonly STATIONARY_FRAMES_REQUIRED = 5; // Number of consecutive frames below threshold to trigger ZUPT
    private stationaryFrameCount = 0;

    constructor() {
        this.position = new Vector3(0, 0, 0);
        this.velocity = new Vector3(0, 0, 0);
        this.lastUpdateTime = performance.now();
    }

    public update(accel: { x: number, y: number, z: number }, quat: { x: number, y: number, z: number, w: number }) {
        const now = performance.now();
        const dt = (now - this.lastUpdateTime) / 1000; // Convert ms to seconds
        this.lastUpdateTime = now;

        if (dt > 1.0) {
            // If time delta is too large (e.g. tab inactive), skip update to avoid huge jumps
            return this.getPosition();
        }

        // 1. Prepare vectors
        // Device acceleration (LinearAccel is already gravity-removed)
        const a_device = new Vector3(accel.x, accel.y, accel.z);

        // Dynamic ZUPT
        if (a_device.length() < this.ACCEL_THRESHOLD) {
            this.stationaryFrameCount++;
        } else {
            this.stationaryFrameCount = 0;
        }

        // If stationary for enough frames, force velocity to zero
        if (this.stationaryFrameCount >= this.STATIONARY_FRAMES_REQUIRED) {
            this.velocity.set(0, 0, 0);
            return this.getPosition();
        }

        // 2. Rotate acceleration to World Frame
        // a_world = q * a_device * q_inverse
        const orientation = new Quaternion(quat.x, quat.y, quat.z, quat.w);
        const a_world = a_device.clone().applyQuaternion(orientation);

        // 3. Integrate
        // v = v + a * dt
        this.velocity.addScaledVector(a_world, dt);

        // p = p + v * dt
        this.position.addScaledVector(this.velocity, dt);

        // Apply drag/damping to drift?
        // this.velocity.multiplyScalar(0.98); 

        return this.getPosition();
    }

    public getPosition() {
        return { x: this.position.x, y: this.position.y, z: this.position.z };
    }

    public reset() {
        this.position.set(0, 0, 0);
        this.velocity.set(0, 0, 0);
        this.stationaryFrameCount = 0;
        this.lastUpdateTime = performance.now();
    }
}
