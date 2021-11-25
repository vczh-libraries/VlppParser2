#include "../../Source/Calculator/Parser/Calculator_Assembler.h"
#include "../../Source/Calculator/Parser/Calculator_Lexer.h"
#include "../../Source/LogParser.h"

using namespace vl::regex;
using namespace calculator;

extern void GenerateCalculatorSyntax(SyntaxSymbolManager& manager);

namespace TestSyntax_TestObjects
{
	FilePath LogCalculatorSyntax(SyntaxSymbolManager& syntaxManager, const WString& phase)
	{
		return LogSyntax(
			syntaxManager,
			L"Calculator",
			phase,
			[](vint32_t type) { return WString::Unmanaged(CalculatorTypeName((CalculatorClasses)type)); },
			[](vint32_t field) { return WString::Unmanaged(CalculatorFieldName((CalculatorFields)field)); },
			[](vint32_t token)
			{
				auto n = CalculatorTokenId((CalculatorTokens)token);
				auto d = CalculatorTokenDisplayText((CalculatorTokens)token);
				return d ? L"\"" + WString::Unmanaged(d) + L"\"" : WString::Unmanaged(n);
			});
	}

	FilePath LogCalculatorAutomaton(Executable& executable, Metadata& metadata)
	{
		return LogAutomaton(
			L"Calculator",
			executable,
			metadata,
			[](vint32_t type) { return WString::Unmanaged(CalculatorTypeName((CalculatorClasses)type)); },
			[](vint32_t field) { return WString::Unmanaged(CalculatorFieldName((CalculatorFields)field)); },
			[](vint32_t token)
			{
				auto n = CalculatorTokenId((CalculatorTokens)token);
				auto d = CalculatorTokenDisplayText((CalculatorTokens)token);
				return d ? L"\"" + WString::Unmanaged(d) + L"\"" : WString::Unmanaged(n);
			});
	}
}
using namespace TestSyntax_TestObjects;

TEST_FILE
{
	ParserSymbolManager global;
	SyntaxSymbolManager syntaxManager(global);
	GenerateCalculatorSyntax(syntaxManager);
	TEST_CASE_ASSERT(global.Errors().Count() == 0);
	LogCalculatorSyntax(syntaxManager, L"NFA[1]");

	syntaxManager.BuildCompactNFA();
	TEST_CASE_ASSERT(global.Errors().Count() == 0);
	LogCalculatorSyntax(syntaxManager, L"NFA[2]");

	syntaxManager.BuildCrossReferencedNFA();
	TEST_CASE_ASSERT(global.Errors().Count() == 0);
	auto fileCompact = LogCalculatorSyntax(syntaxManager, L"NFA[3]");

	Executable executable;
	Metadata metadata;
	syntaxManager.BuildAutomaton(CalculatorTokenCount, executable, metadata);
	auto fileAutomaton = LogCalculatorAutomaton(executable, metadata);

	auto contentCompact = File(fileCompact).ReadAllTextByBom();
	auto contectAutomaton = File(fileAutomaton).ReadAllTextByBom();
	TEST_CASE_ASSERT(contentCompact == contectAutomaton);

	MemoryStream lexerData;
	CalculatorLexerData(lexerData);
	lexerData.SeekFromBegin(0);
	RegexLexer lexer(lexerData);

	TEST_CASE(L"Launch Calculator Syntax")
	{
		WString input = LR"(
export 1
)";
		List<RegexToken> tokens;
		lexer.Parse(input).ReadToEnd(tokens, CalculatorTokenDeleter);

		TraceManager tm(executable);
		tm.Initialize(executable.ruleStartStates[metadata.ruleNames.IndexOf(L"Module")]);
		for (vint i = 0; i < tokens.Count(); i++)
		{
			auto&& token = tokens[i];
			tm.Input(i, token.token);
			TEST_ASSERT(tm.concurrentCount > 0);
		}
		tm.EndOfInput();
		TEST_ASSERT(tm.concurrentCount == 1);
		TEST_ASSERT(executable.states[tm.concurrentTraces->Get(0)->state].endingState);
	});

#undef LEXER
}