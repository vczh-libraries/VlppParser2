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
    candidates: (Feature | null)[];
}

export interface PbaFeature {
    $ast: "PbaFeature";
    lts: (Lt | null)[];
    gts: (Gt | null)[];
    optional: Plus | null;
    tail: Plus | null;
    tails: (Plus | null)[];
}

export interface Pwa1Feature {
    $ast: "Pwa1Feature";
    pba: Feature | null;
    lts: (Lt | null)[];
    gts: (Gt | null)[];
}

