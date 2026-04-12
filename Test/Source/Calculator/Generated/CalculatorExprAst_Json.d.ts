export type UnaryOp = "Positive" | "Negative";

export type BinaryOp = "Add" | "Minus" | "Multiply" | "Divid" | "GT" | "GE" | "LT" | "LE" | "EQ" | "NE";

export type Expr = NumExpr | Ref | True | False | Func | Call | LetExpr | Unary | Binary;

export type Expandable = LetExpr | Unary | Binary;

export interface Expandable_Common {
    expanded: Expr | null;
}

export interface NumExpr {
    $ast: "NumExpr";
    value: string;
}

export interface Ref {
    $ast: "Ref";
    name: string;
}

export interface True {
    $ast: "True";
}

export interface False {
    $ast: "False";
}

export interface Arg {
    $ast: "Arg";
    name: string;
}

export interface Func {
    $ast: "Func";
    args: (Arg | null)[];
    value: Expr | null;
}

export interface Call {
    $ast: "Call";
    func: Expr | null;
    args: (Expr | null)[];
}

export interface LetExpr extends Expandable_Common {
    $ast: "LetExpr";
    name: string;
    value: Expr | null;
    result: Expr | null;
}

export interface Unary extends Expandable_Common {
    $ast: "Unary";
    op: UnaryOp;
    operand: Expr | null;
}

export interface Binary extends Expandable_Common {
    $ast: "Binary";
    op: BinaryOp;
    left: Expr | null;
    right: Expr | null;
}

export interface Import {
    $ast: "Import";
    name: string;
}

export interface Module {
    $ast: "Module";
    imports: (Import | null)[];
    exported: Expr | null;
}

