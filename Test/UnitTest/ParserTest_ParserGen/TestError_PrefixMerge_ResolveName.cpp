#include "TestError.h"

TEST_FILE
{
	TypeParser typeParser;
	RuleParser ruleParser;

	const wchar_t* astCode =
LR"AST(
class Expr
{
}
)AST";

	const wchar_t* lexerCode =
LR"LEXER(
ID:[a-zA-Z_]/w*
NUM:/d+(./d+)?
ADD:/+
MUL:/*
OPEN:/(
CLOSE:/)
discard SPACE:/s+
)LEXER";

	//////////////////////////////////////////////////////
	// SyntaxInvolvesPrefixMergeWithIllegalRuleName
	//////////////////////////////////////////////////////

	TEST_CASE(L"SyntaxInvolvesPrefixMergeWithIllegalRuleName")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM as Expr
  ;
Exp1
  ::= !prefix_merge(Exp0)
  ::= !Exp1 ADD
  ;
LRI_Something
  ::= !Exp1
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::SyntaxInvolvesPrefixMergeWithIllegalRuleName,L"LRI_Something" }
			);
	});

	//////////////////////////////////////////////////////
	// SyntaxInvolvesPrefixMergeWithIllegalPlaceholderName
	//////////////////////////////////////////////////////

	TEST_CASE(L"SyntaxInvolvesPrefixMergeWithIllegalPlaceholderName")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM as Expr
  ;
Exp1
  ::= !prefix_merge(Exp0)
  ::= !Exp1 ADD
  ;
Exp2
  ::= left_recursion_placeholder(LRIP_Something)
  ::= !Exp1
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::SyntaxInvolvesPrefixMergeWithIllegalPlaceholderName,L"Exp2",L"LRIP_Something"}
			);
	});
}