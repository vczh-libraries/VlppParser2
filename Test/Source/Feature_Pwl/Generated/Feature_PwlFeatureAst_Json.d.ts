export type Feature = FeatureToResolve | PwlFeature;

export interface Plus {
    $ast: "Plus";
}

export interface Lt {
    $ast: "Lt";
}

export interface FeatureToResolve {
    $ast: "FeatureToResolve";
    candidates: (Feature | undefined)[];
}

export interface PwlFeature {
    $ast: "PwlFeature";
    prefix: (Plus | undefined)[];
    one: (Lt | undefined)[];
    two: (Lt | undefined)[];
    prev: PwlFeature | undefined;
}

