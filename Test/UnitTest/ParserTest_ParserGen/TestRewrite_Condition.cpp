#include "TestError.h"

namespace TestRewrite_Condition
{
	const wchar_t* astCode =
LR"AST(
class Node
{
}

class IdNode : Node
{
	var id : token;
}

class SeqNode : Node
{
	var nodes : Node[];
}
)AST";

	const wchar_t* lexerCode =
LR"LEXER(
A:a
B:b
C:c
D:d
OPEN:/(
CLOSE:/)
discard SPACE:/s+
)LEXER";
}
using namespace TestRewrite_Condition;

TEST_FILE
{
	TypeParser typeParser;
	RuleParser ruleParser;

	TEST_CASE(L"Test in Push (single switch) false")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch s;
Exp0 ::= !(s; ?(s?: "a":id | !s?: "b":id)) as IdNode;
)SYNTAX";

		const wchar_t* rewrittenCode =
LR"SYNTAX(
Exp0 ::= "a":id as IdNode;
)SYNTAX";

		TestRewrite(typeParser, ruleParser, astCode, lexerCode, syntaxCode, rewrittenCode);
	});
}