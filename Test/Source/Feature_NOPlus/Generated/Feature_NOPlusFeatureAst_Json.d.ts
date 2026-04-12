export type Feature = FeatureToResolve | NestedOptionalFeature;

export interface Plus {
    $ast: "Plus";
}

export interface FeatureToResolve {
    $ast: "FeatureToResolve";
    candidates: (Feature | undefined)[];
}

export interface NestedOptionalFeature {
    $ast: "NestedOptionalFeature";
    optional: Plus | undefined;
    tail1: Plus | undefined;
    tail2: Plus | undefined;
    tail3: Plus | undefined;
    tails: (Plus | undefined)[];
}

