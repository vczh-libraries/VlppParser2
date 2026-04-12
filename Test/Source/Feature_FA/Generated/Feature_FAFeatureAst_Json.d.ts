export type FieldAssignment = "A" | "B";

export type Feature = FeatureToResolve | FaFeature;

export interface FeatureToResolve {
    $ast: "FeatureToResolve";
    candidates: (Feature | undefined)[];
}

export interface FaFeature {
    $ast: "FaFeature";
    fa: FieldAssignment;
}

