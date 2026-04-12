export type Feature = FeatureToResolve | NestedOptionalFeature;

export interface Plus {
    $ast: "Plus";
}

export interface FeatureToResolve {
    $ast: "FeatureToResolve";
    candidates: (Feature | null)[];
}

export interface NestedOptionalFeature {
    $ast: "NestedOptionalFeature";
    optional: Plus | null;
    tail1: Plus | null;
    tail2: Plus | null;
    tail3: Plus | null;
    tails: (Plus | null)[];
}

