import { Vector3, Quaternion } from 'three';
import {
    rotateToWorldFrame,
    vectorMagnitude,
    calculateMagneticHeading,
    normalizeAngle,
    Matrix9,
    StateVector9,
} from './math-utils';

/**
 * Extended Kalman Filter for IMU-based position tracking
 * 
 * State Vector (9 elements):
 *   x = [px, py, pz, vx, vy, vz, bax, bay, baz]ᵀ
 *       └─position─┘ └─velocity─┘ └─accel bias─┘
 * 
 * Features:
 *   - Position/velocity estimation via accelerometer double-integration
 *   - ZUPT (Zero Velocity Update) when stationary
 *   - Magnetometer heading correction for yaw drift
 * 
 * The BNO085 provides sensor-fused orientation (quaternion), so we use that
 * directly for frame transformation rather than estimating orientation.
 */
export class EKFTracker {
    // State vector: [px, py, pz, vx, vy, vz, bax, bay, baz]
    private state: StateVector9;

    // State covariance matrix (9x9)
    private P: Matrix9;

    // Process noise covariance
    private Q: Matrix9;

    // Last update timestamp
    private lastUpdateTime: number;

    // Last known orientation (from BNO085)
    private lastQuaternion: { w: number; x: number; y: number; z: number } | null = null;

    // Reference heading captured at start
    private referenceHeading: number | null = null;

    // Heading correction gain (0-1, higher = more magnetometer trust)
    private readonly HEADING_CORRECTION_GAIN = 0.05;

    // ZUPT parameters
    private readonly ZUPT_ACCEL_THRESHOLD = 0.3; // m/s² - threshold for "stationary"
    private readonly ZUPT_FRAMES_REQUIRED = 5;   // Consecutive frames to trigger ZUPT
    private stationaryFrameCount = 0;

    // Velocity measurement noise for ZUPT (very low = trust ZUPT strongly)
    private readonly ZUPT_VELOCITY_NOISE = 0.001;

    // Maximum time delta before resetting (e.g., tab inactive)
    private readonly MAX_DT = 1.0;

    constructor() {
        // Initialize state to zeros (at origin, stationary, no bias)
        this.state = new StateVector9();

        // Initialize covariance with moderate uncertainty
        this.P = Matrix9.diagonal([
            0.1, 0.1, 0.1,     // Position uncertainty (m²)
            0.01, 0.01, 0.01,  // Velocity uncertainty (m/s)²
            0.01, 0.01, 0.01,  // Accel bias uncertainty (m/s²)²
        ]);

        // Process noise - tune based on sensor characteristics
        // Higher values = less trust in motion model
        this.Q = Matrix9.diagonal([
            0.001, 0.001, 0.001,   // Position process noise
            0.1, 0.1, 0.1,         // Velocity process noise (accelerometers are noisy)
            0.0001, 0.0001, 0.0001, // Bias drift (very slow)
        ]);

        this.lastUpdateTime = performance.now();
    }

    /**
     * EKF Prediction Step
     * Called when new accelerometer + quaternion data is available
     */
    public predict(
        accel: { x: number; y: number; z: number },
        quat: { w: number; x: number; y: number; z: number }
    ): { x: number; y: number; z: number } {
        const now = performance.now();
        const dt = (now - this.lastUpdateTime) / 1000; // seconds
        this.lastUpdateTime = now;

        // Store quaternion for magnetometer updates
        this.lastQuaternion = quat;

        // Skip if dt too large (prevents huge jumps after tab inactive)
        if (dt > this.MAX_DT || dt <= 0) {
            return this.getPosition();
        }

        // Get acceleration in world frame
        const accelWorld = rotateToWorldFrame(accel, quat);

        // Subtract bias estimate
        const ax = accelWorld.x - this.state.get(6);
        const ay = accelWorld.y - this.state.get(7);
        const az = accelWorld.z - this.state.get(8);

        // Check for stationary condition (ZUPT)
        const accelMag = vectorMagnitude(accel);
        if (accelMag < this.ZUPT_ACCEL_THRESHOLD) {
            this.stationaryFrameCount++;
        } else {
            this.stationaryFrameCount = 0;
        }

        // State prediction: x_new = f(x, u)
        // Position: p = p + v*dt + 0.5*a*dt²
        // Velocity: v = v + a*dt
        // Bias: b = b (constant model)

        const px = this.state.get(0) + this.state.get(3) * dt + 0.5 * ax * dt * dt;
        const py = this.state.get(1) + this.state.get(4) * dt + 0.5 * ay * dt * dt;
        const pz = this.state.get(2) + this.state.get(5) * dt + 0.5 * az * dt * dt;

        const vx = this.state.get(3) + ax * dt;
        const vy = this.state.get(4) + ay * dt;
        const vz = this.state.get(5) + az * dt;

        // Update state
        this.state.set(0, px);
        this.state.set(1, py);
        this.state.set(2, pz);
        this.state.set(3, vx);
        this.state.set(4, vy);
        this.state.set(5, vz);
        // Bias stays the same in prediction

        // Jacobian of state transition (linearized)
        // F = ∂f/∂x
        const F = Matrix9.identity();
        // ∂p/∂v = dt (position depends on velocity)
        F.set(0, 3, dt);
        F.set(1, 4, dt);
        F.set(2, 5, dt);
        // ∂p/∂b = -0.5*dt² (position depends on bias through acceleration)
        F.set(0, 6, -0.5 * dt * dt);
        F.set(1, 7, -0.5 * dt * dt);
        F.set(2, 8, -0.5 * dt * dt);
        // ∂v/∂b = -dt (velocity depends on bias)
        F.set(3, 6, -dt);
        F.set(4, 7, -dt);
        F.set(5, 8, -dt);

        // Covariance prediction: P = F*P*Fᵀ + Q
        const FP = F.multiply(this.P);
        const FPFt = FP.multiply(F.transpose());

        // Scale Q by dt for proper noise accumulation
        const Qscaled = this.Q.scale(dt);
        this.P = FPFt.add(Qscaled);

        // Apply ZUPT if stationary for enough frames
        if (this.stationaryFrameCount >= this.ZUPT_FRAMES_REQUIRED) {
            this.applyZUPT();
        }

        return this.getPosition();
    }

    /**
     * Zero Velocity Update (ZUPT)
     * When we detect the device is stationary, correct velocity to zero
     */
    private applyZUPT(): void {
        // Measurement model: z = H*x where we measure velocity = [0, 0, 0]
        // H selects velocity from state: H = [0 0 0 | 1 0 0 | 0 0 0]
        //                                    [0 0 0 | 0 1 0 | 0 0 0]
        //                                    [0 0 0 | 0 0 1 | 0 0 0]

        // Innovation: y = z - H*x = [0,0,0] - [vx,vy,vz] = -v
        const innov = [
            0 - this.state.get(3),
            0 - this.state.get(4),
            0 - this.state.get(5),
        ];

        // Measurement noise R (3x3 diagonal, very small = trust ZUPT)
        const R = this.ZUPT_VELOCITY_NOISE;

        // Kalman gain (simplified for velocity-only measurement)
        // K = P*Hᵀ*(H*P*Hᵀ + R)⁻¹
        // For velocity measurement, this simplifies significantly

        // Extract velocity covariance (3x3 block at [3:6, 3:6])
        const Pvv = [
            [this.P.get(3, 3), this.P.get(3, 4), this.P.get(3, 5)],
            [this.P.get(4, 3), this.P.get(4, 4), this.P.get(4, 5)],
            [this.P.get(5, 3), this.P.get(5, 4), this.P.get(5, 5)],
        ];

        // S = Pvv + R*I (simplified, since R is scalar and H is simple)
        const S = [
            [Pvv[0][0] + R, Pvv[0][1], Pvv[0][2]],
            [Pvv[1][0], Pvv[1][1] + R, Pvv[1][2]],
            [Pvv[2][0], Pvv[2][1], Pvv[2][2] + R],
        ];

        // Invert 3x3 S matrix
        const Sinv = this.invert3x3(S);
        if (!Sinv) return; // Skip if singular

        // Compute Kalman gain for each state element
        // K_i = P[i, 3:6] * Sinv
        const K = new Array(9).fill(null).map(() => [0, 0, 0]);
        for (let i = 0; i < 9; i++) {
            const row = [this.P.get(i, 3), this.P.get(i, 4), this.P.get(i, 5)];
            K[i] = this.multiplyRowBy3x3(row, Sinv);
        }

        // State update: x = x + K*y
        for (let i = 0; i < 9; i++) {
            const correction = K[i][0] * innov[0] + K[i][1] * innov[1] + K[i][2] * innov[2];
            this.state.set(i, this.state.get(i) + correction);
        }

        // Covariance update: P = (I - K*H)*P
        // This is simplified since H only selects velocity
        for (let i = 0; i < 9; i++) {
            for (let j = 0; j < 9; j++) {
                let sum = 0;
                for (let k = 0; k < 3; k++) {
                    sum += K[i][k] * this.P.get(3 + k, j);
                }
                this.P.set(i, j, this.P.get(i, j) - sum);
            }
        }
    }

    /**
     * Update with magnetometer reading
     * Uses heading to correct horizontal velocity direction drift
     */
    public updateMagnetometer(mag: { x: number; y: number; z: number }): void {
        if (!this.lastQuaternion) return;

        // Calculate current magnetic heading
        const heading = calculateMagneticHeading(mag, this.lastQuaternion);

        // Capture reference heading on first reading
        if (this.referenceHeading === null) {
            this.referenceHeading = heading;
            return;
        }

        // Calculate heading error (difference from reference)
        const headingError = normalizeAngle(heading - this.referenceHeading);

        // Get current horizontal velocity
        const vx = this.state.get(3);
        const vz = this.state.get(5);
        const horizontalSpeed = Math.sqrt(vx * vx + vz * vz);

        // Only correct if we're moving (avoid noise when stationary)
        if (horizontalSpeed < 0.1) return;

        // Calculate current velocity heading
        const velocityHeading = Math.atan2(vx, vz);

        // The idea: if magnetometer says we're pointing in a different direction
        // than our velocity is heading, rotate velocity slightly towards mag heading
        // This helps correct the "walk straight but path curves" problem

        // Correction: rotate velocity by a fraction of the heading error
        const correction = this.HEADING_CORRECTION_GAIN * headingError;
        const cos_c = Math.cos(correction);
        const sin_c = Math.sin(correction);

        // Rotate velocity vector in XZ plane
        const newVx = vx * cos_c - vz * sin_c;
        const newVz = vx * sin_c + vz * cos_c;

        this.state.set(3, newVx);
        this.state.set(5, newVz);
    }

    /**
     * Get current position estimate
     */
    public getPosition(): { x: number; y: number; z: number } {
        return {
            x: this.state.get(0),
            y: this.state.get(1),
            z: this.state.get(2),
        };
    }

    /**
     * Get current velocity estimate
     */
    public getVelocity(): { x: number; y: number; z: number } {
        return {
            x: this.state.get(3),
            y: this.state.get(4),
            z: this.state.get(5),
        };
    }

    /**
     * Reset tracker to origin
     */
    public reset(): void {
        this.state = new StateVector9();
        this.P = Matrix9.diagonal([
            0.1, 0.1, 0.1,
            0.01, 0.01, 0.01,
            0.01, 0.01, 0.01,
        ]);
        this.stationaryFrameCount = 0;
        this.lastQuaternion = null;
        this.referenceHeading = null;
        this.lastUpdateTime = performance.now();
    }

    // ========== Helper methods ==========

    /**
     * Invert a 3x3 matrix
     */
    private invert3x3(m: number[][]): number[][] | null {
        const det =
            m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
            m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
            m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

        if (Math.abs(det) < 1e-10) return null;

        const invDet = 1 / det;

        return [
            [
                (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet,
                (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invDet,
                (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet,
            ],
            [
                (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invDet,
                (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet,
                (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * invDet,
            ],
            [
                (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet,
                (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * invDet,
                (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet,
            ],
        ];
    }

    /**
     * Multiply a row vector by a 3x3 matrix
     */
    private multiplyRowBy3x3(row: number[], m: number[][]): number[] {
        return [
            row[0] * m[0][0] + row[1] * m[1][0] + row[2] * m[2][0],
            row[0] * m[0][1] + row[1] * m[1][1] + row[2] * m[2][1],
            row[0] * m[0][2] + row[1] * m[1][2] + row[2] * m[2][2],
        ];
    }
}
