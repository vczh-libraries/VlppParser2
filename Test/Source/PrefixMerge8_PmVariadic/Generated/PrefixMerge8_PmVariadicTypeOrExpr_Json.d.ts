export type TypeOrExprOrOthers = TypeOrExprOrOthersToResolve | VariadicArgument | TypeOrExprToResolve | Name | MemberName | GenericName | GenericMemberName | CallExpr | CtorExpr | MulExpr | ConstType | PointerType | FunctionType;

export type TypeOrExpr = TypeOrExprToResolve | Name | MemberName | GenericName | GenericMemberName | CallExpr | CtorExpr | MulExpr | ConstType | PointerType | FunctionType;

export type QualifiedName = Name | MemberName | GenericName | GenericMemberName;

export type GenericQualifiedName = GenericName | GenericMemberName;

export interface GenericQualifiedName_Common {
    args: (TypeOrExprOrOthers | null)[];
}

export interface TypeOrExprOrOthersToResolve {
    $ast: "TypeOrExprOrOthersToResolve";
    candidates: (TypeOrExprOrOthers | null)[];
}

export interface VariadicArgument {
    $ast: "VariadicArgument";
    operand: TypeOrExpr | null;
}

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

export interface GenericName extends GenericQualifiedName_Common {
    $ast: "GenericName";
    name: string;
}

export interface GenericMemberName extends GenericQualifiedName_Common {
    $ast: "GenericMemberName";
    parent: QualifiedName | null;
    member: string;
}

export interface CallExpr {
    $ast: "CallExpr";
    func: TypeOrExpr | null;
    args: (TypeOrExprOrOthers | null)[];
}

export interface CtorExpr {
    $ast: "CtorExpr";
    type: TypeOrExpr | null;
    args: (TypeOrExprOrOthers | null)[];
}

export interface MulExpr {
    $ast: "MulExpr";
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
    args: (TypeOrExprOrOthers | null)[];
}

