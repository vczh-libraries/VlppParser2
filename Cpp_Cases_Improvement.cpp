/*
This is a BuiltIn-Cpp parser corpus, not a compiler test or a parser-generator
test. Some names are deliberately undeclared and the PRACTICAL cases are
deliberately not strict standard C++.

EXPECT-ONE:
    A syntax-only AST postpass should retain one interpretation. No symbol
    lookup is required.

EXPECT-RECLASSIFY:
    The current grammar accepts the tokens through an older/general node. A
    syntax-only postpass can give the construct its standard identity.

EXPECT-AMBIGUITY:
    Selecting an interpretation requires name, type, template, pack, or
    language-version information. Preserve an ambiguity node.

PRACTICAL:
    The spelling is not strict standard C++, but BuiltIn-Cpp deliberately
    accepts it because the resulting operator interpretation is useful and
    unambiguous.
*/

namespace qname_fixture
{
    namespace A
    {
        int B;

        namespace b
        {
            struct c
            {
                int field;
            };
        }
    }
}

void qualified_name_canonicalization()
{
    // EXPECT-ONE: whitespace and comments do not split a qualified name.
    A ::B;
    qname_fixture::A ::B;
    qname_fixture::A:: B;
    qname_fixture /* comment */ :: A /* comment */ :: B;

    // EXPECT-ONE: the root marker and every component form one chain.
    ::qname_fixture::A::B;
}

// EXPECT-ONE: the member-pointer qualifier is the complete
// ::qname_fixture::A::b::c::. It must not be split across nested declarators.
int ::qname_fixture::A::b::c::* canonical_member_pointer;

// Exact regression shape from TODO.md. It is syntactically valid when the
// names acquire the required namespace/class meanings.
int ::a::b::c::* todo_member_pointer;

// EXPECT-ONE: the elaborated-type-specifier forces a type interpretation.
class qname_fixture::A::b::c* elaborated_type_pointer;

template<typename T>
void explicit_disambiguators(T& object)
{
    // EXPECT-ONE: typename forces a type interpretation.
    typename T::value_type* value;

    // EXPECT-ONE: typename and template force the dependent type/template
    // interpretation. Preserve both tokens or their source ranges in the AST.
    typename T::template rebind<int>* rebound;

    // EXPECT-ONE: template forces call to be a template-id.
    object.template call<int>();
}

void declaration_ambiguity_resolution()
{
    // EXPECT-ONE: function declarations according to [dcl.ambig.res].
    S v(int(a));
    S w(int());

    // EXPECT-ONE: object declarations. Extra parentheses or a different
    // initializer form prevent the function-declaration interpretation.
    S x((int(a)));
    S y((int)a);
    S z = int(a);

    // EXPECT-ONE: the potential parameter declaration has a trailing return
    // type but does not begin with auto, so this is an object declaration.
    S object_case(B()->C);

    // EXPECT-AMBIGUITY: auto activates the trailing-return-type candidate,
    // but choosing it still requires C to resolve as a type.
    S function_case(auto()->C);
}

namespace nested_parameter_fixture
{
    class C
    {
    };

    // EXPECT-AMBIGUITY: after lookup establishes that C is a type,
    // [dcl.ambig.res] selects a parameter of pointer-to-function type rather
    // than a parameter named C with redundant parentheses.
    void nested_type_parameter(int(C));

    // EXPECT-ONE: explicit spelling of the interpretation selected above.
    void nested_type_parameter_control(int(*fp)(C));

    // EXPECT-AMBIGUITY: after C is known to be a type, this is a parameter of
    // pointer-to-function type whose parameter is adjusted from C[10].
    void nested_array_parameter(int *(C[10]));
}

void statement_ambiguity_resolution()
{
    // EXPECT-ONE: declaration statements. The built-in int makes the type
    // classification independent of lookup.
    int(a);
    int(*b)();
    int(c) = 7;
    int(d), e, f = 3;
    int(g)(h, 2);
    int(indexed)[5];

    // EXPECT-ONE: complete expression shapes that cannot be declarations.
    int(expression1)->member = 7;
    int(expression2)++;
    int(expression3, 5) << value;

    // EXPECT-AMBIGUITY: M must resolve as a type before this can be selected
    // as a function declaration with a trailing return type.
    auto(s)()->M;

    // EXPECT-ONE: without leading auto, this is an expression regardless of
    // whether M later resolves as a type.
    S(s)()->M;
}

void condition_ambiguity_resolution()
{
    // EXPECT-ONE: these are condition-declarations. The built-in type makes
    // the decision syntax-only.
    if (int(if_value) = 0)
    {
    }

    while (int(while_value) = 0)
    {
    }
}

X<int()> type_id_template_argument;
X<int(1)> expression_template_argument;

void type_id_ambiguity_resolution()
{
    // EXPECT-ONE (TYPE-ID): [dcl.ambig.res] prefers these type-id shapes.
    // Their later semantic validity is outside this pass.
    sizeof(int());
    sizeof(int(unsigned(a)));

    // EXPECT-ONE (EXPRESSION): a named declarator cannot occur in a type-id.
    sizeof(int(a));

    // EXPECT-ONE (TYPE-ID): the same preference applies in a cast expression.
    (int()) + 1;
    (int(unsigned(a))) + 1;

    // EXPECT-ONE (EXPRESSION): int(a) is the parenthesized expression operand.
    (int(a)) + 1;
}

void lookup_dependent_ambiguities_must_remain()
{
    // EXPECT-AMBIGUITY: declaration statement or relational expression.
    A<B>C;

    // EXPECT-AMBIGUITY: declaration or multiplication expression.
    T * pointer_or_product;

    // EXPECT-AMBIGUITY: declarations or functional conversions/calls.
    T(value);
    T(value) = other;
    T(indexed)[5];

    // EXPECT-AMBIGUITY: C-style cast or call of a parenthesized expression.
    (T)(value);

    // EXPECT-AMBIGUITY: template-id qualified names or relational forms.
    A<B>::C;
    Name<a < b>;

    // EXPECT-AMBIGUITY: existing coverage expects call, binary-expression,
    // and function-type candidates.
    A<B>(C);
}

void structurally_shared_syntax()
{
    // These need later semantic classification, but not duplicate spelling
    // trees: T is the same qualified-name syntax node in either role.
    sizeof(T);
    typeid(T);
    X<T> value;
}

template<typename T>
struct GuideBox
{
    GuideBox(T);
};

// EXPECT-RECLASSIFY: the arrow and matching template-name shape identify a
// deduction guide. The current generic function-declaration AST loses this.
template<typename T>
GuideBox(T) -> GuideBox<T>;

enum class Color
{
    red,
    green,
};

// EXPECT-RECLASSIFY: enum is explicit, so no lookup is needed to replace the
// current UsingValueDeclaration shape with a using-enum declaration.
using enum Color;

void version_sensitive_subscripts()
{
    // EXPECT-AMBIGUITY: in C++23 this can be a two-argument subscript; in an
    // earlier edition it can be one comma-expression index. Preserve the
    // versioned alternatives or a lossless representation of both shapes.
    matrix[row, column];
}

// EXPECT-AMBIGUITY: before lookup, C can be a concept constraining a type
// parameter or a type naming a non-type template parameter.
template<C T>
struct constrained_or_nontype_parameter;

// EXPECT-RECLASSIFY: T is declared as a non-pack, so this comma-less ellipsis
// is the function's deprecated C-style varargs suffix.
template<typename T>
void dependent_pack_or_varargs(T...);

// EXPECT-RECLASSIFY: Types is declared as a type pack, so this is a function
// parameter pack and not C-style varargs.
template<typename... Types>
void known_pack_parameter(Types...);

// EXPECT-ONE: a built-in type cannot be a pack pattern; this is varargs.
void known_varargs_parameter(int...);

// EXPECT-ONE: a placeholder followed by ellipsis declares a parameter pack.
void placeholder_pack_parameter(auto...);

void practical_split_punctuators()
{
    left >> right;
    left > > right;       // PRACTICAL: intentionally accepted as right shift.

    left << right;
    left < < right;       // PRACTICAL: intentionally accepted as left shift.

    left >= right;
    left > = right;       // PRACTICAL: intentionally accepted as >=.

    left <= right;
    left < = right;       // PRACTICAL: intentionally accepted as <=.

    left >>= right;
    left > > = right;     // PRACTICAL: intentionally accepted as >>=.

    left <<= right;
    left < < = right;     // PRACTICAL: intentionally accepted as <<=.

    // PRACTICAL: the standard longest-new-type-id rule consumes * as a
    // pointer declarator and then rejects the stray i. Accepting the useful
    // multiplication-shaped overparse does not misread any valid program.
    new int * i;
}

// C++26 reflect-expression longest-operand cases are kept with the complete
// reflection syntax inventory in Cpp_Cases_26.md.

struct PracticalOperatorNames
{
    // PRACTICAL: the operator name uses the same split-token policy.
    PracticalOperatorNames operator > >(int);
    PracticalOperatorNames operator < <(int);
};
