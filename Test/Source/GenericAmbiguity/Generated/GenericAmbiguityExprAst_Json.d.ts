export type PostfixOp = "Add" | "Sub" | "Increment";

export type BinaryOp = "LT" | "GT";

export type Expr = ExprToResolve | RefExpr | GenericExpr | CallExpr | PostfixExpr | DecrementExpr | BinaryExpr;

export interface ExprToResolve {
    $ast: "ExprToResolve";
    candidates: (Expr | null)[];
}

export interface RefExpr {
    $ast: "RefExpr";
    name: string;
}

export interface GenericExpr {
    $ast: "GenericExpr";
    name: string;
    args: (Expr | null)[];
}

export interface CallExpr {
    $ast: "CallExpr";
    func: Expr | null;
    args: (Expr | null)[];
}

export interface PostfixExpr {
    $ast: "PostfixExpr";
    op: PostfixOp;
    expr: Expr | null;
}

export interface DecrementExpr {
    $ast: "DecrementExpr";
    expr: Expr | null;
}

export interface BinaryExpr {
    $ast: "BinaryExpr";
    op: BinaryOp;
    left: Expr | null;
    right: Expr | null;
}

export interface Module {
    $ast: "Module";
    expr: Expr | null;
}

