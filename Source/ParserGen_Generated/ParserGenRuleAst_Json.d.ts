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
    condition: Condition | null;
}

export interface AndCondition {
    $ast: "AndCondition";
    first: Condition | null;
    second: Condition | null;
}

export interface OrCondition {
    $ast: "OrCondition";
    first: Condition | null;
    second: Condition | null;
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
    syntax: Syntax | null;
    delimiter: Syntax | null;
}

export interface OptionalSyntax {
    $ast: "OptionalSyntax";
    priority: OptionalPriority;
    syntax: Syntax | null;
}

export interface SequenceSyntax {
    $ast: "SequenceSyntax";
    first: Syntax | null;
    second: Syntax | null;
}

export interface AlternativeSyntax {
    $ast: "AlternativeSyntax";
    first: Syntax | null;
    second: Syntax | null;
}

export interface PushConditionSyntax {
    $ast: "PushConditionSyntax";
    switches: (SwitchItem | null)[];
    syntax: Syntax | null;
}

export interface TestConditionBranch {
    $ast: "TestConditionBranch";
    condition: Condition | null;
    syntax: Syntax | null;
}

export interface TestConditionSyntax {
    $ast: "TestConditionSyntax";
    branches: (TestConditionBranch | null)[];
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
    syntax: Syntax | null;
    assignments: (Assignment | null)[];
}

export interface PartialClause {
    $ast: "PartialClause";
    type: string;
    syntax: Syntax | null;
    assignments: (Assignment | null)[];
}

export interface ReuseClause {
    $ast: "ReuseClause";
    syntax: Syntax | null;
    assignments: (Assignment | null)[];
}

export interface Rule {
    $ast: "Rule";
    attPublic: string;
    attParser: string;
    name: string;
    type: string;
    clauses: (Clause | null)[];
}

export interface SyntaxFile {
    $ast: "SyntaxFile";
    switches: (SwitchItem | null)[];
    rules: (Rule | null)[];
}

