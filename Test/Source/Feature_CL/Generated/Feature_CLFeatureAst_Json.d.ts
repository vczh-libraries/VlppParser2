export type Feature = FeatureToResolve | ClFeature;

export interface FeatureToResolve {
    $ast: "FeatureToResolve";
    candidates: (Feature | undefined)[];
}

export interface ClFeature {
    $ast: "ClFeature";
    id: string;
}

