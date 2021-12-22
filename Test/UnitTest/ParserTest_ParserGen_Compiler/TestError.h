#ifndef VCZH_VLPPPARSER2_UNITTEST_TESTERROR
#define VCZH_VLPPPARSER2_UNITTEST_TESTERROR

#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleParser.h"
#include "../../../Source/ParserGen/Compiler.h"
#include "../../../Source/Ast/AstCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::glr;
using namespace vl::glr::automaton;
using namespace vl::glr::parsergen;

namespace TestError_Syntax_TestObjects
{
	struct ParserErrorWithoutLocation
	{
		ParserErrorType				type;
		const wchar_t* arg1 = nullptr;
		const wchar_t* arg2 = nullptr;
		const wchar_t* arg3 = nullptr;
		const wchar_t* arg4 = nullptr;
	};

	extern void AssertError(ParserSymbolManager& global, ParserErrorWithoutLocation expectedError);
	extern void ExpectError(TypeParser& typeParser, RuleParser& ruleParser, const WString& astCode, const WString& lexerCode, const WString& syntaxCode, ParserErrorWithoutLocation expectedError);
}
using namespace TestError_Syntax_TestObjects;

#endif