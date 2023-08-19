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

	TEST_CASE(L"Test called by Push (and)")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch s,t;
Switches
  ::=  ?( s&& t?: "a":id) as IdNode
  ::=  ?( s&&!t?: "b":id) as IdNode
  ::=  ?(!s&& t?: "c":id) as IdNode
  ::=  ?(!s&&!t?: "d":id) as IdNode
  ;
Exp0 ::= !( s, t; !Switches);
Exp1 ::= !( s,!t; !Switches);
Exp2 ::= !(!s, t; !Switches);
Exp3 ::= !(!s,!t; !Switches);
)SYNTAX";
	
		const wchar_t* rewrittenCode =
LR"SYNTAX(
Switches_SWITCH_0s_0t:IdNode ::= "d":id as IdNode;
Switches_SWITCH_0s_1t:IdNode ::= "c":id as IdNode;
Switches_SWITCH_1s_0t:IdNode ::= "b":id as IdNode;
Switches_SWITCH_1s_1t:IdNode ::= "a":id as IdNode;
Exp0 ::= !Switches_SWITCH_1s_1t;
Exp1 ::= !Switches_SWITCH_1s_0t;
Exp2 ::= !Switches_SWITCH_0s_1t;
Exp3 ::= !Switches_SWITCH_0s_0t;
)SYNTAX";
	
		TestRewrite(typeParser, ruleParser, astCode, lexerCode, syntaxCode, rewrittenCode);
	});

	TEST_CASE(L"Test called by Push (or)")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch s,t;
Switches
  ::=  ?( s|| t?: "a":id) as IdNode
  ::=  ?( s||!t?: "b":id) as IdNode
  ::=  ?(!s|| t?: "c":id) as IdNode
  ::=  ?(!s||!t?: "d":id) as IdNode
  ;
Exp0 ::= !( s, t; !Switches);
Exp1 ::= !( s,!t; !Switches);
Exp2 ::= !(!s, t; !Switches);
Exp3 ::= !(!s,!t; !Switches);
)SYNTAX";
	
		const wchar_t* rewrittenCode =
LR"SYNTAX(
Switches_SWITCH_0s_0t:IdNode ::= "b":id as IdNode ::= "c":id as IdNode ::= "d":id as IdNode;
Switches_SWITCH_0s_1t:IdNode ::= "a":id as IdNode ::= "c":id as IdNode ::= "d":id as IdNode;
Switches_SWITCH_1s_0t:IdNode ::= "a":id as IdNode ::= "b":id as IdNode ::= "d":id as IdNode;
Switches_SWITCH_1s_1t:IdNode ::= "a":id as IdNode ::= "b":id as IdNode ::= "c":id as IdNode;
Exp0 ::= !Switches_SWITCH_1s_1t;
Exp1 ::= !Switches_SWITCH_1s_0t;
Exp2 ::= !Switches_SWITCH_0s_1t;
Exp3 ::= !Switches_SWITCH_0s_0t;
)SYNTAX";
	
		TestRewrite(typeParser, ruleParser, astCode, lexerCode, syntaxCode, rewrittenCode);
	});

	TEST_CASE(L"Test called by Push (partial)")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch s,t;
SwitchesAnd
  ::=  ?( s&& t?: "a":id) as IdNode
  ::=  ?( s&&!t?: "b":id) as IdNode
  ::=  ?(!s&& t?: "c":id) as IdNode
  ::=  ?(!s&&!t?: "d":id) as IdNode
  ;
SwitchesOr
  ::=  ?( s|| t?: "a":id) as IdNode
  ::=  ?( s||!t?: "b":id) as IdNode
  ::=  ?(!s|| t?: "c":id) as IdNode
  ::=  ?(!s||!t?: "d":id) as IdNode
  ;
Exp0 ::= !( s, t; !SwitchesAnd);
Exp1 ::= !( s,!t; !SwitchesAnd);
Exp2 ::= !(!s, t; !SwitchesOr);
Exp3 ::= !(!s,!t; !SwitchesOr);
)SYNTAX";
	
		const wchar_t* rewrittenCode =
LR"SYNTAX(
SwitchesAnd_SWITCH_1s_0t:IdNode ::= "b":id as IdNode;
SwitchesAnd_SWITCH_1s_1t:IdNode ::= "a":id as IdNode;
SwitchesOr_SWITCH_0s_0t:IdNode ::= "b":id as IdNode ::= "c":id as IdNode ::= "d":id as IdNode;
SwitchesOr_SWITCH_0s_1t:IdNode ::= "a":id as IdNode ::= "c":id as IdNode ::= "d":id as IdNode;
Exp0 ::= !SwitchesAnd_SWITCH_1s_1t;
Exp1 ::= !SwitchesAnd_SWITCH_1s_0t;
Exp2 ::= !SwitchesOr_SWITCH_0s_1t;
Exp3 ::= !SwitchesOr_SWITCH_0s_0t;
)SYNTAX";
	
		TestRewrite(typeParser, ruleParser, astCode, lexerCode, syntaxCode, rewrittenCode);
	});
}