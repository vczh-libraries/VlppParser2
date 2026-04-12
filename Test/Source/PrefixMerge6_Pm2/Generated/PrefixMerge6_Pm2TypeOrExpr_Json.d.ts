export type TypeOrExpr = TypeOrExprToResolve | Name | MemberName | CallExpr | CtorExpr | MulExpr | ThrowExpr | CommaExpr | ConstType | PointerType | FunctionType;

export type QualifiedName = Name | MemberName;

export interface TypeOrExprToResolve {
    $ast: "TypeOrExprToResolve";
    candidates: (TypeOrExpr | null)[];
}

export interface Name {
    $ast: "Name";
    name: string;
}

export interface MemberName {
    $ast: "MemberName";
    parent: QualifiedName | null;
    member: string;
}

export interface CallExpr {
    $ast: "CallExpr";
    func: TypeOrExpr | null;
    args: (TypeOrExpr | null)[];
}

export interface CtorExpr {
    $ast: "CtorExpr";
    type: TypeOrExpr | null;
    args: (TypeOrExpr | null)[];
}

export interface MulExpr {
    $ast: "MulExpr";
    first: TypeOrExpr | null;
    second: TypeOrExpr | null;
}

export interface ThrowExpr {
    $ast: "ThrowExpr";
    arg: (TypeOrExpr | null)[];
}

export interface CommaExpr {
    $ast: "CommaExpr";
    first: TypeOrExpr | null;
    second: TypeOrExpr | null;
}

export interface ConstType {
    $ast: "ConstType";
    type: TypeOrExpr | null;
}

export interface PointerType {
    $ast: "PointerType";
    type: TypeOrExpr | null;
}

export interface FunctionType {
    $ast: "FunctionType";
    returnType: TypeOrExpr | null;
    args: (TypeOrExpr | null)[];
}

