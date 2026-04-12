export type Feature = FeatureToResolve | PbaFeature | Pwa1Feature;

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

export interface Pwa1Feature {
    $ast: "Pwa1Feature";
    pba: Feature | undefined;
    lts: (Lt | undefined)[];
    gts: (Gt | undefined)[];
}

