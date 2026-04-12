export type PropType = "Token" | "Type" | "Array";

export type Type = Enum | Class;

export interface Type_Common {
    attPublic: string;
    name: string;
}

export interface EnumItem {
    $ast: "EnumItem";
    name: string;
}

export interface Enum extends Type_Common {
    $ast: "Enum";
    items: (EnumItem | undefined)[];
}

export interface ClassProp {
    $ast: "ClassProp";
    name: string;
    propType: PropType;
    propTypeName: string;
}

export interface Class extends Type_Common {
    $ast: "Class";
    attAmbiguous: string;
    baseClass: string;
    props: (ClassProp | undefined)[];
}

export interface AstFile {
    $ast: "AstFile";
    types: (Type | undefined)[];
}

