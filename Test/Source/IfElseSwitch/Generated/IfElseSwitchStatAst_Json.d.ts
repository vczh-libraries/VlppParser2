export type Stat = DoStat | IfStat | BlockStat;

export interface DoStat {
    $ast: "DoStat";
}

export interface IfStat {
    $ast: "IfStat";
    thenBranch: Stat | undefined;
    elseBranch: Stat | undefined;
}

export interface BlockStat {
    $ast: "BlockStat";
    stats: (Stat | undefined)[];
}

export interface Module {
    $ast: "Module";
    stat: Stat | undefined;
}

