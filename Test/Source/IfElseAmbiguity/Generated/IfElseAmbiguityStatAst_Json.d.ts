export type Stat = DoStat | IfStat | BlockStat;

export type IfContent = IfContentToResolve | IfContentCandidate;

export interface DoStat {
    $ast: "DoStat";
}

export interface IfContentToResolve {
    $ast: "IfContentToResolve";
    candidates: (IfContent | null)[];
}

export interface IfContentCandidate {
    $ast: "IfContentCandidate";
    thenBranch: Stat | null;
    elseBranch: Stat | null;
}

export interface IfStat {
    $ast: "IfStat";
    content: IfContent | null;
}

export interface BlockStat {
    $ast: "BlockStat";
    stats: (Stat | null)[];
}

export interface Module {
    $ast: "Module";
    stat: Stat | null;
}

