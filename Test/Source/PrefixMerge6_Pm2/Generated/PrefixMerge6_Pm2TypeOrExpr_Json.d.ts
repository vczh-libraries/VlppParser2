export type TypeOrExpr = TypeOrExprToResolve | Name | MemberName | CallExpr | CtorExpr | MulExpr | ThrowExpr | CommaExpr | ConstType | PointerType | FunctionType;

export type QualifiedName = Name | MemberName;

export interface TypeOrExprToResolve {
    $ast: "TypeOrExprToResolve";
    candidates: (TypeOrExpr | undefined)[];
}

export interface Name {
    $ast: "Name";
    name: string;
}

export interface MemberName {
    $ast: "MemberName";
    parent: QualifiedName | undefined;
    member: string;
}

export interface CallExpr {
    $ast: "CallExpr";
    func: TypeOrExpr | undefined;
    args: (TypeOrExpr | undefined)[];
}

export interface CtorExpr {
    $ast: "CtorExpr";
    type: TypeOrExpr | undefined;
    args: (TypeOrExpr | undefined)[];
}

export interface MulExpr {
    $ast: "MulExpr";
    first: TypeOrExpr | undefined;
    second: TypeOrExpr | undefined;
}

export interface ThrowExpr {
    $ast: "ThrowExpr";
    arg: (TypeOrExpr | undefined)[];
}

export interface CommaExpr {
    $ast: "CommaExpr";
    first: TypeOrExpr | undefined;
    second: TypeOrExpr | undefined;
}

export interface ConstType {
    $ast: "ConstType";
    type: TypeOrExpr | undefined;
}

export interface PointerType {
    $ast: "PointerType";
    type: TypeOrExpr | undefined;
}

export interface FunctionType {
    $ast: "FunctionType";
    returnType: TypeOrExpr | undefined;
    args: (TypeOrExpr | undefined)[];
}

