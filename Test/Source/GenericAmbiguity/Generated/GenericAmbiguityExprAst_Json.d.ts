export type PostfixOp = "Add" | "Sub" | "Increment";

export type BinaryOp = "LT" | "GT";

export type Expr = ExprToResolve | RefExpr | GenericExpr | CallExpr | PostfixExpr | DecrementExpr | BinaryExpr;

export interface ExprToResolve {
    $ast: "ExprToResolve";
    candidates: (Expr | undefined)[];
}

export interface RefExpr {
    $ast: "RefExpr";
    name: string;
}

export interface GenericExpr {
    $ast: "GenericExpr";
    name: string;
    args: (Expr | undefined)[];
}

export interface CallExpr {
    $ast: "CallExpr";
    func: Expr | undefined;
    args: (Expr | undefined)[];
}

export interface PostfixExpr {
    $ast: "PostfixExpr";
    op: PostfixOp;
    expr: Expr | undefined;
}

export interface DecrementExpr {
    $ast: "DecrementExpr";
    expr: Expr | undefined;
}

export interface BinaryExpr {
    $ast: "BinaryExpr";
    op: BinaryOp;
    left: Expr | undefined;
    right: Expr | undefined;
}

export interface Module {
    $ast: "Module";
    expr: Expr | undefined;
}

