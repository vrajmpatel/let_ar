'use client';

import { createContext, useContext, useState, useCallback, ReactNode } from 'react';

export interface Quaternion {
    w: number;
    x: number;
    y: number;
    z: number;
}

interface QuaternionContextType {
    quaternion: Quaternion | null;
    setQuaternion: (q: Quaternion) => void;
}

const QuaternionContext = createContext<QuaternionContextType | null>(null);

export function QuaternionProvider({ children }: { children: ReactNode }) {
    const [quaternion, setQuaternionState] = useState<Quaternion | null>(null);

    const setQuaternion = useCallback((q: Quaternion) => {
        setQuaternionState(q);
    }, []);

    return (
        <QuaternionContext.Provider value={{ quaternion, setQuaternion }}>
            {children}
        </QuaternionContext.Provider>
    );
}

export function useQuaternion() {
    const context = useContext(QuaternionContext);
    if (!context) {
        throw new Error('useQuaternion must be used within a QuaternionProvider');
    }
    return context;
}

// Optional hook for components outside the context (returns null if no provider)
export function useQuaternionSafe() {
    return useContext(QuaternionContext);
}
