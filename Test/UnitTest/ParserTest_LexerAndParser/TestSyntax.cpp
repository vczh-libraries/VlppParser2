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

		auto currentTrace = tm.PrepareTraceRoute();
		CalculatorAstInsReceiver receiver;
		while (currentTrace)
		{
			if (currentTrace->byEdge != -1)
			{
				auto& edgeDesc = executable.edges[currentTrace->byEdge];
				for (vint insRef = 0; insRef < edgeDesc.insBeforeInput.count; insRef++)
				{
					vint insIndex = edgeDesc.insBeforeInput.start + insRef;
					auto& ins = executable.instructions[insIndex];
					auto& token = tokens[currentTrace->previousTokenIndex == -1 ? 0 : currentTrace->previousTokenIndex];
					receiver.Execute(ins, token);
				}
				for (vint insRef = 0; insRef < edgeDesc.insAfterInput.count; insRef++)
				{
					vint insIndex = edgeDesc.insAfterInput.start + insRef;
					auto& ins = executable.instructions[insIndex];
					auto& token = tokens[currentTrace->currentTokenIndex];
					receiver.Execute(ins, token);
				}
			}

			if (currentTrace->executedReturn != -1)
			{
				auto& returnDesc = executable.returns[currentTrace->executedReturn];
				for (vint insRef = 0; insRef < returnDesc.insAfterInput.count; insRef++)
				{
					vint insIndex = returnDesc.insAfterInput.start + insRef;
					auto& ins = executable.instructions[insIndex];
					auto& token = tokens[currentTrace->currentTokenIndex];
					receiver.Execute(ins, token);
				}
			}

			if (currentTrace->selectedNext == -1)
			{
				currentTrace = nullptr;
			}
			else
			{
				currentTrace = tm.GetTrace(currentTrace->selectedNext);
			}
		}
		auto ast = receiver.Finished();
		auto astModule = ast.Cast<Module>();
		TEST_ASSERT(astModule);
	});

#undef LEXER
}