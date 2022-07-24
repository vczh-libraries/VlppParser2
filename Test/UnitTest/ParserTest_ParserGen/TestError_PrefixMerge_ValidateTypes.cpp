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
	// PartialRuleInLeftRecursionInject
	//////////////////////////////////////////////////////

	TEST_CASE(L"PartialRuleInPrefixMerge")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Partial
  ::= NUM:value as partial NumExpr
  ;
Exp0
  ::= !prefix_merge(Partial)
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PartialRuleInPrefixMerge,L"Exp0",L"Partial"}
			);
	});
}