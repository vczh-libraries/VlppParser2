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
	// PrefixMergeAffectedBySwitches
	//////////////////////////////////////////////////////

	TEST_CASE(L"PrefixMergeAffectedBySwitches 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0 ::= ?(first: NUM:value) as NumExpr;
PM ::= !prefix_merge(Exp0);
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PrefixMergeAffectedBySwitches,L"PM",L"Exp0",L"first"}
		);
	});

	TEST_CASE(L"PrefixMergeAffectedBySwitches 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first, second;
Exp0 ::= !(first; ?(first && !second: NUM:value)) as NumExpr;
PM ::= !prefix_merge(Exp0);
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PrefixMergeAffectedBySwitches,L"PM",L"Exp0",L"second"}
		);
	});

	TEST_CASE(L"PrefixMergeAffectedBySwitches 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first, second;
Exp0 ::= ?(first && !second: NUM:value) as NumExpr;
Exp1 ::= !(first; !Exp0);
PM ::= !prefix_merge(Exp1);
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PrefixMergeAffectedBySwitches,L"PM",L"Exp1",L"second"}
		);
	});
}