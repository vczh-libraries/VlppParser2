export type TypeOrExprOrOthers = TypeOrExprOrOthersToResolve | VariadicArgument | TypeOrExprToResolve | Name | MemberName | GenericName | GenericMemberName | CallExpr | CtorExpr | MulExpr | ConstType | PointerType | FunctionType;

export type TypeOrExpr = TypeOrExprToResolve | Name | MemberName | GenericName | GenericMemberName | CallExpr | CtorExpr | MulExpr | ConstType | PointerType | FunctionType;

export type QualifiedName = Name | MemberName | GenericName | GenericMemberName;

export type GenericQualifiedName = GenericName | GenericMemberName;

export interface GenericQualifiedName_Common {
    args: (TypeOrExprOrOthers | undefined)[];
}

export interface TypeOrExprOrOthersToResolve {
    $ast: "TypeOrExprOrOthersToResolve";
    candidates: (TypeOrExprOrOthers | undefined)[];
}

export interface VariadicArgument {
    $ast: "VariadicArgument";
    operand: TypeOrExpr | undefined;
}

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

export interface GenericName extends GenericQualifiedName_Common {
    $ast: "GenericName";
    name: string;
}

export interface GenericMemberName extends GenericQualifiedName_Common {
    $ast: "GenericMemberName";
    parent: QualifiedName | undefined;
    member: string;
}

export interface CallExpr {
    $ast: "CallExpr";
    func: TypeOrExpr | undefined;
    args: (TypeOrExprOrOthers | undefined)[];
}

export interface CtorExpr {
    $ast: "CtorExpr";
    type: TypeOrExpr | undefined;
    args: (TypeOrExprOrOthers | undefined)[];
}

export interface MulExpr {
    $ast: "MulExpr";
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
    args: (TypeOrExprOrOthers | undefined)[];
}

