#include "../../Source/Calculator/Generated/CalculatorExprAst_Json.h"
#include "../../Source/Calculator/Generated/CalculatorModuleParser.h"
#include "../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Json.h"
#include "../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityModuleParser.h"
#include "../../Source/IfElseAmbiguity2/Generated/IfElseAmbiguity2StatAst_Json.h"
#include "../../Source/IfElseAmbiguity2/Generated/IfElseAmbiguity2ModuleParser.h"
#include "../../Source/IfElseAmbiguityOnStat/Generated/IfElseAmbiguityOnStatStatAst_Json.h"
#include "../../Source/IfElseAmbiguityOnStat/Generated/IfElseAmbiguityOnStatModuleParser.h"
#include "../../Source/IfElseAmbiguityOnStat2/Generated/IfElseAmbiguityOnStat2StatAst_Json.h"
#include "../../Source/IfElseAmbiguityOnStat2/Generated/IfElseAmbiguityOnStat2ModuleParser.h"
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
#include "../../Source/PrefixMerge1_Lri/Generated/PrefixMerge1_LriTypeOrExpr_Json.h"
#include "../../Source/PrefixMerge1_Lri/Generated/PrefixMerge1_LriModuleParser.h"
#include "../../Source/PrefixMerge2_LriRequired/Generated/PrefixMerge2_LriRequiredTypeOrExpr_Json.h"
#include "../../Source/PrefixMerge2_LriRequired/Generated/PrefixMerge2_LriRequiredModuleParser.h"
#include "../../Source/PrefixMerge3_LriNested/Generated/PrefixMerge3_LriNestedTypeOrExpr_Json.h"
#include "../../Source/PrefixMerge3_LriNested/Generated/PrefixMerge3_LriNestedModuleParser.h"
#include "../../Source/PrefixMerge4_LriMultiple/Generated/PrefixMerge4_LriMultipleTypeOrExpr_Json.h"
#include "../../Source/PrefixMerge4_LriMultiple/Generated/PrefixMerge4_LriMultipleModuleParser.h"
#include "../../Source/PrefixMerge5_Pm/Generated/PrefixMerge5_PmTypeOrExpr_Json.h"
#include "../../Source/PrefixMerge5_Pm/Generated/PrefixMerge5_PmModuleParser.h"
#include "../../Source/PrefixMerge6_Pm2/Generated/PrefixMerge6_Pm2TypeOrExpr_Json.h"
#include "../../Source/PrefixMerge6_Pm2/Generated/PrefixMerge6_Pm2ModuleParser.h"
#include "../../Source/PrefixMerge7_PmSwitch/Generated/PrefixMerge7_PmSwitchTypeOrExpr_Json.h"
#include "../../Source/PrefixMerge7_PmSwitch/Generated/PrefixMerge7_PmSwitchModuleParser.h"
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
			WString& caseName,
			FilePath dirOutput
		)
	{
		//if (parserName != L"IfElseAmbiguity") return;
		auto inputPath = GetTestParserInputPath(testFolder);
		Folder dirInput = FilePath(inputPath) / L"Input";
		FilePath dirBaseline = FilePath(inputPath) / L"ParserLog";

		List<File> inputFiles;
		dirInput.GetFiles(inputFiles);
		for (auto&& inputFile : inputFiles)
		{
			caseName = inputFile.GetFilePath().GetName();
			if (caseName.Length() < 4 || caseName.Right(4) != L".txt") continue;
			caseName = caseName.Left(caseName.Length() - 4);
			//if (caseName != L"IfElseAmbiguity1") continue;

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
			WString& caseName,
			const Array<WString>& testFolders
		)
	{
		TEST_CATEGORY(L"Test " + parserName + L" Syntax")
		{
			FilePath dirOutput = GetOutputDir(L"Generated-" + parserName);
			if (testFolders.Count() == 0)
			{
				RunParserSingleTestFolder<TParser, TJsonVisitor>(parser, parserName, parserName, caseName, dirOutput);
			}
			else
			{
				for (auto&& testFolder : From(testFolders))
				{
					TEST_CATEGORY(testFolder)
					{
						RunParserSingleTestFolder<TParser, TJsonVisitor>(parser, parserName, testFolder, caseName, dirOutput);
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

		parser.OnTraceProcessing.Add(
			[&](TraceProcessingArgs& args)
			{
				auto& traceManager = *dynamic_cast<TraceManager*>(args.executor);
				LogTraceManager(
					L"Generated-" + parserName,
					caseName,
					args.executable,
					traceManager,
					args.phase,
					args.tokens,
					[=](vint32_t type) { return WString::Unmanaged(typeName((TClasses)type)); },
					[=](vint32_t field) { return WString::Unmanaged(fieldName((TFields)field)); },
					[=](vint32_t token) { return WString::Unmanaged(tokenId((TTokens)token)); },
					[=](vint32_t rule) { return WString::Unmanaged(ruleName(rule)); },
					[=](vint32_t state) { return WString::Unmanaged(stateLabel(state)); },
					[=](vint32_t switchId) { return WString::Unmanaged(switchName(switchId)); }
				);
			});

		parser.OnReadyToExecute.Add(
			[&](ReadyToExecuteArgs& args)
			{
				auto& traceManager = *dynamic_cast<TraceManager*>(args.executor);
				LogTraceExecution(
					L"Generated-" + parserName,
					caseName,
					[=](vint32_t type) { return WString::Unmanaged(typeName((TClasses)type)); },
					[=](vint32_t field) { return WString::Unmanaged(fieldName((TFields)field)); },
					[=](vint32_t token) { return WString::Unmanaged(tokenId((TTokens)token)); },
					[&](IAstInsReceiver& receiver)
					{
						traceManager.ExecuteTrace(receiver, args.tokens);
					});
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
		RunParser<TParser, TJsonVisitor>(parser, parserName, caseName, testFolderArray);
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
		&ifelseambiguity::ModuleParserSwitchName,
		L"TestCase_IfElseAmbiguity"
		);
	TestParser<ifelseambiguity2::ModuleParser, ifelseambiguity2::json_visitor::StatAstVisitor>(
		L"IfElseAmbiguity2",
		&ifelseambiguity2::IfElseAmbiguity2TypeName,
		&ifelseambiguity2::IfElseAmbiguity2FieldName,
		&ifelseambiguity2::IfElseAmbiguity2TokenId,
		&ifelseambiguity2::ModuleParserRuleName,
		&ifelseambiguity2::ModuleParserStateLabel,
		&ifelseambiguity2::ModuleParserSwitchName,
		L"TestCase_IfElseAmbiguity"
		);
	TestParser<ifelseambiguityonstat::ModuleParser, ifelseambiguityonstat::json_visitor::StatAstVisitor>(
		L"IfElseAmbiguityOnStat",
		&ifelseambiguityonstat::IfElseAmbiguityOnStatTypeName,
		&ifelseambiguityonstat::IfElseAmbiguityOnStatFieldName,
		&ifelseambiguityonstat::IfElseAmbiguityOnStatTokenId,
		&ifelseambiguityonstat::ModuleParserRuleName,
		&ifelseambiguityonstat::ModuleParserStateLabel,
		&ifelseambiguityonstat::ModuleParserSwitchName,
		L"TestCase_IfElseAmbiguityOnStat"
		);
	TestParser<ifelseambiguityonstat2::ModuleParser, ifelseambiguityonstat2::json_visitor::StatAstVisitor>(
		L"IfElseAmbiguityOnStat2",
		&ifelseambiguityonstat2::IfElseAmbiguityOnStat2TypeName,
		&ifelseambiguityonstat2::IfElseAmbiguityOnStat2FieldName,
		&ifelseambiguityonstat2::IfElseAmbiguityOnStat2TokenId,
		&ifelseambiguityonstat2::ModuleParserRuleName,
		&ifelseambiguityonstat2::ModuleParserStateLabel,
		&ifelseambiguityonstat2::ModuleParserSwitchName,
		L"TestCase_IfElseAmbiguityOnStat"
		);
	TestParser<ifelsepriority::ModuleParser, ifelsepriority::json_visitor::StatAstVisitor>(
		L"IfElsePriority",
		&ifelsepriority::IfElsePriorityTypeName,
		&ifelsepriority::IfElsePriorityFieldName,
		&ifelsepriority::IfElsePriorityTokenId,
		&ifelsepriority::ModuleParserRuleName,
		&ifelsepriority::ModuleParserStateLabel,
		&ifelsepriority::ModuleParserSwitchName,
		L"TestCase_IfElse"
		);
	TestParser<ifelsemanual::ModuleParser, ifelsemanual::json_visitor::StatAstVisitor>(
		L"IfElseManual",
		&ifelsemanual::IfElseManualTypeName,
		&ifelsemanual::IfElseManualFieldName,
		&ifelsemanual::IfElseManualTokenId,
		&ifelsemanual::ModuleParserRuleName,
		&ifelsemanual::ModuleParserStateLabel,
		&ifelsemanual::ModuleParserSwitchName,
		L"TestCase_IfElse"
		);
	TestParser<ifelseswitch::ModuleParser, ifelseswitch::json_visitor::StatAstVisitor>(
		L"IfElseSwitch",
		&ifelseswitch::IfElseSwitchTypeName,
		&ifelseswitch::IfElseSwitchFieldName,
		&ifelseswitch::IfElseSwitchTokenId,
		&ifelseswitch::ModuleParserRuleName,
		&ifelseswitch::ModuleParserStateLabel,
		&ifelseswitch::ModuleParserSwitchName,
		L"TestCase_IfElse"
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
	TestParser<prefixmerge1_lri::ModuleParser, prefixmerge1_lri::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge1_Lri",
		&prefixmerge1_lri::PrefixMerge1_LriTypeName,
		&prefixmerge1_lri::PrefixMerge1_LriFieldName,
		&prefixmerge1_lri::PrefixMerge1_LriTokenId,
		&prefixmerge1_lri::ModuleParserRuleName,
		&prefixmerge1_lri::ModuleParserStateLabel,
		&prefixmerge1_lri::ModuleParserSwitchName,
		L"TestCase_PrefixMerge"
		);
	TestParser<prefixmerge2_lrirequired::ModuleParser, prefixmerge2_lrirequired::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge2_LriRequired",
		&prefixmerge2_lrirequired::PrefixMerge2_LriRequiredTypeName,
		&prefixmerge2_lrirequired::PrefixMerge2_LriRequiredFieldName,
		&prefixmerge2_lrirequired::PrefixMerge2_LriRequiredTokenId,
		&prefixmerge2_lrirequired::ModuleParserRuleName,
		&prefixmerge2_lrirequired::ModuleParserStateLabel,
		&prefixmerge2_lrirequired::ModuleParserSwitchName,
		L"TestCase_PrefixMerge"
		);
	TestParser<prefixmerge3_lrinested::ModuleParser, prefixmerge3_lrinested::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge3_LriNested",
		&prefixmerge3_lrinested::PrefixMerge3_LriNestedTypeName,
		&prefixmerge3_lrinested::PrefixMerge3_LriNestedFieldName,
		&prefixmerge3_lrinested::PrefixMerge3_LriNestedTokenId,
		&prefixmerge3_lrinested::ModuleParserRuleName,
		&prefixmerge3_lrinested::ModuleParserStateLabel,
		&prefixmerge3_lrinested::ModuleParserSwitchName,
		L"TestCase_PrefixMerge",
		L"TestCase_PrefixMerge_CtorExpr"
		);
	TestParser<prefixmerge4_lrimultiple::ModuleParser, prefixmerge4_lrimultiple::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge4_LriMultiple",
		&prefixmerge4_lrimultiple::PrefixMerge4_LriMultipleTypeName,
		&prefixmerge4_lrimultiple::PrefixMerge4_LriMultipleFieldName,
		&prefixmerge4_lrimultiple::PrefixMerge4_LriMultipleTokenId,
		&prefixmerge4_lrimultiple::ModuleParserRuleName,
		&prefixmerge4_lrimultiple::ModuleParserStateLabel,
		&prefixmerge4_lrimultiple::ModuleParserSwitchName,
		L"TestCase_PrefixMerge",
		L"TestCase_PrefixMerge_CtorExpr"
		);
	TestParser<prefixmerge5_pm::ModuleParser, prefixmerge5_pm::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge5_Pm",
		&prefixmerge5_pm::PrefixMerge5_PmTypeName,
		&prefixmerge5_pm::PrefixMerge5_PmFieldName,
		&prefixmerge5_pm::PrefixMerge5_PmTokenId,
		&prefixmerge5_pm::ModuleParserRuleName,
		&prefixmerge5_pm::ModuleParserStateLabel,
		&prefixmerge5_pm::ModuleParserSwitchName,
		L"TestCase_PrefixMerge",
		L"TestCase_PrefixMerge_CtorExpr"
		);
	TestParser<prefixmerge6_pm2::ModuleParser, prefixmerge6_pm2::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge6_Pm2",
		&prefixmerge6_pm2::PrefixMerge6_Pm2TypeName,
		&prefixmerge6_pm2::PrefixMerge6_Pm2FieldName,
		&prefixmerge6_pm2::PrefixMerge6_Pm2TokenId,
		&prefixmerge6_pm2::ModuleParserRuleName,
		&prefixmerge6_pm2::ModuleParserStateLabel,
		&prefixmerge6_pm2::ModuleParserSwitchName,
		L"TestCase_PrefixMerge",
		L"TestCase_PrefixMerge_CtorExpr",
		L"TestCase_PrefixMerge_ThrowComma"
		);
	TestParser<prefixmerge7_pmswitch::ModuleParser, prefixmerge7_pmswitch::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge7_PmSwitch",
		&prefixmerge7_pmswitch::PrefixMerge7_PmSwitchTypeName,
		&prefixmerge7_pmswitch::PrefixMerge7_PmSwitchFieldName,
		&prefixmerge7_pmswitch::PrefixMerge7_PmSwitchTokenId,
		&prefixmerge7_pmswitch::ModuleParserRuleName,
		&prefixmerge7_pmswitch::ModuleParserStateLabel,
		&prefixmerge7_pmswitch::ModuleParserSwitchName,
		L"TestCase_PrefixMerge",
		L"TestCase_PrefixMerge_CtorExpr",
		L"TestCase_PrefixMerge_ThrowComma",
		L"TestCase_PrefixMerge_Generic"
		);
}