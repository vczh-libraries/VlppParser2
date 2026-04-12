export type NameKinds = "Normal" | "Enum" | "EnumClass" | "Class" | "Struct" | "Union" | "Dtor" | "UserDefinedLiteral";

export type Operators = "New" | "NewArray" | "Delete" | "DeleteArray" | "Comma" | "RoundBracket" | "Parantheses" | "Bracket" | "PointerDeref" | "Pointer" | "MemberDeref" | "Member" | "Compare" | "EQ" | "NE" | "LT" | "LE" | "GT" | "GE" | "Not" | "Revert" | "Xor" | "And" | "BitwiseAnd" | "Or" | "BitwiseOr" | "Mul" | "Div" | "Mod" | "Plus" | "Increase" | "Minus" | "Decrease" | "LeftShift" | "RightShift" | "Assign" | "RevertAssign" | "XorAssign" | "AndAssign" | "OrAssign" | "MulAssign" | "DivAssign" | "ModAssign" | "PlusAssign" | "MinusAssign" | "LeftShiftAssign" | "RightShiftAssign";

export type QualifiedNameKinds = "Root" | "Context" | "Auto" | "Decltype" | "Member";

export type PrimitiveExprLiteralKinds = "True" | "False" | "Nullptr" | "This" | "__Nullptr";

export type NumericExprLiteralKinds = "Integer" | "Hex" | "Binary" | "Float" | "FloatHex" | "Char";

export type StringLiteralKinds = "String" | "Macro_LPREFIX";

export type LambdaCaptureObjectKinds = "Default" | "This" | "Id" | "PackId" | "PackInit";

export type LambdaCaptureRefeferenceKinds = "Ref" | "Copy";

export type OperatorScope = "Root" | "Context";

export type OperatorArray = "Array" | "NotArray";

export type CallKinds = "Parenthesis" | "Brace";

export type PrimitiveTypeKinds = "Neutral" | "Signed" | "Unsigned";

export type AdvancedTypeKinds = "LRef" | "RRef" | "Const" | "Volatile" | "Pointer" | "Pointer32" | "Pointer64" | "Member" | "AlignAs";

export type ClassKind = "Class" | "Struct" | "Union";

export type ClassAccessor = "Default" | "Private" | "Protected" | "Public";

export type EnumKind = "Enum" | "EnumClass";

export type TypeOrExprOrOthers = TypeOrExprOrOthersToResolve | DeclarationToResolve | VariablesDeclaration | ClassDeclaration | EnumDeclaration | TemplateDeclaration | StaticAssertDeclaration | TypedefDeclaration | ExternDeclaration | NamespaceDeclaration | UsingNamespaceDeclaration | UsingValueDeclaration | UsingTypeDeclaration | FriendTypeDeclaration | TypeOrExprToResolve | PrimitiveExprLiteral | NumericExprLiteral | StringLiteral | LambdaExpr | ParenthesisExpr | BraceExpr | CastExpr | SysFuncExpr | SizeofExpr | DeleteExpr | NewExpr | PrefixUnaryExpr | PostfixUnaryExpr | IndexExpr | CallExpr | BinaryExpr | IfExpr | ThrowExpr | VariadicExpr | PrimitiveType | ConstType | VolatileType | QualifiedName | DeclaratorType | OrdinaryGenericParameter | GenericArgument;

export type Declaration = DeclarationToResolve | VariablesDeclaration | ClassDeclaration | EnumDeclaration | TemplateDeclaration | StaticAssertDeclaration | TypedefDeclaration | ExternDeclaration | NamespaceDeclaration | UsingNamespaceDeclaration | UsingValueDeclaration | UsingTypeDeclaration | FriendTypeDeclaration;

export type DeclarationCommon = VariablesDeclaration | ClassDeclaration | EnumDeclaration | TemplateDeclaration | StaticAssertDeclaration | TypedefDeclaration | ExternDeclaration | NamespaceDeclaration | UsingNamespaceDeclaration | UsingValueDeclaration | UsingTypeDeclaration | FriendTypeDeclaration;

export type TypeOrExpr = TypeOrExprToResolve | PrimitiveExprLiteral | NumericExprLiteral | StringLiteral | LambdaExpr | ParenthesisExpr | BraceExpr | CastExpr | SysFuncExpr | SizeofExpr | DeleteExpr | NewExpr | PrefixUnaryExpr | PostfixUnaryExpr | IndexExpr | CallExpr | BinaryExpr | IfExpr | ThrowExpr | VariadicExpr | PrimitiveType | ConstType | VolatileType | QualifiedName | DeclaratorType;

export type ExprOnly = PrimitiveExprLiteral | NumericExprLiteral | StringLiteral | LambdaExpr | ParenthesisExpr | BraceExpr | CastExpr | SysFuncExpr | SizeofExpr | DeleteExpr | NewExpr | PrefixUnaryExpr | PostfixUnaryExpr | IndexExpr | CallExpr | BinaryExpr | IfExpr | ThrowExpr | VariadicExpr;

export type TypeOnly = PrimitiveType | ConstType | VolatileType;

export type Statement = StatementToResolve | EmptyStat | BlockStat | ExprStat | DeclStat | BreakStat | ContinueStat | ReturnStat | LabelStat | GotoStat | CaseStat | DefaultStat | __LeaveStat | WhileStat | DoWhileStat | IfElseStat | ForStat | SwitchStat | TryStat | __TryStat;

export type Identifier = NameIdentifier | OperatorIdentifier | OperatorTypeIdentifier;

export type DeclaratorFunctionPart = DeclaratorFunctionPartToResolve | DeclaratorFunctionPartCommon;

export type Declarator = DeclaratorToResolve | DeclaratorCommon;

export type VarInit = VarValueInit | VarParanthesisInit | VarBraceInit | VarStatInit;

export type DeclaratorVariablePart = DeclaratorVariablePartToResolve | DeclaratorVariablePartCommon;

export type ForStatConditionPart = ForStatLoopCondition | ForStatIterateCondition;

export interface DeclarationCommon_Common {
    keywords: (DeclaratorKeyword | null)[];
}

export interface TypeOrExprOrOthersToResolve {
    $ast: "TypeOrExprOrOthersToResolve";
    candidates: (TypeOrExprOrOthers | null)[];
}

export interface DeclarationToResolve {
    $ast: "DeclarationToResolve";
    candidates: (Declaration | null)[];
}

export interface TypeOrExprToResolve {
    $ast: "TypeOrExprToResolve";
    candidates: (TypeOrExpr | null)[];
}

export interface StatementToResolve {
    $ast: "StatementToResolve";
    candidates: (Statement | null)[];
}

export interface OrdinaryGenericParameter {
    $ast: "OrdinaryGenericParameter";
    genericHeader: GenericHeader | null;
    typenameToken: string;
    variadic: string;
    id: Identifier | null;
    init: TypeOrExpr | null;
}

export interface GenericHeader {
    $ast: "GenericHeader";
    parameters: (TypeOrExprOrOthers | null)[];
}

export interface File {
    $ast: "File";
    decls: (Declaration | null)[];
}

export interface NameIdentifier {
    $ast: "NameIdentifier";
    kind: NameKinds;
    name: string;
}

export interface OperatorIdentifier {
    $ast: "OperatorIdentifier";
    op: Operators;
}

export interface OperatorTypeIdentifier {
    $ast: "OperatorTypeIdentifier";
    type: TypeOrExprOrOthers | null;
}

export interface GenericArgument {
    $ast: "GenericArgument";
    argument: TypeOrExpr | null;
    variadic: string;
}

export interface GenericArguments {
    $ast: "GenericArguments";
    arguments: (TypeOrExprOrOthers | null)[];
}

export interface QualifiedName {
    $ast: "QualifiedName";
    kind: QualifiedNameKinds;
    expr: TypeOrExpr | null;
    parent: QualifiedName | null;
    id: Identifier | null;
    arguments: GenericArguments | null;
}

export interface PrimitiveExprLiteral {
    $ast: "PrimitiveExprLiteral";
    kind: PrimitiveExprLiteralKinds;
}

export interface NumericExprLiteral {
    $ast: "NumericExprLiteral";
    kind: NumericExprLiteralKinds;
    literal: string;
}

export interface StringLiteralFragment {
    $ast: "StringLiteralFragment";
    kind: StringLiteralKinds;
    literal: string;
}

export interface StringLiteral {
    $ast: "StringLiteral";
    fragments: (StringLiteralFragment | null)[];
}

export interface LambdaCapture {
    $ast: "LambdaCapture";
    objKind: LambdaCaptureObjectKinds;
    refKind: LambdaCaptureRefeferenceKinds;
    id: Identifier | null;
    init: VarInit | null;
}

export interface LambdaExpr {
    $ast: "LambdaExpr";
    captures: (LambdaCapture | null)[];
    genericHeader: GenericHeader | null;
    functionHeader: DeclaratorFunctionPart | null;
    stat: Statement | null;
}

export interface ParenthesisExpr {
    $ast: "ParenthesisExpr";
    expr: TypeOrExpr | null;
}

export interface BraceExpr {
    $ast: "BraceExpr";
    arguments: (TypeOrExpr | null)[];
}

export interface CastExpr {
    $ast: "CastExpr";
    keyword: string;
    type: TypeOrExpr | null;
    expr: TypeOrExpr | null;
}

export interface SysFuncExpr {
    $ast: "SysFuncExpr";
    keyword: string;
    variadic: string;
    argument: TypeOrExpr | null;
}

export interface SizeofExpr {
    $ast: "SizeofExpr";
    argument: TypeOrExpr | null;
    variadic: string;
}

export interface DeleteExpr {
    $ast: "DeleteExpr";
    scope: OperatorScope;
    array: OperatorArray;
    argument: TypeOrExpr | null;
}

export interface NewExpr {
    $ast: "NewExpr";
    scope: OperatorScope;
    type: (TypeOrExpr | null)[];
    placementArguments: (TypeOrExpr | null)[];
    arrayArguments: (TypeOrExpr | null)[];
    init: VarInit | null;
}

export interface PrefixUnaryExpr {
    $ast: "PrefixUnaryExpr";
    op: Operators;
    operand: TypeOrExpr | null;
}

export interface PostfixUnaryExpr {
    $ast: "PostfixUnaryExpr";
    op: Operators;
    operand: TypeOrExpr | null;
}

export interface IndexExpr {
    $ast: "IndexExpr";
    operand: TypeOrExpr | null;
    index: TypeOrExpr | null;
}

export interface CallExpr {
    $ast: "CallExpr";
    kind: CallKinds;
    operand: TypeOrExpr | null;
    arguments: (TypeOrExpr | null)[];
}

export interface BinaryExpr {
    $ast: "BinaryExpr";
    op: Operators;
    left: TypeOrExpr | null;
    right: TypeOrExpr | null;
}

export interface IfExpr {
    $ast: "IfExpr";
    condition: TypeOrExpr | null;
    trueBranch: TypeOrExpr | null;
    falseBranch: TypeOrExpr | null;
}

export interface ThrowExpr {
    $ast: "ThrowExpr";
    argument: TypeOrExpr | null;
}

export interface VariadicExpr {
    $ast: "VariadicExpr";
    operand: TypeOrExpr | null;
    variadic: string;
}

export interface PrimitiveType {
    $ast: "PrimitiveType";
    kind: PrimitiveTypeKinds;
    literal1: string;
    literal2: string;
}

export interface ConstType {
    $ast: "ConstType";
    type: TypeOrExpr | null;
}

export interface VolatileType {
    $ast: "VolatileType";
    type: TypeOrExpr | null;
}

export interface AdvancedType {
    $ast: "AdvancedType";
    kind: AdvancedTypeKinds;
    argument: TypeOrExpr | null;
}

export interface DeclaratorKeyword {
    $ast: "DeclaratorKeyword";
    keyword: string;
}

export interface FunctionKeyword {
    $ast: "FunctionKeyword";
    keyword: string;
    arguments: (TypeOrExpr | null)[];
}

export interface DeclaratorFunctionPartToResolve {
    $ast: "DeclaratorFunctionPartToResolve";
    candidates: (DeclaratorFunctionPart | null)[];
}

export interface DeclaratorFunctionPartCommon {
    $ast: "DeclaratorFunctionPartCommon";
    parameters: (TypeOrExprOrOthers | null)[];
    variadic: string;
    keywords: (FunctionKeyword | null)[];
    deferredType: TypeOrExpr | null;
}

export interface DeclaratorArrayPart {
    $ast: "DeclaratorArrayPart";
    argument: TypeOrExpr | null;
}

export interface DeclaratorToResolve {
    $ast: "DeclaratorToResolve";
    candidates: (Declarator | null)[];
}

export interface DeclaratorCommon {
    $ast: "DeclaratorCommon";
    keywords: (DeclaratorKeyword | null)[];
    advancedTypes: (AdvancedType | null)[];
    variadic: string;
    id: Identifier | null;
    arguments: GenericArguments | null;
    bitfield: TypeOrExpr | null;
    innerDeclarator: Declarator | null;
    funcPart: DeclaratorFunctionPart | null;
    arrayParts: (DeclaratorArrayPart | null)[];
}

export interface DeclaratorType {
    $ast: "DeclaratorType";
    keywords: (DeclaratorKeyword | null)[];
    type: TypeOrExpr | null;
    declarator: Declarator | null;
}

export interface VarValueInit {
    $ast: "VarValueInit";
    expr: TypeOrExpr | null;
}

export interface VarParanthesisInit {
    $ast: "VarParanthesisInit";
    arguments: (TypeOrExpr | null)[];
}

export interface VarBraceInit {
    $ast: "VarBraceInit";
    arguments: (TypeOrExpr | null)[];
}

export interface VarStatInitItem {
    $ast: "VarStatInitItem";
    name: string;
    init: VarInit | null;
}

export interface VarStatInit {
    $ast: "VarStatInit";
    initItems: (VarStatInitItem | null)[];
    stat: BlockStat | null;
}

export interface DeclaratorVariablePartToResolve {
    $ast: "DeclaratorVariablePartToResolve";
    candidates: (DeclaratorVariablePart | null)[];
}

export interface DeclaratorVariablePartCommon {
    $ast: "DeclaratorVariablePartCommon";
    declarator: Declarator | null;
    init: VarInit | null;
    nextVarPart: DeclaratorVariablePart | null;
}

export interface VariablesDeclaration extends DeclarationCommon_Common {
    $ast: "VariablesDeclaration";
    type: TypeOrExpr | null;
    firstVarPart: DeclaratorVariablePart | null;
}

export interface ClassInheritance {
    $ast: "ClassInheritance";
    accessor: ClassAccessor;
    variadic: string;
    type: TypeOrExpr | null;
}

export interface ClassMemberPart {
    $ast: "ClassMemberPart";
    accessor: ClassAccessor;
    decls: (Declaration | null)[];
}

export interface ClassBody {
    $ast: "ClassBody";
    inheritances: (ClassInheritance | null)[];
    memberParts: (ClassMemberPart | null)[];
    firstVarPart: DeclaratorVariablePart | null;
}

export interface ClassDeclaration extends DeclarationCommon_Common {
    $ast: "ClassDeclaration";
    kind: ClassKind;
    name: string;
    arguments: GenericArguments | null;
    body: ClassBody | null;
}

export interface EnumItem {
    $ast: "EnumItem";
    name: string;
    expr: TypeOrExpr | null;
}

export interface EnumBody {
    $ast: "EnumBody";
    items: (EnumItem | null)[];
    firstVarPart: DeclaratorVariablePart | null;
}

export interface EnumDeclaration extends DeclarationCommon_Common {
    $ast: "EnumDeclaration";
    kind: EnumKind;
    name: string;
    type: TypeOrExpr | null;
    body: EnumBody | null;
}

export interface TemplateDeclaration extends DeclarationCommon_Common {
    $ast: "TemplateDeclaration";
    genericHeader: GenericHeader | null;
    decl: Declaration | null;
}

export interface StaticAssertDeclaration extends DeclarationCommon_Common {
    $ast: "StaticAssertDeclaration";
    expr: TypeOrExpr | null;
    message: TypeOrExpr | null;
}

export interface TypedefDeclaration extends DeclarationCommon_Common {
    $ast: "TypedefDeclaration";
    decl: Declaration | null;
}

export interface ExternDeclaration extends DeclarationCommon_Common {
    $ast: "ExternDeclaration";
    decls: (Declaration | null)[];
}

export interface NamespaceName {
    $ast: "NamespaceName";
    name: string;
}

export interface NamespaceDeclaration extends DeclarationCommon_Common {
    $ast: "NamespaceDeclaration";
    names: (NamespaceName | null)[];
    decls: (Declaration | null)[];
}

export interface UsingNamespaceDeclaration extends DeclarationCommon_Common {
    $ast: "UsingNamespaceDeclaration";
    names: (NamespaceName | null)[];
}

export interface UsingValueDeclaration extends DeclarationCommon_Common {
    $ast: "UsingValueDeclaration";
    typenameKeyword: string;
    name: QualifiedName | null;
}

export interface UsingTypeDeclaration extends DeclarationCommon_Common {
    $ast: "UsingTypeDeclaration";
    name: string;
    type: TypeOrExpr | null;
}

export interface FriendTypeDeclaration extends DeclarationCommon_Common {
    $ast: "FriendTypeDeclaration";
    type: QualifiedName | null;
}

export interface EmptyStat {
    $ast: "EmptyStat";
}

export interface BlockStat {
    $ast: "BlockStat";
    statements: (Statement | null)[];
}

export interface ExprStat {
    $ast: "ExprStat";
    expr: TypeOrExpr | null;
}

export interface DeclStat {
    $ast: "DeclStat";
    decl: Declaration | null;
}

export interface BreakStat {
    $ast: "BreakStat";
}

export interface ContinueStat {
    $ast: "ContinueStat";
}

export interface ReturnStat {
    $ast: "ReturnStat";
    expr: TypeOrExpr | null;
}

export interface LabelStat {
    $ast: "LabelStat";
    label: string;
    stat: Statement | null;
}

export interface GotoStat {
    $ast: "GotoStat";
    label: string;
}

export interface CaseStat {
    $ast: "CaseStat";
    expr: TypeOrExpr | null;
    stat: Statement | null;
}

export interface DefaultStat {
    $ast: "DefaultStat";
    stat: Statement | null;
}

export interface __LeaveStat {
    $ast: "__LeaveStat";
}

export interface WhileStat {
    $ast: "WhileStat";
    condition: TypeOrExprOrOthers | null;
    stat: Statement | null;
}

export interface DoWhileStat {
    $ast: "DoWhileStat";
    condition: TypeOrExpr | null;
    stat: Statement | null;
}

export interface IfElseStat {
    $ast: "IfElseStat";
    varsDecl: VariablesDeclaration | null;
    condition: TypeOrExprOrOthers | null;
    trueStat: Statement | null;
    falseStat: Statement | null;
}

export interface ForStatLoopCondition {
    $ast: "ForStatLoopCondition";
    varsDecl: TypeOrExprOrOthers | null;
    condition: TypeOrExpr | null;
    sideEffect: TypeOrExpr | null;
}

export interface ForStatIterateCondition {
    $ast: "ForStatIterateCondition";
    decl: VariablesDeclaration | null;
    collection: TypeOrExpr | null;
}

export interface ForStat {
    $ast: "ForStat";
    conditionPart: ForStatConditionPart | null;
    stat: Statement | null;
}

export interface SwitchStat {
    $ast: "SwitchStat";
    condition: TypeOrExprOrOthers | null;
    stat: Statement | null;
}

export interface TryStatCatchPart {
    $ast: "TryStatCatchPart";
    decl: TypeOrExprOrOthers | null;
    stat: Statement | null;
}

export interface TryStat {
    $ast: "TryStat";
    tryStat: Statement | null;
    catchParts: (TryStatCatchPart | null)[];
}

export interface __TryStat {
    $ast: "__TryStat";
    tryStat: Statement | null;
    exceptStat: Statement | null;
    finallyStat: Statement | null;
    filter: TypeOrExpr | null;
}

