'use client';

import { useState, useEffect, useCallback, useRef } from 'react';
import { CalibrationManager, CalibrationStep, CalibrationData, Vector3Data, CALIBRATION_STEPS } from '@/lib/CalibrationManager';

export interface UseCalibrationOptions {
    onCalibrationMessage?: (message: string, type: 'system' | 'data' | 'error') => void;
}

export interface UseCalibrationReturn {
    isCalibrating: boolean;
    hasCalibration: boolean;
    currentStep: CalibrationStep;
    calibrationData: CalibrationData | null;
    startCalibration: () => void;
    cancelCalibration: () => void;
    clearCalibration: () => void;
    addAccelSample: (accel: Vector3Data) => void;
    transformAcceleration: (accel: Vector3Data) => Vector3Data;
    getStepProgress: () => { current: number; total: number; label: string } | null;
}

export function useCalibration(options: UseCalibrationOptions = {}): UseCalibrationReturn {
    const { onCalibrationMessage } = options;

    const [manager] = useState(() => new CalibrationManager());
    const managerRef = useRef(manager);
    const [isCalibrating, setIsCalibrating] = useState(() => manager.isCalibrating());
    const [hasCalibration, setHasCalibration] = useState(() => manager.hasCalibration());
    const [currentStep, setCurrentStep] = useState<CalibrationStep>(() => manager.getCurrentStep());
    const [calibrationData, setCalibrationData] = useState<CalibrationData | null>(() => manager.getCalibration());

    // Keep callback ref updated
    const onCalibrationMessageRef = useRef(onCalibrationMessage);
    useEffect(() => {
        onCalibrationMessageRef.current = onCalibrationMessage;
    }, [onCalibrationMessage]);

    // Initialize callbacks on the manager (avoid setState-in-effect lint by not setting state directly here)
    useEffect(() => {
        manager.setCallbacks(
            (step, message) => {
                setCurrentStep(step);
                setIsCalibrating(step !== 'idle' && step !== 'complete');
                if (step === 'complete') {
                    setHasCalibration(true);
                    setCalibrationData(manager.getCalibration());
                }
                onCalibrationMessageRef.current?.(message, 'system');
            },
            (data) => {
                setCalibrationData(data);
                setHasCalibration(true);
            }
        );
    }, [manager]);

    const startCalibration = useCallback(() => {
        managerRef.current.startCalibration();
        setIsCalibrating(true);
        setCurrentStep('posX');
    }, []);

    const cancelCalibration = useCallback(() => {
        managerRef.current.cancelCalibration();
        setIsCalibrating(false);
        setCurrentStep('idle');
    }, []);

    const clearCalibration = useCallback(() => {
        managerRef.current.clearCalibration();
        setHasCalibration(false);
        setCalibrationData(null);
        setCurrentStep('idle');
        onCalibrationMessageRef.current?.('Calibration data cleared.', 'system');
    }, []);

    const addAccelSample = useCallback((accel: Vector3Data) => {
        if (managerRef.current.isCalibrating()) managerRef.current.addSample(accel);
    }, []);

    const transformAcceleration = useCallback((accel: Vector3Data): Vector3Data => {
        return managerRef.current.transformAcceleration(accel);
    }, []);

    const getStepProgress = useCallback((): { current: number; total: number; label: string } | null => {
        if (!isCalibrating) return null;
        const stepIndex = CALIBRATION_STEPS.findIndex(s => s.step === currentStep);
        if (stepIndex === -1) return null;
        return {
            current: stepIndex + 1,
            total: CALIBRATION_STEPS.length,
            label: CALIBRATION_STEPS[stepIndex].label,
        };
    }, [isCalibrating, currentStep]);

    return {
        isCalibrating,
        hasCalibration,
        currentStep,
        calibrationData,
        startCalibration,
        cancelCalibration,
        clearCalibration,
        addAccelSample,
        transformAcceleration,
        getStepProgress,
    };
}
