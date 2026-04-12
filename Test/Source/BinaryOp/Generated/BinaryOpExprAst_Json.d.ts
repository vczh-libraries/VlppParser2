export type BinaryOp = "Add" | "Mul" | "Exp" | "Assign" | "Try" | "Dollar";

export type Expr = RefExpr | BinaryExpr;

export interface RefExpr {
    $ast: "RefExpr";
    name: string;
}

export interface BinaryExpr {
    $ast: "BinaryExpr";
    op: BinaryOp;
    left: Expr | undefined;
    right: Expr | undefined;
}

