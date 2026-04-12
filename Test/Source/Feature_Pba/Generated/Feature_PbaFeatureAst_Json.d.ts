export type Feature = FeatureToResolve | PbaFeature;

export interface Plus {
    $ast: "Plus";
}

export interface Lt {
    $ast: "Lt";
}

export interface Gt {
    $ast: "Gt";
}

export interface FeatureToResolve {
    $ast: "FeatureToResolve";
    candidates: (Feature | undefined)[];
}

export interface PbaFeature {
    $ast: "PbaFeature";
    lts: (Lt | undefined)[];
    gts: (Gt | undefined)[];
    optional: Plus | undefined;
    tail: Plus | undefined;
    tails: (Plus | undefined)[];
}

