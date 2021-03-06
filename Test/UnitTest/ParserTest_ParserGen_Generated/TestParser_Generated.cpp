#include "../../Source/Calculator/Generated/CalculatorExprAst_Json.h"
#include "../../Source/Calculator/Generated/CalculatorModuleParser.h"
#include "../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Json.h"
#include "../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityModuleParser.h"
#include "../../Source/IfElseAmbiguity2/Generated/IfElseAmbiguity2StatAst_Json.h"
#include "../../Source/IfElseAmbiguity2/Generated/IfElseAmbiguity2ModuleParser.h"
#include "../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Json.h"
#include "../../Source/IfElsePriority/Generated/IfElsePriorityModuleParser.h"
#include "../../Source/IfElseManual/Generated/IfElseManualStatAst_Json.h"
#include "../../Source/IfElseManual/Generated/IfElseManualModuleParser.h"
#include "../../Source/IfElseSwitch/Generated/IfElseSwitchStatAst_Json.h"
#include "../../Source/IfElseSwitch/Generated/IfElseSwitchModuleParser.h"
#include "../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Json.h"
#include "../../Source/GenericAmbiguity/Generated/GenericAmbiguityModuleParser.h"
#include "../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Json.h"
#include "../../Source/FeatureTest/Generated/FeatureTestModuleParser.h"
#include "../../Source/BinaryOp/Generated/BinaryOpExprAst_Json.h"
#include "../../Source/BinaryOp/Generated/BinaryOpModuleParser.h"
#include "../../Source/PrefixSubset/Generated/PrefixSubsetTypeOrExpr_Json.h"
#include "../../Source/PrefixSubset/Generated/PrefixSubsetModuleParser.h"
#include "../../Source/PrefixSubset2/Generated/PrefixSubset2TypeOrExpr_Json.h"
#include "../../Source/PrefixSubset2/Generated/PrefixSubset2ModuleParser.h"
#include "../../Source/PrefixSubset3/Generated/PrefixSubset3TypeOrExpr_Json.h"
#include "../../Source/PrefixSubset3/Generated/PrefixSubset3ModuleParser.h"
#include "../../Source/PrefixSubset4/Generated/PrefixSubset4TypeOrExpr_Json.h"
#include "../../Source/PrefixSubset4/Generated/PrefixSubset4ModuleParser.h"
#include "../../Source/PrefixSubset5/Generated/PrefixSubset5TypeOrExpr_Json.h"
#include "../../Source/PrefixSubset5/Generated/PrefixSubset5ModuleParser.h"
#include "../../Source/LogTrace.h"

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

namespace TestParser_Generated_TestObjects
{
	template<
		typename TParser,
		typename TJsonVisitor
	>
		void RunParserSingleTestFolder(
			TParser& parser,
			const WString& parserName,
			const WString& testFolder,
			FilePath dirOutput
		)
	{
		WString caseName;

		auto inputPath = GetTestParserInputPath(testFolder);
		Folder dirInput = FilePath(inputPath) / L"Input";
		FilePath dirBaseline = FilePath(inputPath) / L"Output";

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
	}

	template<
		typename TParser,
		typename TJsonVisitor
	>
		void RunParser(
			TParser& parser,
			const WString& parserName,
			const Array<WString>& testFolders
		)
	{
		TEST_CATEGORY(L"Test " + parserName + L" Syntax")
		{
			FilePath dirOutput = GetOutputDir(L"Generated-" + parserName);
			if (testFolders.Count() == 0)
			{
				RunParserSingleTestFolder<TParser, TJsonVisitor>(parser, parserName, parserName, dirOutput);
			}
			else
			{
				for (auto&& testFolder : From(testFolders))
				{
					TEST_CATEGORY(testFolder)
					{
						RunParserSingleTestFolder<TParser, TJsonVisitor>(parser, parserName, testFolder, dirOutput);
					});
				}
			}
		});
	}

	template<
		typename TParser,
		typename TJsonVisitor,
		typename TClasses,
		typename TFields,
		typename TTokens,
		typename ...TTestFolders
		>
	void TestParser(
		const wchar_t* parserNameRaw,
		const wchar_t* (*typeName)(TClasses),
		const wchar_t* (*fieldName)(TFields),
		const wchar_t* (*tokenId)(TTokens),
		const wchar_t* (*ruleName)(vint),
		const wchar_t* (*stateLabel)(vint),
		const wchar_t* (*switchName)(vint),
		TTestFolders&& ...testFolders
		)
	{
		auto parserName = WString::Unmanaged(parserNameRaw);
		TParser parser;
		WString caseName;

		parser.OnError.Add(
			[&](ErrorArgs& args)
			{
				args.throwError = true;
			});

		parser.OnEndOfInput.Add(
			[&](EndOfInputArgs& args)
			{
				auto& traceManager = *dynamic_cast<TraceManager*>(args.executor);
				LogTraceManager(
					L"Generated-" + parserName,
					caseName,
					args.executable,
					traceManager,
					args.rootTrace,
					args.tokens,
					[=](vint32_t type) { return WString::Unmanaged(typeName((TClasses)type)); },
					[=](vint32_t field) { return WString::Unmanaged(fieldName((TFields)field)); },
					[=](vint32_t token) { return WString::Unmanaged(tokenId((TTokens)token)); },
					[=](vint32_t rule) { return WString::Unmanaged(ruleName(rule)); },
					[=](vint32_t state) { return WString::Unmanaged(stateLabel(state)); },
					[=](vint32_t switchId) { return WString::Unmanaged(switchName(switchId)); }
				);

				if (traceManager.concurrentCount == 1)
				{
					LogTraceExecution(
						L"Generated-" + parserName,
						caseName,
						[=](vint32_t type) { return WString::Unmanaged(typeName((TClasses)type)); },
						[=](vint32_t field) { return WString::Unmanaged(fieldName((TFields)field)); },
						[=](vint32_t token) { return WString::Unmanaged(tokenId((TTokens)token)); },
						[&](IAstInsReceiver& receiver)
						{
							traceManager.ExecuteTrace(args.rootTrace, receiver, args.tokens);
						});
				}
			});

		Array<WString> testFolderArray;
		if constexpr (sizeof...(testFolders) > 0)
		{
			const wchar_t* testFolderRawArray[] = { testFolders... };
			testFolderArray.Resize(sizeof...(testFolders));
			for (vint i = 0; i < sizeof...(testFolders); i++)
			{
				testFolderArray[i] =  WString::Unmanaged(testFolderRawArray[i]);
			}
		}
		RunParser<TParser, TJsonVisitor>(parser, parserName, testFolderArray);
	}
}
using namespace TestParser_Generated_TestObjects;

TEST_FILE
{
	TestParser<calculator::ModuleParser, calculator::json_visitor::ExprAstVisitor>(
		L"Calculator",
		&calculator::CalculatorTypeName,
		&calculator::CalculatorFieldName,
		&calculator::CalculatorTokenId,
		&calculator::ModuleParserRuleName,
		&calculator::ModuleParserStateLabel,
		&calculator::ModuleParserSwitchName
		);
	TestParser<ifelseambiguity::ModuleParser, ifelseambiguity::json_visitor::StatAstVisitor>(
		L"IfElseAmbiguity",
		&ifelseambiguity::IfElseAmbiguityTypeName,
		&ifelseambiguity::IfElseAmbiguityFieldName,
		&ifelseambiguity::IfElseAmbiguityTokenId,
		&ifelseambiguity::ModuleParserRuleName,
		&ifelseambiguity::ModuleParserStateLabel,
		&ifelseambiguity::ModuleParserSwitchName
		);
	TestParser<ifelseambiguity2::ModuleParser, ifelseambiguity2::json_visitor::StatAstVisitor>(
		L"IfElseAmbiguity2",
		&ifelseambiguity2::IfElseAmbiguity2TypeName,
		&ifelseambiguity2::IfElseAmbiguity2FieldName,
		&ifelseambiguity2::IfElseAmbiguity2TokenId,
		&ifelseambiguity2::ModuleParserRuleName,
		&ifelseambiguity2::ModuleParserStateLabel,
		&ifelseambiguity2::ModuleParserSwitchName
		);
	TestParser<ifelsepriority::ModuleParser, ifelsepriority::json_visitor::StatAstVisitor>(
		L"IfElsePriority",
		&ifelsepriority::IfElsePriorityTypeName,
		&ifelsepriority::IfElsePriorityFieldName,
		&ifelsepriority::IfElsePriorityTokenId,
		&ifelsepriority::ModuleParserRuleName,
		&ifelsepriority::ModuleParserStateLabel,
		&ifelsepriority::ModuleParserSwitchName
		);
	TestParser<ifelsemanual::ModuleParser, ifelsemanual::json_visitor::StatAstVisitor>(
		L"IfElseManual",
		&ifelsemanual::IfElseManualTypeName,
		&ifelsemanual::IfElseManualFieldName,
		&ifelsemanual::IfElseManualTokenId,
		&ifelsemanual::ModuleParserRuleName,
		&ifelsemanual::ModuleParserStateLabel,
		&ifelsemanual::ModuleParserSwitchName
		);
	TestParser<ifelseswitch::ModuleParser, ifelseswitch::json_visitor::StatAstVisitor>(
		L"IfElseSwitch",
		&ifelseswitch::IfElseSwitchTypeName,
		&ifelseswitch::IfElseSwitchFieldName,
		&ifelseswitch::IfElseSwitchTokenId,
		&ifelseswitch::ModuleParserRuleName,
		&ifelseswitch::ModuleParserStateLabel,
		&ifelseswitch::ModuleParserSwitchName
		);
	TestParser<genericambiguity::ModuleParser, genericambiguity::json_visitor::ExprAstVisitor>(
		L"GenericAmbiguity",
		&genericambiguity::GenericAmbiguityTypeName,
		&genericambiguity::GenericAmbiguityFieldName,
		&genericambiguity::GenericAmbiguityTokenId,
		&genericambiguity::ModuleParserRuleName,
		&genericambiguity::ModuleParserStateLabel,
		&genericambiguity::ModuleParserSwitchName
		);
	TestParser<featuretest::ModuleParser, featuretest::json_visitor::FeatureAstVisitor>(
		L"FeatureTest",
		&featuretest::FeatureTestTypeName,
		&featuretest::FeatureTestFieldName,
		&featuretest::FeatureTestTokenId,
		&featuretest::ModuleParserRuleName,
		&featuretest::ModuleParserStateLabel,
		&featuretest::ModuleParserSwitchName
		);
	TestParser<binaryop::ModuleParser, binaryop::json_visitor::ExprAstVisitor>(
		L"BinaryOp",
		&binaryop::BinaryOpTypeName,
		&binaryop::BinaryOpFieldName,
		&binaryop::BinaryOpTokenId,
		&binaryop::ModuleParserRuleName,
		&binaryop::ModuleParserStateLabel,
		&binaryop::ModuleParserSwitchName
		);
	TestParser<prefixsubset::ModuleParser, prefixsubset::json_visitor::TypeOrExprVisitor>(
		L"PrefixSubset",
		&prefixsubset::PrefixSubsetTypeName,
		&prefixsubset::PrefixSubsetFieldName,
		&prefixsubset::PrefixSubsetTokenId,
		&prefixsubset::ModuleParserRuleName,
		&prefixsubset::ModuleParserStateLabel,
		&prefixsubset::ModuleParserSwitchName,
		L"TestCase_PrefixSubset"
		);
	TestParser<prefixsubset2::ModuleParser, prefixsubset2::json_visitor::TypeOrExprVisitor>(
		L"PrefixSubset2",
		&prefixsubset2::PrefixSubset2TypeName,
		&prefixsubset2::PrefixSubset2FieldName,
		&prefixsubset2::PrefixSubset2TokenId,
		&prefixsubset2::ModuleParserRuleName,
		&prefixsubset2::ModuleParserStateLabel,
		&prefixsubset2::ModuleParserSwitchName,
		L"TestCase_PrefixSubset"
		);
	TestParser<prefixsubset3::ModuleParser, prefixsubset3::json_visitor::TypeOrExprVisitor>(
		L"PrefixSubset3",
		&prefixsubset3::PrefixSubset3TypeName,
		&prefixsubset3::PrefixSubset3FieldName,
		&prefixsubset3::PrefixSubset3TokenId,
		&prefixsubset3::ModuleParserRuleName,
		&prefixsubset3::ModuleParserStateLabel,
		&prefixsubset3::ModuleParserSwitchName,
		L"TestCase_PrefixSubset",
		L"TestCase_PrefixSubset_CtorExpr"
		);
	TestParser<prefixsubset4::ModuleParser, prefixsubset4::json_visitor::TypeOrExprVisitor>(
		L"PrefixSubset4",
		&prefixsubset4::PrefixSubset4TypeName,
		&prefixsubset4::PrefixSubset4FieldName,
		&prefixsubset4::PrefixSubset4TokenId,
		&prefixsubset4::ModuleParserRuleName,
		&prefixsubset4::ModuleParserStateLabel,
		&prefixsubset4::ModuleParserSwitchName,
		L"TestCase_PrefixSubset",
		L"TestCase_PrefixSubset_CtorExpr"
		);
	TestParser<prefixsubset5::ModuleParser, prefixsubset5::json_visitor::TypeOrExprVisitor>(
		L"PrefixSubset5",
		&prefixsubset5::PrefixSubset5TypeName,
		&prefixsubset5::PrefixSubset5FieldName,
		&prefixsubset5::PrefixSubset5TokenId,
		&prefixsubset5::ModuleParserRuleName,
		&prefixsubset5::ModuleParserStateLabel,
		&prefixsubset5::ModuleParserSwitchName,
		L"TestCase_PrefixSubset",
		L"TestCase_PrefixSubset_CtorExpr"
		);
}