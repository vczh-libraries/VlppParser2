#include "../../Source/Calculator/Parser/CalculatorAst_Json.h"
#include "../../Source/Calculator/Parser/Calculator_Assembler.h"
#include "../../Source/Calculator/Parser/Calculator_Lexer.h"
#include "../../Source/LogParser.h"

using namespace vl::regex;
using namespace calculator;

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);
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

	Ptr<Module> ParseCalculator(const WString& input, RegexLexer& lexer, Executable& executable, Metadata& metadata, const WString& caseName)
	{
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

		auto rootTrace = tm.PrepareTraceRoute();
		LogTrace(
			L"Calculator",
			caseName,
			executable,
			metadata,
			tm,
			rootTrace,
			tokens,
			[](vint32_t type) { return WString::Unmanaged(CalculatorTypeName((CalculatorClasses)type)); },
			[](vint32_t field) { return WString::Unmanaged(CalculatorFieldName((CalculatorFields)field)); },
			[](vint32_t token) { return WString::Unmanaged(CalculatorTokenId((CalculatorTokens)token)); }
			);
		CalculatorAstInsReceiver receiver;
		auto ast = tm.ExecuteTrace(rootTrace, receiver, tokens);
		auto astModule = ast.Cast<Module>();
		TEST_ASSERT(astModule);

		return astModule;
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
		auto ast = ParseCalculator(input, lexer, executable, metadata, L"Calculator");
		AssertAst<json_visitor::AstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "NumExpr",
        "value": "1"
    },
    "imports": []
})");
	});

	TEST_CATEGORY(L"Test Calculator Syntax")
	{
		Folder dirInput = FilePath(GetTestParserInputPath(L"Calculator")) / L"Input";
		FilePath dirBaseline = FilePath(GetTestParserInputPath(L"Calculator")) / L"Output";
		FilePath dirOutput = GetOutputDir(L"Calculator");

		List<File> inputFiles;
		dirInput.GetFiles(inputFiles);
		for (auto&& inputFile : inputFiles)
		{
			auto caseName = inputFile.GetFilePath().GetName();
			caseName = caseName.Left(caseName.Length() - 4);

			TEST_CASE(caseName)
			{
				auto input = inputFile.ReadAllTextByBom();
				auto ast = ParseCalculator(input, lexer, executable, metadata, caseName);
				auto actualJson = PrintAstJson<json_visitor::AstVisitor>(ast);
				File(dirOutput / (L"Output[" + caseName + L"].json")).WriteAllText(actualJson, true, BomEncoder::Utf8);
			});
		}
	});

#undef LEXER
}