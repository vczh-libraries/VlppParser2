export type OptionalProprity = "Equal" | "PreferTake" | "PreferSkip";

export type Feature = FeatureToResolve | OptionalFeature;

export interface Plus {
    $ast: "Plus";
}

export interface FeatureToResolve {
    $ast: "FeatureToResolve";
    candidates: (Feature | undefined)[];
}

export interface OptionalFeature {
    $ast: "OptionalFeature";
    priority: OptionalProprity;
    optional: Plus | undefined;
    loop: (Plus | undefined)[];
}

