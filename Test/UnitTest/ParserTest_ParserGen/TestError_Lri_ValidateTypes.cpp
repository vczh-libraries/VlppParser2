#include "TestError.h"

namespace TestError_CalculatorAstAndLexer
{
	extern const wchar_t* astCode;
	extern const wchar_t* lexerCode;
}
using namespace TestError_CalculatorAstAndLexer;

TEST_FILE
{
	TypeParser typeParser;
	RuleParser ruleParser;

	//////////////////////////////////////////////////////
	// LeftRecursionPlaceholderNotFoundInRule
	//////////////////////////////////////////////////////

	TEST_CASE(L"LeftRecursionPlaceholderNotFoundInRule 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= !Exp0
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
Exp2
  ::= !Exp0 left_recursion_inject(Expression) Exp1
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderNotFoundInRule,L"Exp2",L"Expression",L"Exp1" }
			);
	});

	TEST_CASE(L"LeftRecursionPlaceholderNotFoundInRule 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= !Exp0
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
Exp2
  ::= !Exp0 [left_recursion_inject(Expression) Exp1]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderNotFoundInRule,L"Exp2",L"Expression",L"Exp1" }
			);
	});

	TEST_CASE(L"LeftRecursionPlaceholderNotFoundInRule 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
Exp2
  ::= !Exp0
  ::= Exp2:left "+" Exp0:right as BinaryExpr
  ;
Exp3
  ::= !Exp0 [left_recursion_inject(Expression) (Exp1 left_recursion_inject(Expression) Exp2)]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderNotFoundInRule,L"Exp3",L"Expression",L"Exp2" }
			);
	});

	TEST_CASE(L"LeftRecursionPlaceholderNotFoundInRule 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
Exp2
  ::= !Exp0
  ::= Exp2:left "+" Exp0:right as BinaryExpr
  ;
Exp3
  ::= !Exp0 [left_recursion_inject(Expression)
        (Exp1 left_recursion_inject(Expression) Exp1)
      | (Exp1 left_recursion_inject(Expression) Exp2)
	  ]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderNotFoundInRule,L"Exp3",L"Expression",L"Exp2" }
			);
	});

	//////////////////////////////////////////////////////
	// LeftRecursionPlaceholderNotUnique
	//////////////////////////////////////////////////////

	TEST_CASE(L"LeftRecursionPlaceholderNotUnique 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "*" Exp0:right as BinaryExpr
  ;
Exp2
  ::= left_recursion_placeholder(Expression)
  ::= !Exp1
  ::= Exp2:left "+" Exp1:right as BinaryExpr
  ;
Exp3
  ::= !Exp0 left_recursion_inject(Expression) Exp2
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderNotUnique,L"Exp3",L"Expression",L"Exp2" }
			);
	});

	TEST_CASE(L"LeftRecursionPlaceholderNotUnique 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "*" Exp0:right as BinaryExpr
  ;
Exp2
  ::= left_recursion_placeholder(Expression)
  ::= !Exp1
  ::= Exp2:left "+" Exp1:right as BinaryExpr
  ;
Exp3
  ::= !Exp0 [left_recursion_inject(Expression) Exp2]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderNotUnique,L"Exp3",L"Expression",L"Exp2" }
			);
	});

	TEST_CASE(L"LeftRecursionPlaceholderNotUnique 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "*" Exp0:right as BinaryExpr
  ;
Exp2
  ::= left_recursion_placeholder(Expression)
  ::= !Exp1
  ::= Exp2:left "+" Exp1:right as BinaryExpr
  ;
Exp3
  ::= !Exp0 [left_recursion_inject(Expression) (Exp1 left_recursion_inject(Expression) Exp2)]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderNotUnique,L"Exp3",L"Expression",L"Exp2" }
			);
	});

	TEST_CASE(L"LeftRecursionPlaceholderNotUnique 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "*" Exp0:right as BinaryExpr
  ;
Exp2
  ::= left_recursion_placeholder(Expression)
  ::= !Exp1
  ::= Exp2:left "+" Exp1:right as BinaryExpr
  ;
Exp3
  ::= !Exp0 [left_recursion_inject(Expression)
        (Exp1 left_recursion_inject(Expression) Exp1)
      | (Exp1 left_recursion_inject(Expression) Exp2)
      ]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderNotUnique,L"Exp3",L"Expression",L"Exp2" }
			);
	});

	//////////////////////////////////////////////////////
	// LeftRecursionPlaceholderTypeMismatched
	//////////////////////////////////////////////////////

	TEST_CASE(L"LeftRecursionPlaceholderTypeMismatched 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "*" Exp0:right as BinaryExpr
  ;
Exp2
  ::= Exp1 "+" as Module
  ;
Module
  ::= "+" as Module
  ;
Exp3
  ::= !Module left_recursion_inject(Expression) Exp2
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderTypeMismatched,L"Exp3",L"Expression",L"Exp2",L"Exp1"}
			);
	});

	TEST_CASE(L"LeftRecursionPlaceholderTypeMismatched 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "*" Exp0:right as BinaryExpr
  ;
Exp2
  ::= Exp1 "+" as Module
  ;
Module
  ::= "+" as Module
  ;
Exp3
  ::= !Module [left_recursion_inject(Expression) Exp2]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderTypeMismatched,L"Exp3",L"Expression",L"Exp2",L"Exp1"}
			);
	});

	TEST_CASE(L"LeftRecursionPlaceholderTypeMismatched 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "*" Exp0:right as BinaryExpr
  ;
Exp2
  ::= Exp1 "+" as Module
  ;
Module : Module
  ::= left_recursion_placeholder(Prefix)
  ::= !Module "+"
  ;
Exp3
  ::= !Exp0 [left_recursion_inject(Prefix) (Module left_recursion_inject(Expression) Exp2)]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderTypeMismatched,L"Exp3",L"Prefix",L"Module",L"Module"}
			);
	});

	//////////////////////////////////////////////////////
	// PartialRuleInLeftRecursionInject
	//////////////////////////////////////////////////////

	TEST_CASE(L"PartialRuleInLeftRecursionInject 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0Partial
  ::= NUM:value as partial NumExpr
  ;
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
Exp2
  ::= !Exp0Partial left_recursion_inject(Expression) Exp1
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PartialRuleInLeftRecursionInject,L"Exp2",L"Exp0Partial"}
			);
	});

	TEST_CASE(L"PartialRuleInLeftRecursionInject 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0Partial
  ::= NUM:value as partial NumExpr
  ;
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
Exp2
  ::= !Exp0Partial [left_recursion_inject(Expression) Exp1]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PartialRuleInLeftRecursionInject,L"Exp2",L"Exp0Partial"}
			);
	});

	//////////////////////////////////////////////////////
	// LeftRecursionInjectTargetIsPrefixOfAnotherSameEnding
	//////////////////////////////////////////////////////

	TEST_CASE(L"LeftRecursionInjectTargetIsPrefixOfAnotherSameEnding 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp0Something
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= !Exp0Something 'something'
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= !Exp0Something 'anything'
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
Exp2
  ::= !Exp0 [left_recursion_inject_multiple(Expression) Exp1 | (Exp0Something left_recursion_inject_multiple(Expression) Exp1)]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionInjectTargetIsPrefixOfAnotherSameEnding,L"Exp2",L"Expression",L"Exp0Something",L"Exp1"}
			);
	});

	TEST_CASE(L"LeftRecursionInjectTargetIsPrefixOfAnotherSameEnding 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp0Something
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= !Exp0Something 'something'
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= !Exp0Something 'anything'
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
Exp2
  ::= !Exp0 [left_recursion_inject_multiple(Expression) (Exp0Something left_recursion_inject_multiple(Expression) Exp1) | Exp1]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionInjectTargetIsPrefixOfAnotherSameEnding,L"Exp2",L"Expression",L"Exp0Something",L"Exp1"}
			);
	});

	TEST_CASE(L"LeftRecursionInjectTargetIsPrefixOfAnotherSameEnding 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp0Something
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= !Exp0Something 'something'
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= !Exp0Something 'anything'
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
RandomThing
  ::= !Exp1
  ::= left_recursion_placeholder(Random)
  ;
Exp2
  ::= !Exp0 [left_recursion_inject_multiple(Expression)
              (Exp0Something left_recursion_inject_multiple(Expression) Exp1) |
              (Exp1 [left_recursion_inject_multiple(Random) RandomThing])     ]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionInjectTargetIsPrefixOfAnotherSameEnding,L"Exp2",L"Expression",L"Exp0Something",L"Exp1"}
			);
	});

	TEST_CASE(L"LeftRecursionInjectTargetIsPrefixOfAnotherSameEnding 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp0Something
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= !Exp0Something 'something'
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= !Exp0Something 'anything'
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
RandomThing
  ::= !Exp1
  ::= left_recursion_placeholder(Random)
  ;
Exp2
  ::= !Exp0 [left_recursion_inject_multiple(Expression)
              (Exp0Something left_recursion_inject_multiple(Expression) (Exp1 left_recursion_inject_multiple(Random) RandomThing)) |
              (Exp1 left_recursion_inject_multiple(Random) RandomThing)                                                            ]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionInjectTargetIsPrefixOfAnotherSameEnding,L"Exp2",L"Expression",L"Exp0Something",L"Exp1"}
			);
	});
}