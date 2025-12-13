'use client';

/**
 * CalibrationManager - Handles IMU axis calibration and transformation
 * 
 * During calibration, the user moves the device in 6 directions (+X, -X, +Y, -Y, +Z, -Z).
 * The system collects accelerometer readings for each direction and builds a transformation
 * matrix to align IMU axes with world/display axes.
 */

export type CalibrationStep =
    | 'idle'
    | 'posX'
    | 'negX'
    | 'posY'
    | 'negY'
    | 'posZ'
    | 'negZ'
    | 'complete';

export interface Vector3Data {
    x: number;
    y: number;
    z: number;
}

export interface CalibrationData {
    posX: Vector3Data;  // Average accel when moving in +X direction
    negX: Vector3Data;
    posY: Vector3Data;
    negY: Vector3Data;
    posZ: Vector3Data;
    negZ: Vector3Data;
    timestamp: number;  // When calibration was performed
    deviceName?: string;
}

// Step configuration
export const CALIBRATION_STEPS: { step: CalibrationStep; label: string; instruction: string }[] = [
    { step: 'posX', label: '+X', instruction: 'Move the device in the POSITIVE X direction (right) and hold steady...' },
    { step: 'negX', label: '-X', instruction: 'Move the device in the NEGATIVE X direction (left) and hold steady...' },
    { step: 'posY', label: '+Y', instruction: 'Move the device in the POSITIVE Y direction (up) and hold steady...' },
    { step: 'negY', label: '-Y', instruction: 'Move the device in the NEGATIVE Y direction (down) and hold steady...' },
    { step: 'posZ', label: '+Z', instruction: 'Move the device in the POSITIVE Z direction (forward) and hold steady...' },
    { step: 'negZ', label: '-Z', instruction: 'Move the device in the NEGATIVE Z direction (backward) and hold steady...' },
];

export const CALIBRATION_STORAGE_KEY = 'imu_calibration_data';
const SAMPLES_PER_STEP = 25;

export function transformAccelerationWithCalibration(calibrationData: CalibrationData | null, accel: Vector3Data): Vector3Data {
    if (!calibrationData) {
        return accel; // No calibration, return as-is
    }

    // Calculate the axis mapping from calibration data
    // The idea: during +X calibration, the dominant acceleration axis tells us which IMU axis maps to world X
    const cal = calibrationData;

    // Calculate difference vectors for each axis (positive - negative = axis direction)
    const xAxis = {
        x: cal.posX.x - cal.negX.x,
        y: cal.posX.y - cal.negX.y,
        z: cal.posX.z - cal.negX.z,
    };
    const yAxis = {
        x: cal.posY.x - cal.negY.x,
        y: cal.posY.y - cal.negY.y,
        z: cal.posY.z - cal.negY.z,
    };
    const zAxis = {
        x: cal.posZ.x - cal.negZ.x,
        y: cal.posZ.y - cal.negZ.y,
        z: cal.posZ.z - cal.negZ.z,
    };

    // Normalize the axis vectors
    const normalize = (v: Vector3Data): Vector3Data => {
        const len = Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        if (len === 0) return { x: 0, y: 0, z: 0 };
        return { x: v.x / len, y: v.y / len, z: v.z / len };
    };

    const xNorm = normalize(xAxis);
    const yNorm = normalize(yAxis);
    const zNorm = normalize(zAxis);

    // Transform the acceleration using the calibration matrix (dot products)
    // This projects the raw acceleration onto our calibrated axes
    return {
        x: accel.x * xNorm.x + accel.y * xNorm.y + accel.z * xNorm.z,
        y: accel.x * yNorm.x + accel.y * yNorm.y + accel.z * yNorm.z,
        z: accel.x * zNorm.x + accel.y * zNorm.y + accel.z * zNorm.z,
    };
}

export class CalibrationManager {
    private samples: Vector3Data[] = [];
    private currentStep: CalibrationStep = 'idle';
    private calibrationData: CalibrationData | null = null;
    private onStepChange?: (step: CalibrationStep, message: string) => void;
    private onComplete?: (data: CalibrationData) => void;

    constructor() {
        // Try to load existing calibration from localStorage
        this.loadFromStorage();
    }

    private getStorage(): Storage | null {
        if (typeof window === 'undefined') return null;
        try {
            return window.localStorage;
        } catch {
            return null;
        }
    }

    /**
     * Check if calibration data exists
     */
    public hasCalibration(): boolean {
        return this.calibrationData !== null;
    }

    /**
     * Get current calibration data
     */
    public getCalibration(): CalibrationData | null {
        return this.calibrationData;
    }

    /**
     * Get current calibration step
     */
    public getCurrentStep(): CalibrationStep {
        return this.currentStep;
    }

    /**
     * Check if currently in calibration mode
     */
    public isCalibrating(): boolean {
        return this.currentStep !== 'idle' && this.currentStep !== 'complete';
    }

    /**
     * Set callbacks for calibration events
     */
    public setCallbacks(
        onStepChange: (step: CalibrationStep, message: string) => void,
        onComplete: (data: CalibrationData) => void
    ) {
        this.onStepChange = onStepChange;
        this.onComplete = onComplete;
    }

    /**
     * Start the calibration process
     */
    public startCalibration() {
        this.samples = [];
        this.currentStep = 'posX';
        const stepConfig = CALIBRATION_STEPS.find(s => s.step === this.currentStep);
        if (this.onStepChange && stepConfig) {
            this.onStepChange(this.currentStep, `Starting calibration... ${stepConfig.instruction}`);
        }
    }

    /**
     * Cancel calibration and return to idle
     */
    public cancelCalibration() {
        this.samples = [];
        this.currentStep = 'idle';
        if (this.onStepChange) {
            this.onStepChange('idle', 'Calibration cancelled.');
        }
    }

    /**
     * Add an accelerometer sample during calibration
     * Returns true if the step is complete
     */
    public addSample(accel: Vector3Data): boolean {
        if (!this.isCalibrating()) return false;

        this.samples.push({ ...accel });

        // Check if we have enough samples for this step
        if (this.samples.length >= SAMPLES_PER_STEP) {
            this.completeCurrentStep();
            return true;
        }

        // Report progress
        const progress = Math.floor((this.samples.length / SAMPLES_PER_STEP) * 100);
        if (this.samples.length % 5 === 0 && this.onStepChange) {
            this.onStepChange(this.currentStep, `Collecting samples... ${progress}%`);
        }

        return false;
    }

    /**
     * Complete current step and move to next
     */
    private completeCurrentStep() {
        // Calculate average of samples
        const avg = this.averageSamples(this.samples);

        // Initialize calibration data if needed
        if (!this.calibrationData) {
            this.calibrationData = {
                posX: { x: 0, y: 0, z: 0 },
                negX: { x: 0, y: 0, z: 0 },
                posY: { x: 0, y: 0, z: 0 },
                negY: { x: 0, y: 0, z: 0 },
                posZ: { x: 0, y: 0, z: 0 },
                negZ: { x: 0, y: 0, z: 0 },
                timestamp: Date.now(),
            };
        }

        // Store the average for current step
        switch (this.currentStep) {
            case 'posX': this.calibrationData.posX = avg; break;
            case 'negX': this.calibrationData.negX = avg; break;
            case 'posY': this.calibrationData.posY = avg; break;
            case 'negY': this.calibrationData.negY = avg; break;
            case 'posZ': this.calibrationData.posZ = avg; break;
            case 'negZ': this.calibrationData.negZ = avg; break;
        }

        // Clear samples for next step
        this.samples = [];

        // Move to next step
        const stepOrder: CalibrationStep[] = ['posX', 'negX', 'posY', 'negY', 'posZ', 'negZ', 'complete'];
        const currentIndex = stepOrder.indexOf(this.currentStep);
        this.currentStep = stepOrder[currentIndex + 1];

        if (this.currentStep === 'complete') {
            this.calibrationData.timestamp = Date.now();
            this.saveToStorage();
            if (this.onStepChange) {
                this.onStepChange('complete', 'Calibration complete! Data saved.');
            }
            if (this.onComplete && this.calibrationData) {
                this.onComplete(this.calibrationData);
            }
        } else {
            const stepConfig = CALIBRATION_STEPS.find(s => s.step === this.currentStep);
            if (this.onStepChange && stepConfig) {
                this.onStepChange(this.currentStep, `Step complete! Next: ${stepConfig.instruction}`);
            }
        }
    }

    /**
     * Average a set of vector samples
     */
    private averageSamples(samples: Vector3Data[]): Vector3Data {
        if (samples.length === 0) return { x: 0, y: 0, z: 0 };

        const sum = samples.reduce(
            (acc, s) => ({ x: acc.x + s.x, y: acc.y + s.y, z: acc.z + s.z }),
            { x: 0, y: 0, z: 0 }
        );

        return {
            x: sum.x / samples.length,
            y: sum.y / samples.length,
            z: sum.z / samples.length,
        };
    }

    /**
     * Apply calibration transformation to incoming acceleration data.
     * This maps the IMU's axes to the expected world axes based on calibration.
     */
    public transformAcceleration(accel: Vector3Data): Vector3Data {
        return transformAccelerationWithCalibration(this.calibrationData, accel);
    }

    /**
     * Save calibration data to localStorage
     */
    private saveToStorage() {
        if (this.calibrationData) {
            const storage = this.getStorage();
            if (!storage) return;
            try {
                storage.setItem(CALIBRATION_STORAGE_KEY, JSON.stringify(this.calibrationData));
            } catch (e) {
                console.error('Failed to save calibration to localStorage:', e);
            }
        }
    }

    /**
     * Load calibration data from localStorage
     */
    private loadFromStorage() {
        const storage = this.getStorage();
        if (!storage) return;
        try {
            const stored = storage.getItem(CALIBRATION_STORAGE_KEY);
            if (stored) {
                this.calibrationData = JSON.parse(stored);
                console.log('Loaded calibration from storage:', this.calibrationData);
            }
        } catch (e) {
            console.error('Failed to load calibration from localStorage:', e);
            this.calibrationData = null;
        }
    }

    /**
     * Clear stored calibration data
     */
    public clearCalibration() {
        this.calibrationData = null;
        this.currentStep = 'idle';
        const storage = this.getStorage();
        if (!storage) return;
        try {
            storage.removeItem(CALIBRATION_STORAGE_KEY);
        } catch (e) {
            console.error('Failed to clear calibration from localStorage:', e);
        }
    }

    /**
     * Export calibration data as JSON string (for file save)
     */
    public exportCalibration(): string | null {
        if (!this.calibrationData) return null;
        return JSON.stringify(this.calibrationData, null, 2);
    }

    /**
     * Import calibration data from JSON string
     */
    public importCalibration(json: string): boolean {
        try {
            const data = JSON.parse(json) as CalibrationData;
            // Validate structure
            if (data.posX && data.negX && data.posY && data.negY && data.posZ && data.negZ) {
                this.calibrationData = data;
                this.saveToStorage();
                return true;
            }
        } catch (e) {
            console.error('Failed to import calibration:', e);
        }
        return false;
    }
}
