export type Item = ItemToResolve | IntItem | IntCommaItem | IntDotItem | IntQuestionItem | ClassItem | ClassQuestionItem | QuestionItem;

export interface ItemToResolve {
    $ast: "ItemToResolve";
    candidates: (Item | null)[];
}

export interface IntItem {
    $ast: "IntItem";
}

export interface IntCommaItem {
    $ast: "IntCommaItem";
}

export interface IntDotItem {
    $ast: "IntDotItem";
}

export interface IntQuestionItem {
    $ast: "IntQuestionItem";
}

export interface ClassItem {
    $ast: "ClassItem";
}

export interface ClassQuestionItem {
    $ast: "ClassQuestionItem";
}

export interface QuestionItem {
    $ast: "QuestionItem";
    item: Item | null;
}

export interface File {
    $ast: "File";
    items: (Item | null)[];
}

