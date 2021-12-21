#include "../../Source/Calculator/Parser/CalculatorExprAst_Builder.h"
#include "../../Source/Calculator/Parser/CalculatorExprAst_Copy.h"
#include "../../Source/Calculator/Parser/CalculatorExprAst_Json.h"
#include "../../Source/Calculator/Parser/CalculatorExprAst_Traverse.h"
#include "../../Source/Calculator/Parser/Calculator_Assembler.h"
#include "../../Source/Calculator/Parser/Calculator_Lexer.h"
#include "../../Source/LogParser.h"

using namespace calculator;
using namespace calculator::builder;

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
		for (vint32_t i = 0; i < tokens.Count(); i++)
		{
			auto&& token = tokens[i];
			auto lookAhead = i == tokens.Count() - 1 ? -1 : tokens[i + 1].token;
			tm.Input(i, (vint32_t)token.token, (vint32_t)lookAhead);
			TEST_ASSERT(tm.concurrentCount > 0);
		}

		tm.EndOfInput();
		auto rootTrace = tm.PrepareTraceRoute();

		LogTraceManager(
			L"Calculator",
			caseName,
			executable,
			tm,
			rootTrace,
			tokens,
			[](vint32_t type) { return WString::Unmanaged(CalculatorTypeName((CalculatorClasses)type)); },
			[](vint32_t field) { return WString::Unmanaged(CalculatorFieldName((CalculatorFields)field)); },
			[](vint32_t token) { return WString::Unmanaged(CalculatorTokenId((CalculatorTokens)token)); },
			[&](vint32_t rule) { return metadata.ruleNames[rule]; },
			[&](vint32_t state) { return metadata.stateLabels[state]; }
			);

		TEST_ASSERT(tm.concurrentCount == 1);
		TEST_ASSERT(executable.states[tm.concurrentTraces->Get(0)->state].endingState);

		LogTraceExecution(
			L"Calculator",
			caseName,
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

	class CalculatorInspectTokenVisitor : public traverse_visitor::ExprAstVisitor
	{
	public:
		WString			visitedTokens;

	protected:
		void Traverse(vl::glr::ParsingToken& token) override
		{
			visitedTokens += L"[" + token.value + L"]";
		}
	};

	class CalculatorInspectExprVisitor : public traverse_visitor::ExprAstVisitor
	{
	public:
		WString			input;
		List<WString>	visitorExprs;

		CalculatorInspectExprVisitor(const WString& _input)
			: input(_input)
		{
		}

	protected:
		void Traverse(Expr* node) override
		{
			visitorExprs.Add(input.Sub(
				node->codeRange.start.index,
				node->codeRange.end.index - node->codeRange.start.index + 1
				));
		}
	};
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
		AssertAst<json_visitor::ExprAstVisitor>(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "NumExpr",
        "value": "1"
    },
    "imports": []
})");
	});

	TEST_CATEGORY(L"Test Generated Utilities")
	{
		WString input = LR"(
import sin
import cos
import abs
export abs((sin(x) + cos(y)))
)";
		const wchar_t* output = LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "Call",
        "args": [{
            "$ast": "Binary",
            "expanded": null,
            "left": {
                "$ast": "Call",
                "args": [{
                    "$ast": "Ref",
                    "name": "x"
                }],
                "func": {
                    "$ast": "Ref",
                    "name": "sin"
                }
            },
            "op": "Add",
            "right": {
                "$ast": "Call",
                "args": [{
                    "$ast": "Ref",
                    "name": "y"
                }],
                "func": {
                    "$ast": "Ref",
                    "name": "cos"
                }
            }
        }],
        "func": {
            "$ast": "Ref",
            "name": "abs"
        }
    },
    "imports": [{
        "$ast": "Import",
        "name": "sin"
    }, {
        "$ast": "Import",
        "name": "cos"
    }, {
        "$ast": "Import",
        "name": "abs"
    }]
})";
		Ptr<Module> ast;

		TEST_CASE(L"Test json_visitor::ExprAstVisitor")
		{
			ast = ParseCalculator(input, lexer, executable, metadata, L"Calculator");
			AssertAst<json_visitor::ExprAstVisitor>(ast, output);
		});

		TEST_CASE(L"Test copy_visitor::ExprAstVisitor")
		{
			auto copiedAst = copy_visitor::ExprAstVisitor().CopyNode(ast.Obj());
			AssertAst<json_visitor::ExprAstVisitor>(copiedAst, output);
		});

		TEST_CASE(L"Test traverse_visitor::ExprAstVisitor")
		{
			CalculatorInspectTokenVisitor visitor;
			visitor.InspectInto(ast.Obj());
			TEST_ASSERT(visitor.visitedTokens == L"[x][sin][y][cos][abs][sin][cos][abs]");
		});

		TEST_CASE(L"Test Builder")
		{
			Ptr<Module> makedAst = MakeModule()
				.imports(MakeImport().name(L"sin"))
				.imports(MakeImport().name(L"cos"))
				.imports(MakeImport().name(L"abs"))
				.exported(MakeCall()
					.func(MakeRef().name(L"abs"))
					.args(MakeBinary()
						.op(BinaryOp::Add)
						.left(MakeCall()
							.func(MakeRef().name(L"sin"))
							.args(MakeRef().name(L"x"))
							)
						.right(MakeCall()
							.func(MakeRef().name(L"cos"))
							.args(MakeRef().name(L"y"))
							)
						)
					);
			AssertAst<json_visitor::ExprAstVisitor>(makedAst, output);
		});

		TEST_CASE(L"Test CodeRange")
		{
			CalculatorInspectExprVisitor visitor(input);
			visitor.InspectInto(ast.Obj());

			List<WString> expected;
			expected.Add(L"x");
			expected.Add(L"sin(x)");
			expected.Add(L"y");
			expected.Add(L"cos(y)");
			expected.Add(L"sin(x) + cos(y)");
			expected.Add(L"abs((sin(x) + cos(y)))");

			TEST_ASSERT(CompareEnumerable(expected, visitor.visitorExprs) == 0);
		});
	});

	MemoryStream executableStream;
	executable.Serialize(executableStream);
	executableStream.SeekFromBegin(0);
	Executable executable2(executableStream);

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
				auto ast = ParseCalculator(input, lexer, executable2, metadata, caseName);
				auto actualJson = PrintAstJson<json_visitor::ExprAstVisitor>(ast);
				File(dirOutput / (L"Output[" + caseName + L"].json")).WriteAllText(actualJson, true, BomEncoder::Utf8);
				auto expectedJson = File(dirBaseline / (caseName + L".json")).ReadAllTextByBom();
				AssertLines(expectedJson, actualJson);
			});
		}
	});
}