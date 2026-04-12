export type Feature = FeatureToResolve | PwlFeature;

export interface Plus {
    $ast: "Plus";
}

export interface Lt {
    $ast: "Lt";
}

export interface FeatureToResolve {
    $ast: "FeatureToResolve";
    candidates: (Feature | null)[];
}

export interface PwlFeature {
    $ast: "PwlFeature";
    prefix: (Plus | null)[];
    one: (Lt | null)[];
    two: (Lt | null)[];
    prev: PwlFeature | null;
}

