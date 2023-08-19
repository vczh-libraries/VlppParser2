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
Exp0 ::= !(!s; ?(s?: "a":id | !s?: "b":id)) as IdNode;
)SYNTAX";

		const wchar_t* rewrittenCode =
LR"SYNTAX(
Exp0 ::= "b":id as IdNode;
)SYNTAX";

		TestRewrite(typeParser, ruleParser, astCode, lexerCode, syntaxCode, rewrittenCode);
	});

	TEST_CASE(L"Test in Push (single switch) true")
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

	TEST_CASE(L"Test in Push (and)")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch s,t;
Exp0 ::= !( s, t; ?(s&&t?: "a":id | s&&!t?: "b":id | !s&&t?: "c":id | !s&&!t?: "d":id)) as IdNode;
Exp1 ::= !( s,!t; ?(s&&t?: "a":id | s&&!t?: "b":id | !s&&t?: "c":id | !s&&!t?: "d":id)) as IdNode;
Exp2 ::= !(!s, t; ?(s&&t?: "a":id | s&&!t?: "b":id | !s&&t?: "c":id | !s&&!t?: "d":id)) as IdNode;
Exp3 ::= !(!s,!t; ?(s&&t?: "a":id | s&&!t?: "b":id | !s&&t?: "c":id | !s&&!t?: "d":id)) as IdNode;
)SYNTAX";
	
		const wchar_t* rewrittenCode =
LR"SYNTAX(
Exp0 ::= "a":id as IdNode;
Exp1 ::= "b":id as IdNode;
Exp2 ::= "c":id as IdNode;
Exp3 ::= "d":id as IdNode;
)SYNTAX";
	
		TestRewrite(typeParser, ruleParser, astCode, lexerCode, syntaxCode, rewrittenCode);
	});

	TEST_CASE(L"Test in Push (or)")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch s,t;
Exp0 ::= !( s, t; ?(s||t?: "a":id | s||!t?: "b":id | !s||t?: "c":id | !s||!t?: "d":id)) as IdNode;
Exp1 ::= !( s,!t; ?(s||t?: "a":id | s||!t?: "b":id | !s||t?: "c":id | !s||!t?: "d":id)) as IdNode;
Exp2 ::= !(!s, t; ?(s||t?: "a":id | s||!t?: "b":id | !s||t?: "c":id | !s||!t?: "d":id)) as IdNode;
Exp3 ::= !(!s,!t; ?(s||t?: "a":id | s||!t?: "b":id | !s||t?: "c":id | !s||!t?: "d":id)) as IdNode;
)SYNTAX";
	
		const wchar_t* rewrittenCode =
LR"SYNTAX(
Exp0 ::= "a":id | "b":id | "c":id as IdNode;
Exp1 ::= "a":id | "b":id | "d":id as IdNode;
Exp2 ::= "a":id | "c":id | "d":id as IdNode;
Exp3 ::= "b":id | "c":id | "d":id as IdNode;
)SYNTAX";
	
		TestRewrite(typeParser, ruleParser, astCode, lexerCode, syntaxCode, rewrittenCode);
	});
}