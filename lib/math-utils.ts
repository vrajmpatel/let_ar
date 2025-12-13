import { Vector3, Quaternion } from 'three';

/**
 * Calculate magnetic heading (yaw) from magnetometer reading
 * Returns heading in radians, where 0 = North, positive = clockwise
 * 
 * The magnetometer provides the Earth's magnetic field vector in the device frame.
 * We project this onto the XY plane (horizontal) to get magnetic heading.
 * 
 * @param mag Magnetometer reading in device frame (µT)
 * @param quat Device orientation quaternion (world frame)
 * @returns Heading in radians (0 to 2π)
 */
export function calculateMagneticHeading(mag: { x: number; y: number; z: number }, quat: { w: number; x: number; y: number; z: number }): number {
    // Transform magnetometer reading to world frame
    const magVec = new Vector3(mag.x, mag.y, mag.z);
    const orientation = new Quaternion(quat.x, quat.y, quat.z, quat.w);
    const magWorld = magVec.clone().applyQuaternion(orientation);

    // Project onto horizontal plane and calculate heading
    // atan2(East, North) gives heading from North
    // In our coordinate system: X = East, Y = Up, Z = North (typically)
    // Adjust based on your coordinate convention
    const heading = Math.atan2(magWorld.x, magWorld.z);

    // Normalize to 0-2π
    return heading < 0 ? heading + 2 * Math.PI : heading;
}

/**
 * Rotate a vector from device frame to world frame using quaternion
 * @param vec Vector in device frame
 * @param quat Orientation quaternion
 * @returns Vector in world frame
 */
export function rotateToWorldFrame(
    vec: { x: number; y: number; z: number },
    quat: { w: number; x: number; y: number; z: number }
): { x: number; y: number; z: number } {
    const v = new Vector3(vec.x, vec.y, vec.z);
    const q = new Quaternion(quat.x, quat.y, quat.z, quat.w);
    const rotated = v.applyQuaternion(q);
    return { x: rotated.x, y: rotated.y, z: rotated.z };
}

/**
 * Calculate the magnitude of a 3D vector
 */
export function vectorMagnitude(v: { x: number; y: number; z: number }): number {
    return Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * Normalize angle to range [-π, π]
 */
export function normalizeAngle(angle: number): number {
    while (angle > Math.PI) angle -= 2 * Math.PI;
    while (angle < -Math.PI) angle += 2 * Math.PI;
    return angle;
}

/**
 * Simple 9x9 matrix operations for EKF
 * Using flat arrays for simplicity (row-major order)
 */
export class Matrix9 {
    data: number[];

    constructor(data?: number[]) {
        this.data = data || new Array(81).fill(0);
    }

    static identity(): Matrix9 {
        const m = new Matrix9();
        for (let i = 0; i < 9; i++) {
            m.data[i * 9 + i] = 1;
        }
        return m;
    }

    static diagonal(values: number[]): Matrix9 {
        const m = new Matrix9();
        for (let i = 0; i < Math.min(9, values.length); i++) {
            m.data[i * 9 + i] = values[i];
        }
        return m;
    }

    get(row: number, col: number): number {
        return this.data[row * 9 + col];
    }

    set(row: number, col: number, value: number): void {
        this.data[row * 9 + col] = value;
    }

    add(other: Matrix9): Matrix9 {
        const result = new Matrix9();
        for (let i = 0; i < 81; i++) {
            result.data[i] = this.data[i] + other.data[i];
        }
        return result;
    }

    multiply(other: Matrix9): Matrix9 {
        const result = new Matrix9();
        for (let i = 0; i < 9; i++) {
            for (let j = 0; j < 9; j++) {
                let sum = 0;
                for (let k = 0; k < 9; k++) {
                    sum += this.get(i, k) * other.get(k, j);
                }
                result.set(i, j, sum);
            }
        }
        return result;
    }

    transpose(): Matrix9 {
        const result = new Matrix9();
        for (let i = 0; i < 9; i++) {
            for (let j = 0; j < 9; j++) {
                result.set(j, i, this.get(i, j));
            }
        }
        return result;
    }

    scale(s: number): Matrix9 {
        const result = new Matrix9();
        for (let i = 0; i < 81; i++) {
            result.data[i] = this.data[i] * s;
        }
        return result;
    }

    clone(): Matrix9 {
        return new Matrix9([...this.data]);
    }
}

/**
 * Simple state vector operations (9 elements)
 */
export class StateVector9 {
    data: number[];

    constructor(data?: number[]) {
        this.data = data || new Array(9).fill(0);
    }

    get(i: number): number {
        return this.data[i];
    }

    set(i: number, value: number): void {
        this.data[i] = value;
    }

    add(other: StateVector9): StateVector9 {
        const result = new StateVector9();
        for (let i = 0; i < 9; i++) {
            result.data[i] = this.data[i] + other.data[i];
        }
        return result;
    }

    subtract(other: StateVector9): StateVector9 {
        const result = new StateVector9();
        for (let i = 0; i < 9; i++) {
            result.data[i] = this.data[i] - other.data[i];
        }
        return result;
    }

    clone(): StateVector9 {
        return new StateVector9([...this.data]);
    }
}
