export type Stat = StatToResolve | DoStat | IfStat | BlockStat;

export interface StatToResolve {
    $ast: "StatToResolve";
    candidates: (Stat | null)[];
}

export interface DoStat {
    $ast: "DoStat";
}

export interface IfStat {
    $ast: "IfStat";
    thenBranch: Stat | null;
    elseBranch: Stat | null;
}

export interface BlockStat {
    $ast: "BlockStat";
    stats: (Stat | null)[];
}

export interface Module {
    $ast: "Module";
    stat: Stat | null;
}

