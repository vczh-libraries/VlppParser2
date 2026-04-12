export type Stat = DoStat | IfStat | BlockStat;

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

