#include "../../Source/Calculator/Generated/CalculatorExprAst_Json.h"
#include "../../Source/Calculator/Generated/CalculatorModuleParser.h"
#include "../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Json.h"
#include "../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityModuleParser.h"
#include "../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Json.h"
#include "../../Source/IfElsePriority/Generated/IfElsePriorityModuleParser.h"
#include "../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Json.h"
#include "../../Source/GenericAmbiguity/Generated/GenericAmbiguityModuleParser.h"
#include "../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Json.h"
#include "../../Source/FeatureTest/Generated/FeatureTestModuleParser.h"
#include "../../Source/BinaryOp/Generated/BinaryOpExprAst_Json.h"
#include "../../Source/BinaryOp/Generated/BinaryOpModuleParser.h"
#include "../../Source/LogTrace.h"

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

namespace TestParser_Generated_TestObjects
{
	template<
		typename TParser,
		typename TJsonVisitor,
		typename TStates,
		typename TClasses,
		typename TFields,
		typename TTokens
		>
	void TestParser(
		const WString& parserName,
		TStates startState,
		const wchar_t* (*typeName)(TClasses),
		const wchar_t* (*fieldName)(TFields),
		const wchar_t* (*tokenId)(TTokens),
		const wchar_t* (*ruleName)(vint),
		const wchar_t* (*stateLabel)(vint),
		const wchar_t* (*switchName)(vint)
		)
	{
		TParser parser;
		WString caseName;

		parser.OnEndOfInput.Add(
			[&](EndOfInputArgs& args)
			{
				LogTraceManager(
					L"Generated-" + parserName,
					caseName,
					args.executable,
					args.traceManager,
					args.rootTrace,
					args.tokens,
					[=](vint32_t type) { return WString::Unmanaged(typeName((TClasses)type)); },
					[=](vint32_t field) { return WString::Unmanaged(fieldName((TFields)field)); },
					[=](vint32_t token) { return WString::Unmanaged(tokenId((TTokens)token)); },
					[=](vint32_t rule) { return WString::Unmanaged(ruleName(rule)); },
					[=](vint32_t state) { return WString::Unmanaged(stateLabel(state)); },
					[=](vint32_t switchId) { return WString::Unmanaged(switchName(switchId)); }
				);

				if (args.traceManager.concurrentCount == 1)
				{
					LogTraceExecution(
						L"Generated-" + parserName,
						caseName,
						[=](vint32_t type) { return WString::Unmanaged(typeName((TClasses)type)); },
						[=](vint32_t field) { return WString::Unmanaged(fieldName((TFields)field)); },
						[=](vint32_t token) { return WString::Unmanaged(tokenId((TTokens)token)); },
						[&](IAstInsReceiver& receiver)
						{
							args.traceManager.ExecuteTrace(args.rootTrace, receiver, args.tokens);
						});
				}
			});

		TEST_CATEGORY(L"Test " + parserName + L" Syntax")
		{
			Folder dirInput = FilePath(GetTestParserInputPath(parserName)) / L"Input";
			FilePath dirBaseline = FilePath(GetTestParserInputPath(parserName)) / L"Output";
			FilePath dirOutput = GetOutputDir(L"Generated-" + parserName);

			List<File> inputFiles;
			dirInput.GetFiles(inputFiles);
			for (auto&& inputFile : inputFiles)
			{
				caseName = inputFile.GetFilePath().GetName();
				if (caseName.Length() < 4 || caseName.Right(4) != L".txt") continue;
				caseName = caseName.Left(caseName.Length() - 4);

				TEST_CASE(caseName)
				{
					auto input = inputFile.ReadAllTextByBom();
					auto ast = parser.ParseModule(input);
					auto actualJson = PrintAstJson<TJsonVisitor>(ast);
					File(dirOutput / (L"Output[" + caseName + L"].json")).WriteAllText(actualJson, true, BomEncoder::Utf8);

					auto expectedJsonFile = File(dirBaseline / (caseName + L".json"));
					if (!expectedJsonFile.Exists()) return;
					TEST_PRINT(L"Compared with expectedJson");
					auto expectedJson = expectedJsonFile.ReadAllTextByBom();
					AssertLines(expectedJson, actualJson);
				});
			}
		});
	}
}
using namespace TestParser_Generated_TestObjects;

TEST_FILE
{
	TestParser<calculator::ModuleParser, calculator::json_visitor::ExprAstVisitor>(
		L"Calculator",
		calculator::ModuleParserStates::Module,
		&calculator::CalculatorTypeName,
		&calculator::CalculatorFieldName,
		&calculator::CalculatorTokenId,
		&calculator::ModuleParserRuleName,
		&calculator::ModuleParserStateLabel,
		&calculator::ModuleParserSwitchName
		);
	TestParser<ifelseambiguity::ModuleParser, ifelseambiguity::json_visitor::StatAstVisitor>(
		L"IfElseAmbiguity",
		ifelseambiguity::ModuleParserStates::Module,
		&ifelseambiguity::IfElseAmbiguityTypeName,
		&ifelseambiguity::IfElseAmbiguityFieldName,
		&ifelseambiguity::IfElseAmbiguityTokenId,
		&ifelseambiguity::ModuleParserRuleName,
		&ifelseambiguity::ModuleParserStateLabel,
		&ifelseambiguity::ModuleParserSwitchName
		);
	TestParser<ifelsepriority::ModuleParser, ifelsepriority::json_visitor::StatAstVisitor>(
		L"IfElsePriority",
		ifelsepriority::ModuleParserStates::Module,
		&ifelsepriority::IfElsePriorityTypeName,
		&ifelsepriority::IfElsePriorityFieldName,
		&ifelsepriority::IfElsePriorityTokenId,
		&ifelsepriority::ModuleParserRuleName,
		&ifelsepriority::ModuleParserStateLabel,
		&ifelsepriority::ModuleParserSwitchName
		);
	TestParser<genericambiguity::ModuleParser, genericambiguity::json_visitor::ExprAstVisitor>(
		L"GenericAmbiguity",
		genericambiguity::ModuleParserStates::Module,
		&genericambiguity::GenericAmbiguityTypeName,
		&genericambiguity::GenericAmbiguityFieldName,
		&genericambiguity::GenericAmbiguityTokenId,
		&genericambiguity::ModuleParserRuleName,
		&genericambiguity::ModuleParserStateLabel,
		&genericambiguity::ModuleParserSwitchName
		);
	TestParser<featuretest::ModuleParser, featuretest::json_visitor::FeatureAstVisitor>(
		L"FeatureTest",
		featuretest::ModuleParserStates::Module,
		&featuretest::FeatureTestTypeName,
		&featuretest::FeatureTestFieldName,
		&featuretest::FeatureTestTokenId,
		&featuretest::ModuleParserRuleName,
		&featuretest::ModuleParserStateLabel,
		&featuretest::ModuleParserSwitchName
		);
	TestParser<binaryop::ModuleParser, binaryop::json_visitor::ExprAstVisitor>(
		L"BinaryOp",
		binaryop::ModuleParserStates::Module,
		&binaryop::BinaryOpTypeName,
		&binaryop::BinaryOpFieldName,
		&binaryop::BinaryOpTokenId,
		&binaryop::ModuleParserRuleName,
		&binaryop::ModuleParserStateLabel,
		&binaryop::ModuleParserSwitchName
		);
}