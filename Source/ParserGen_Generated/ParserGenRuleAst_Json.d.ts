export type SwitchValue = "False" | "True";

export type RefType = "Id" | "Literal" | "ConditionalLiteral";

export type OptionalPriority = "Equal" | "PreferTake" | "PreferSkip";

export type AssignmentType = "Strong" | "Weak";

export type Condition = RefCondition | NotCondition | AndCondition | OrCondition;

export type Syntax = RefSyntax | UseSyntax | LoopSyntax | OptionalSyntax | SequenceSyntax | AlternativeSyntax | PushConditionSyntax | TestConditionSyntax;

export type Clause = CreateClause | PartialClause | ReuseClause;

export interface RefCondition {
    $ast: "RefCondition";
    name: string;
}

export interface NotCondition {
    $ast: "NotCondition";
    condition: Condition | undefined;
}

export interface AndCondition {
    $ast: "AndCondition";
    first: Condition | undefined;
    second: Condition | undefined;
}

export interface OrCondition {
    $ast: "OrCondition";
    first: Condition | undefined;
    second: Condition | undefined;
}

export interface SwitchItem {
    $ast: "SwitchItem";
    name: string;
    value: SwitchValue;
}

export interface RefSyntax {
    $ast: "RefSyntax";
    refType: RefType;
    literal: string;
    field: string;
}

export interface UseSyntax {
    $ast: "UseSyntax";
    name: string;
}

export interface LoopSyntax {
    $ast: "LoopSyntax";
    syntax: Syntax | undefined;
    delimiter: Syntax | undefined;
}

export interface OptionalSyntax {
    $ast: "OptionalSyntax";
    priority: OptionalPriority;
    syntax: Syntax | undefined;
}

export interface SequenceSyntax {
    $ast: "SequenceSyntax";
    first: Syntax | undefined;
    second: Syntax | undefined;
}

export interface AlternativeSyntax {
    $ast: "AlternativeSyntax";
    first: Syntax | undefined;
    second: Syntax | undefined;
}

export interface PushConditionSyntax {
    $ast: "PushConditionSyntax";
    switches: (SwitchItem | undefined)[];
    syntax: Syntax | undefined;
}

export interface TestConditionBranch {
    $ast: "TestConditionBranch";
    condition: Condition | undefined;
    syntax: Syntax | undefined;
}

export interface TestConditionSyntax {
    $ast: "TestConditionSyntax";
    branches: (TestConditionBranch | undefined)[];
}

export interface Assignment {
    $ast: "Assignment";
    type: AssignmentType;
    field: string;
    value: string;
}

export interface CreateClause {
    $ast: "CreateClause";
    type: string;
    syntax: Syntax | undefined;
    assignments: (Assignment | undefined)[];
}

export interface PartialClause {
    $ast: "PartialClause";
    type: string;
    syntax: Syntax | undefined;
    assignments: (Assignment | undefined)[];
}

export interface ReuseClause {
    $ast: "ReuseClause";
    syntax: Syntax | undefined;
    assignments: (Assignment | undefined)[];
}

export interface Rule {
    $ast: "Rule";
    attPublic: string;
    attParser: string;
    name: string;
    type: string;
    clauses: (Clause | undefined)[];
}

export interface SyntaxFile {
    $ast: "SyntaxFile";
    switches: (SwitchItem | undefined)[];
    rules: (Rule | undefined)[];
}

