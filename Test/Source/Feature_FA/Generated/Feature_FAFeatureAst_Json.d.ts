export type FieldAssignment = "A" | "B";

export type Feature = FeatureToResolve | FaFeature;

export interface FeatureToResolve {
    $ast: "FeatureToResolve";
    candidates: (Feature | null)[];
}

export interface FaFeature {
    $ast: "FaFeature";
    fa: FieldAssignment;
}

