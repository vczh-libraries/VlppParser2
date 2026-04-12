export type BranchType = "Plus" | "Minus" | "NoCompetition";

export type Feature = FeatureToResolve | BranchedOptionalFeature;

export interface Plus {
    $ast: "Plus";
}

export interface FeatureToResolve {
    $ast: "FeatureToResolve";
    candidates: (Feature | undefined)[];
}

export interface BranchedOptionalFeature {
    $ast: "BranchedOptionalFeature";
    type: BranchType;
    optional: Plus | undefined;
    tails: (Plus | undefined)[];
}

