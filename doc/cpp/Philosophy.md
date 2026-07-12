# C++ Syntax Implementation Philosophy

The C++ syntax will be used as an code indexer, not a part of a compiler, so we don't pursue extreme accuracy.
The C++ syntax generally accepts ambiguity if resolving it requires semantic analyzing, since we don't want to introduce any symbol resolving logic.
Here are a list of principles with examples.
Any principle has a higher priority than former ones, it helps to resolve conflict.

## Accept History of C++ Version but Prefer New Standard

We don't assume the C++ standard incoming code is using.
Some syntax might becomes invalid in later standards, but we choose to accept all if possible.

## Orthogonality

This is an important rule to help the syntax clear and DRY (Don't Repeat Yourself).
For example, if multiple places need a type, they should use the same rule.
But it is very common that exceptions are introduced, just like operator ">" can't be used outside of a pair of parenthesis in template arguments, that's why the switch feature is introduced.
Another example would be reserved keywords. Reserved keywords could be used as a name, so just like how `Identifier` handle operator names, introducing reserved keywords into `Identifier` makes the syntax clean in both way:
- We can easily use `Identifier` everywhere when a reserved keywords is acceptable.
- We can easily use "keyword" in syntax to define the syntax, as "such_syntax" requires a token type that only accept this token to exist.

## Accepting Limited Incorrect Syntax

Accepting limited incorrect syntax could sometimes maintain orthogonality.
For example, the friend keyword could be used half way of a declaration, like `int static * method();`.
To maintain orthogonality design about declarators using the idea of combinators, we can declare `static` a kind of declarators.
It might introduce a problem that makes `using X = int static *;` valid.
But it is fine as such code will be rejected by a real C++ compiler before reaching us.
Another example would be accepting `a > > b`, although this is invalid, but such syntax accepts `a >> b` and also makes `a<b<c>>` becomes so easy to parse, it introduces enough orthogonality, meanwhile it doesn't mess up any parsing of correct code.

## No Introduction of Unnecessary Ambiguity

`auto` used to be a valid name, but `auto` also has its own meaning.
But if we simply list `auto` as an identifier, it makes `auto x;` creates ambiguity, as `auto` might be a identifier interpreted as a type, or might be the "auto" type in the AST.
Since we prefer new standard and we don't want unnecessary ambiguity, we could simple deny `auto` as a type, even it was valid before C++11.
