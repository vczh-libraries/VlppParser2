export type Stat = DoStat | IfStat | BlockStat;

export type IfContent = IfContentToResolve | IfContentCandidate;

export interface DoStat {
    $ast: "DoStat";
}

export interface IfContentToResolve {
    $ast: "IfContentToResolve";
    candidates: (IfContent | undefined)[];
}

export interface IfContentCandidate {
    $ast: "IfContentCandidate";
    thenBranch: Stat | undefined;
    elseBranch: Stat | undefined;
}

export interface IfStat {
    $ast: "IfStat";
    content: IfContent | undefined;
}

export interface BlockStat {
    $ast: "BlockStat";
    stats: (Stat | undefined)[];
}

export interface Module {
    $ast: "Module";
    stat: Stat | undefined;
}

