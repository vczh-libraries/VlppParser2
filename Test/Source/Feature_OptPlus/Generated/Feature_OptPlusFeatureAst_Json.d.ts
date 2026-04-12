export type OptionalProprity = "Equal" | "PreferTake" | "PreferSkip";

export type Feature = FeatureToResolve | OptionalFeature;

export interface Plus {
    $ast: "Plus";
}

export interface FeatureToResolve {
    $ast: "FeatureToResolve";
    candidates: (Feature | null)[];
}

export interface OptionalFeature {
    $ast: "OptionalFeature";
    priority: OptionalProprity;
    optional: Plus | null;
    loop: (Plus | null)[];
}

