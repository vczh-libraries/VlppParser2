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
#include "../../Source/PrefixMerge8_PmVariadic/Generated/PrefixMerge8_PmVariadicTypeOrExpr_Json.h"
#include "../../Source/PrefixMerge8_PmVariadic/Generated/PrefixMerge8_PmVariadicModuleParser.h"
#include "../../Source/PrefixMerge9_PmLoop/Generated/PrefixMerge9_PmLoopFile_Json.h"
#include "../../Source/PrefixMerge9_PmLoop/Generated/PrefixMerge9_PmLoopModuleParser.h"
#include "../../Source/LogTrace.h"

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

// #define PAUSE_CASE L"PrefixMerge9_PmLoop"
// #define PAUSE_INPUT L"Class"
// #define PAUSE_MODULE L"Module-"

namespace TestParser_Generated_TestObjects
{
	vint inputDiscovered = 0;
	vint parsedSuccessfully = 0;
	vint comparedWithBaseline = 0;


	namespace parser_features
	{
		void TestExprModule(...);

		template<typename TParser>
		auto TestExprModule(TParser& parser) -> decltype(parser.ParseExprModule(std::declval<const WString&>(), std::declval<vint>()));

		template<typename TParser, typename = void>
		struct HasExprModule
		{
			static constexpr bool Value = !std::is_same_v<void, decltype(TestExprModule(std::declval<TParser&>()))>;
		};

		void TestTypeModule(...);

		template<typename TParser>
		auto TestTypeModule(TParser& parser) -> decltype(parser.ParseTypeModule(std::declval<const WString&>(), std::declval<vint>()));

		template<typename TParser>
		struct HasTypeModule
		{
			static constexpr bool Value = !std::is_same_v<void, decltype(TestTypeModule(std::declval<TParser&>()))>;
		};
	}

	template<
		typename TParser,
		typename TJsonVisitor
	>
		void RunParserSingleTestFolder(
			TParser& parser,
			const WString& parserName,
			const WString& testFolder,
			WString& displayCaseName,
			FilePath dirOutput
		)
	{
#ifdef PAUSE_CASE
		if (parserName != PAUSE_CASE) return;
#endif
		auto inputPath = GetTestParserInputPath(testFolder);
		Folder dirInput = FilePath(inputPath) / L"Input";
		FilePath dirBaseline = FilePath(inputPath) / L"Output";

		List<File> inputFiles;
		dirInput.GetFiles(inputFiles);

		auto executeTestCases = [&](List<WString>& executedCaseNames, auto&& parserCallback, const WString& caseModule = WString::Empty, Regex* regexFilter = nullptr, FilePath additionalOutput = {})
		{
			for (auto&& inputFile : inputFiles)
			{
				auto caseName = inputFile.GetFilePath().GetName();
				if (caseName.Length() < 4 || caseName.Right(4) != L".txt") continue;
				caseName = caseName.Left(caseName.Length() - 4);
				if (regexFilter)
				{
					auto match = regexFilter->MatchHead(caseName);
					if (!match || match->Result().Length() != caseName.Length()) continue;
				}
				executedCaseNames.Add(caseName);
#ifdef PAUSE_INPUT
				if (caseName != PAUSE_INPUT || caseModule != PAUSE_MODULE ) continue;
#endif
	
				TEST_CASE(caseName)
				{
					displayCaseName = caseModule + caseName;
					inputDiscovered++;

					auto input = inputFile.ReadAllTextByBom();
					auto ast = parserCallback(input);
					parsedSuccessfully++;

					auto actualJson = PrintAstJson<TJsonVisitor>(ast);
					File(dirOutput / (L"Output[" + caseName + L"]" + caseModule + L".json")).WriteAllText(actualJson, true, BomEncoder::Utf8);

					File expectedJsonFile;
					if (!additionalOutput.IsRoot())
					{
						File jsonFile = additionalOutput / (caseName + L".json");
						if (jsonFile.Exists()) expectedJsonFile = jsonFile;
					}
					if (expectedJsonFile.GetFilePath().IsRoot())
					{
						File jsonFile = dirBaseline / (caseName + L".json");
						if (jsonFile.Exists()) expectedJsonFile = jsonFile;
					}

					if (!expectedJsonFile.GetFilePath().IsRoot())
					{
						comparedWithBaseline++;
						TEST_PRINT(L"Compared with: " + dirBaseline.GetRelativePathFor(expectedJsonFile.GetFilePath()));
						auto expectedJson = expectedJsonFile.ReadAllTextByBom();
						AssertLines(expectedJson, actualJson);
					}
				});
			}
		};

		constexpr bool HasExprModule = parser_features::HasExprModule<TParser>::Value;
		constexpr bool HasTypeModule = parser_features::HasTypeModule<TParser>::Value;
		constexpr bool HasExtraFeatures = HasExprModule || HasTypeModule;
		static_assert(HasExprModule == HasTypeModule);

		if constexpr (HasExtraFeatures)
		{
			File fileExprList = FilePath(inputPath) / L"ListExpr.txt";
			File fileTypeList = FilePath(inputPath) / L"ListType.txt";
			TEST_CASE_ASSERT(fileExprList.Exists());
			TEST_CASE_ASSERT(fileTypeList.Exists());
			WString contentExprList = fileExprList.ReadAllTextByBom();
			WString contentTypeList = fileTypeList.ReadAllTextByBom();

			List<WString> allCaseNames, filteredCaseNames;
			TEST_CATEGORY(L"ParseModule")
			{
				executeTestCases(
					allCaseNames,
					[&](auto&& input) { return parser.ParseModule(input); },
					WString::Unmanaged(L"Module-")
					);
			});

			if (contentExprList != L"")
			{
				TEST_CATEGORY(L"ParseExprModule")
				{
					Regex regexFilter(contentExprList);
					executeTestCases(
						filteredCaseNames,
						[&](auto&& input) { return parser.ParseExprModule(input); },
						WString::Unmanaged(L"ExprModule-"),
						&regexFilter,
						dirBaseline / L"Expr"
						);
				});
			}

			if (contentTypeList != L"")
			{
				TEST_CATEGORY(L"ParseTypeModule")
				{
					Regex regexFilter(contentTypeList);
					executeTestCases(
						filteredCaseNames,
						[&](auto&& input) { return parser.ParseTypeModule(input); },
						WString::Unmanaged(L"TypeModule-"),
						&regexFilter,
						dirBaseline / L"Type"
						);
				});
			}

			TEST_CASE_ASSERT(
				CompareEnumerable(
					From(allCaseNames).OrderBySelf(),
					From(filteredCaseNames).Distinct().OrderBySelf()
				) == 0
			);
		}
		else
		{
			List<WString> caseNames;
			executeTestCases(
				caseNames,
				[&](auto&& input) { return parser.ParseModule(input); }
				);
		}
	}

	template<
		typename TParser,
		typename TJsonVisitor
	>
		void RunParser(
			TParser& parser,
			const WString& parserName,
			WString& displayCaseName,
			const Array<WString>& testFolders
		)
	{
		TEST_CATEGORY(L"Test " + parserName + L" Syntax")
		{
			FilePath dirOutput = GetOutputDir(L"Generated-" + parserName);
			if (testFolders.Count() == 0)
			{
				RunParserSingleTestFolder<TParser, TJsonVisitor>(parser, parserName, parserName, displayCaseName, dirOutput);
			}
			else
			{
				for (auto&& testFolder : From(testFolders))
				{
					TEST_CATEGORY(testFolder)
					{
						RunParserSingleTestFolder<TParser, TJsonVisitor>(parser, parserName, testFolder, displayCaseName, dirOutput);
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
		TTestFolders&& ...testFolders
		)
	{
		auto parserName = WString::Unmanaged(parserNameRaw);
		TParser parser;
		WString displayCaseName;

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
					displayCaseName,
					args.executable,
					traceManager,
					args.phase,
					args.tokens,
					[=](vint32_t type) { return WString::Unmanaged(typeName((TClasses)type)); },
					[=](vint32_t field) { return WString::Unmanaged(fieldName((TFields)field)); },
					[=](vint32_t token) { return WString::Unmanaged(tokenId((TTokens)token)); },
					[=](vint32_t rule) { return WString::Unmanaged(ruleName(rule)); },
					[=](vint32_t state) { return WString::Unmanaged(stateLabel(state)); }
				);
			});

		parser.OnReadyToExecute.Add(
			[&](ReadyToExecuteArgs& args)
			{
				auto& traceManager = *dynamic_cast<TraceManager*>(args.executor);
				LogTraceExecution(
					L"Generated-" + parserName,
					displayCaseName,
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
		RunParser<TParser, TJsonVisitor>(parser, parserName, displayCaseName, testFolderArray);
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
		&calculator::ModuleParserStateLabel
		);
	TestParser<ifelseambiguity::ModuleParser, ifelseambiguity::json_visitor::StatAstVisitor>(
		L"IfElseAmbiguity",
		&ifelseambiguity::IfElseAmbiguityTypeName,
		&ifelseambiguity::IfElseAmbiguityFieldName,
		&ifelseambiguity::IfElseAmbiguityTokenId,
		&ifelseambiguity::ModuleParserRuleName,
		&ifelseambiguity::ModuleParserStateLabel,
		L"TestCase_IfElseAmbiguity"
		);
	TestParser<ifelseambiguity2::ModuleParser, ifelseambiguity2::json_visitor::StatAstVisitor>(
		L"IfElseAmbiguity2",
		&ifelseambiguity2::IfElseAmbiguity2TypeName,
		&ifelseambiguity2::IfElseAmbiguity2FieldName,
		&ifelseambiguity2::IfElseAmbiguity2TokenId,
		&ifelseambiguity2::ModuleParserRuleName,
		&ifelseambiguity2::ModuleParserStateLabel,
		L"TestCase_IfElseAmbiguity"
		);
	TestParser<ifelseambiguityonstat::ModuleParser, ifelseambiguityonstat::json_visitor::StatAstVisitor>(
		L"IfElseAmbiguityOnStat",
		&ifelseambiguityonstat::IfElseAmbiguityOnStatTypeName,
		&ifelseambiguityonstat::IfElseAmbiguityOnStatFieldName,
		&ifelseambiguityonstat::IfElseAmbiguityOnStatTokenId,
		&ifelseambiguityonstat::ModuleParserRuleName,
		&ifelseambiguityonstat::ModuleParserStateLabel,
		L"TestCase_IfElseAmbiguityOnStat"
		);
	TestParser<ifelseambiguityonstat2::ModuleParser, ifelseambiguityonstat2::json_visitor::StatAstVisitor>(
		L"IfElseAmbiguityOnStat2",
		&ifelseambiguityonstat2::IfElseAmbiguityOnStat2TypeName,
		&ifelseambiguityonstat2::IfElseAmbiguityOnStat2FieldName,
		&ifelseambiguityonstat2::IfElseAmbiguityOnStat2TokenId,
		&ifelseambiguityonstat2::ModuleParserRuleName,
		&ifelseambiguityonstat2::ModuleParserStateLabel,
		L"TestCase_IfElseAmbiguityOnStat"
		);
	TestParser<ifelsepriority::ModuleParser, ifelsepriority::json_visitor::StatAstVisitor>(
		L"IfElsePriority",
		&ifelsepriority::IfElsePriorityTypeName,
		&ifelsepriority::IfElsePriorityFieldName,
		&ifelsepriority::IfElsePriorityTokenId,
		&ifelsepriority::ModuleParserRuleName,
		&ifelsepriority::ModuleParserStateLabel,
		L"TestCase_IfElse"
		);
	TestParser<ifelsemanual::ModuleParser, ifelsemanual::json_visitor::StatAstVisitor>(
		L"IfElseManual",
		&ifelsemanual::IfElseManualTypeName,
		&ifelsemanual::IfElseManualFieldName,
		&ifelsemanual::IfElseManualTokenId,
		&ifelsemanual::ModuleParserRuleName,
		&ifelsemanual::ModuleParserStateLabel,
		L"TestCase_IfElse"
		);
	TestParser<ifelseswitch::ModuleParser, ifelseswitch::json_visitor::StatAstVisitor>(
		L"IfElseSwitch",
		&ifelseswitch::IfElseSwitchTypeName,
		&ifelseswitch::IfElseSwitchFieldName,
		&ifelseswitch::IfElseSwitchTokenId,
		&ifelseswitch::ModuleParserRuleName,
		&ifelseswitch::ModuleParserStateLabel,
		L"TestCase_IfElse"
		);
	TestParser<genericambiguity::ModuleParser, genericambiguity::json_visitor::ExprAstVisitor>(
		L"GenericAmbiguity",
		&genericambiguity::GenericAmbiguityTypeName,
		&genericambiguity::GenericAmbiguityFieldName,
		&genericambiguity::GenericAmbiguityTokenId,
		&genericambiguity::ModuleParserRuleName,
		&genericambiguity::ModuleParserStateLabel
		);
	TestParser<featuretest::ModuleParser, featuretest::json_visitor::FeatureAstVisitor>(
		L"FeatureTest",
		&featuretest::FeatureTestTypeName,
		&featuretest::FeatureTestFieldName,
		&featuretest::FeatureTestTokenId,
		&featuretest::ModuleParserRuleName,
		&featuretest::ModuleParserStateLabel
		);
	TestParser<binaryop::ModuleParser, binaryop::json_visitor::ExprAstVisitor>(
		L"BinaryOp",
		&binaryop::BinaryOpTypeName,
		&binaryop::BinaryOpFieldName,
		&binaryop::BinaryOpTokenId,
		&binaryop::ModuleParserRuleName,
		&binaryop::ModuleParserStateLabel
		);
	TestParser<prefixmerge1_lri::ModuleParser, prefixmerge1_lri::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge1_Lri",
		&prefixmerge1_lri::PrefixMerge1_LriTypeName,
		&prefixmerge1_lri::PrefixMerge1_LriFieldName,
		&prefixmerge1_lri::PrefixMerge1_LriTokenId,
		&prefixmerge1_lri::ModuleParserRuleName,
		&prefixmerge1_lri::ModuleParserStateLabel,
		L"TestCase_Cpp/Basic",
		L"TestCase_Cpp/Ambiguous1"
		);
	TestParser<prefixmerge2_lrirequired::ModuleParser, prefixmerge2_lrirequired::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge2_LriRequired",
		&prefixmerge2_lrirequired::PrefixMerge2_LriRequiredTypeName,
		&prefixmerge2_lrirequired::PrefixMerge2_LriRequiredFieldName,
		&prefixmerge2_lrirequired::PrefixMerge2_LriRequiredTokenId,
		&prefixmerge2_lrirequired::ModuleParserRuleName,
		&prefixmerge2_lrirequired::ModuleParserStateLabel,
		L"TestCase_Cpp/Basic",
		L"TestCase_Cpp/Ambiguous1"
		);
	TestParser<prefixmerge3_lrinested::ModuleParser, prefixmerge3_lrinested::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge3_LriNested",
		&prefixmerge3_lrinested::PrefixMerge3_LriNestedTypeName,
		&prefixmerge3_lrinested::PrefixMerge3_LriNestedFieldName,
		&prefixmerge3_lrinested::PrefixMerge3_LriNestedTokenId,
		&prefixmerge3_lrinested::ModuleParserRuleName,
		&prefixmerge3_lrinested::ModuleParserStateLabel,
		L"TestCase_Cpp/Basic",
		L"TestCase_Cpp/Ambiguous1",
		L"TestCase_Cpp/CtorExpr"
		);
	TestParser<prefixmerge4_lrimultiple::ModuleParser, prefixmerge4_lrimultiple::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge4_LriMultiple",
		&prefixmerge4_lrimultiple::PrefixMerge4_LriMultipleTypeName,
		&prefixmerge4_lrimultiple::PrefixMerge4_LriMultipleFieldName,
		&prefixmerge4_lrimultiple::PrefixMerge4_LriMultipleTokenId,
		&prefixmerge4_lrimultiple::ModuleParserRuleName,
		&prefixmerge4_lrimultiple::ModuleParserStateLabel,
		L"TestCase_Cpp/Basic",
		L"TestCase_Cpp/Ambiguous1",
		L"TestCase_Cpp/CtorExpr"
		);
	TestParser<prefixmerge5_pm::ModuleParser, prefixmerge5_pm::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge5_Pm",
		&prefixmerge5_pm::PrefixMerge5_PmTypeName,
		&prefixmerge5_pm::PrefixMerge5_PmFieldName,
		&prefixmerge5_pm::PrefixMerge5_PmTokenId,
		&prefixmerge5_pm::ModuleParserRuleName,
		&prefixmerge5_pm::ModuleParserStateLabel,
		L"TestCase_Cpp/Basic",
		L"TestCase_Cpp/Ambiguous2",
		L"TestCase_Cpp/CtorExpr"
		);
	TestParser<prefixmerge6_pm2::ModuleParser, prefixmerge6_pm2::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge6_Pm2",
		&prefixmerge6_pm2::PrefixMerge6_Pm2TypeName,
		&prefixmerge6_pm2::PrefixMerge6_Pm2FieldName,
		&prefixmerge6_pm2::PrefixMerge6_Pm2TokenId,
		&prefixmerge6_pm2::ModuleParserRuleName,
		&prefixmerge6_pm2::ModuleParserStateLabel,
		L"TestCase_Cpp/Basic",
		L"TestCase_Cpp/Ambiguous2",
		L"TestCase_Cpp/CtorExpr",
		L"TestCase_Cpp/ThrowComma"
		);
	TestParser<prefixmerge7_pmswitch::ModuleParser, prefixmerge7_pmswitch::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge7_PmSwitch",
		&prefixmerge7_pmswitch::PrefixMerge7_PmSwitchTypeName,
		&prefixmerge7_pmswitch::PrefixMerge7_PmSwitchFieldName,
		&prefixmerge7_pmswitch::PrefixMerge7_PmSwitchTokenId,
		&prefixmerge7_pmswitch::ModuleParserRuleName,
		&prefixmerge7_pmswitch::ModuleParserStateLabel,
		L"TestCase_Cpp/Basic",
		L"TestCase_Cpp/Ambiguous2",
		L"TestCase_Cpp/CtorExpr",
		L"TestCase_Cpp/ThrowComma",
		L"TestCase_Cpp/Generic"
		);
	TestParser<prefixmerge8_pmvariadic::ModuleParser, prefixmerge8_pmvariadic::json_visitor::TypeOrExprVisitor>(
		L"PrefixMerge8_PmVariadic",
		&prefixmerge8_pmvariadic::PrefixMerge8_PmVariadicTypeName,
		&prefixmerge8_pmvariadic::PrefixMerge8_PmVariadicFieldName,
		&prefixmerge8_pmvariadic::PrefixMerge8_PmVariadicTokenId,
		&prefixmerge8_pmvariadic::ModuleParserRuleName,
		&prefixmerge8_pmvariadic::ModuleParserStateLabel,
		L"TestCase_Cpp/Basic",
		L"TestCase_Cpp/Ambiguous2",
		L"TestCase_Cpp/CtorExpr",
		L"TestCase_Cpp/Generic",
		L"TestCase_Cpp/Variadic"
		);
	TestParser<prefixmerge9_pmloop::ModuleParser, prefixmerge9_pmloop::json_visitor::FileVisitor>(
		L"PrefixMerge9_PmLoop",
		&prefixmerge9_pmloop::PrefixMerge9_PmLoopTypeName,
		&prefixmerge9_pmloop::PrefixMerge9_PmLoopFieldName,
		&prefixmerge9_pmloop::PrefixMerge9_PmLoopTokenId,
		&prefixmerge9_pmloop::ModuleParserRuleName,
		&prefixmerge9_pmloop::ModuleParserStateLabel
		);

	using namespace TestParser_Generated_TestObjects;

	TEST_CASE(L"Ensure all cases have baseline")
	{
		unittest::UnitTest::PrintMessage(L"Input discovered: " + itow(inputDiscovered), unittest::UnitTest::MessageKind::Info);
		unittest::UnitTest::PrintMessage(L"Parsed successfully: " + itow(parsedSuccessfully), unittest::UnitTest::MessageKind::Info);
		unittest::UnitTest::PrintMessage(L"Compared with baseline: " + itow(comparedWithBaseline), unittest::UnitTest::MessageKind::Info);
		TEST_ASSERT(parsedSuccessfully == comparedWithBaseline);
	});
}