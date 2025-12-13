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
#include "../../Source/Feature_BO/Generated/Feature_BOFeatureAst_Json.h"
#include "../../Source/Feature_BO/Generated/Feature_BOModuleParser.h"
#include "../../Source/Feature_CL/Generated/Feature_CLFeatureAst_Json.h"
#include "../../Source/Feature_CL/Generated/Feature_CLModuleParser.h"
#include "../../Source/Feature_ERO/Generated/Feature_EROFeatureAst_Json.h"
#include "../../Source/Feature_ERO/Generated/Feature_EROModuleParser.h"
#include "../../Source/Feature_FA/Generated/Feature_FAFeatureAst_Json.h"
#include "../../Source/Feature_FA/Generated/Feature_FAModuleParser.h"
#include "../../Source/Feature_NOMinus/Generated/Feature_NOMinusFeatureAst_Json.h"
#include "../../Source/Feature_NOMinus/Generated/Feature_NOMinusModuleParser.h"
#include "../../Source/Feature_NOPlus/Generated/Feature_NOPlusFeatureAst_Json.h"
#include "../../Source/Feature_NOPlus/Generated/Feature_NOPlusModuleParser.h"
#include "../../Source/Feature_NO/Generated/Feature_NOFeatureAst_Json.h"
#include "../../Source/Feature_NO/Generated/Feature_NOModuleParser.h"
#include "../../Source/Feature_OptMinus/Generated/Feature_OptMinusFeatureAst_Json.h"
#include "../../Source/Feature_OptMinus/Generated/Feature_OptMinusModuleParser.h"
#include "../../Source/Feature_OptPlus/Generated/Feature_OptPlusFeatureAst_Json.h"
#include "../../Source/Feature_OptPlus/Generated/Feature_OptPlusModuleParser.h"
#include "../../Source/Feature_Opt/Generated/Feature_OptFeatureAst_Json.h"
#include "../../Source/Feature_Opt/Generated/Feature_OptModuleParser.h"
#include "../../Source/Feature_Pba/Generated/Feature_PbaFeatureAst_Json.h"
#include "../../Source/Feature_Pba/Generated/Feature_PbaModuleParser.h"
#include "../../Source/Feature_Pwa/Generated/Feature_PwaFeatureAst_Json.h"
#include "../../Source/Feature_Pwa/Generated/Feature_PwaModuleParser.h"
#include "../../Source/Feature_Pwl/Generated/Feature_PwlFeatureAst_Json.h"
#include "../../Source/Feature_Pwl/Generated/Feature_PwlModuleParser.h"
#include "../../Source/BinaryOp/Generated/BinaryOpExprAst_Json.h"
#include "../../Source/BinaryOp/Generated/BinaryOpModuleParser.h"
#include "../../Source/PrefixMerge5_Pm/Generated/PrefixMerge5_PmTypeOrExpr_Json.h"
#include "../../Source/PrefixMerge5_Pm/Generated/PrefixMerge5_PmModuleParser.h"
//#include "../../Source/PrefixMerge6_Pm2/Generated/PrefixMerge6_Pm2TypeOrExpr_Json.h"
//#include "../../Source/PrefixMerge6_Pm2/Generated/PrefixMerge6_Pm2ModuleParser.h"
//#include "../../Source/PrefixMerge7_PmSwitch/Generated/PrefixMerge7_PmSwitchTypeOrExpr_Json.h"
//#include "../../Source/PrefixMerge7_PmSwitch/Generated/PrefixMerge7_PmSwitchModuleParser.h"
//#include "../../Source/PrefixMerge8_PmVariadic/Generated/PrefixMerge8_PmVariadicTypeOrExpr_Json.h"
//#include "../../Source/PrefixMerge8_PmVariadic/Generated/PrefixMerge8_PmVariadicModuleParser.h"
//#include "../../Source/PrefixMerge9_PmLoop/Generated/PrefixMerge9_PmLoopFile_Json.h"
//#include "../../Source/PrefixMerge9_PmLoop/Generated/PrefixMerge9_PmLoopModuleParser.h"
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

				TraceProcessingArgs tpArgs(args.tokens, args.executable, args.executor, false, TraceProcessingPhase::EndOfInput);
				parser.OnTraceProcessing(tpArgs);
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

#define ENABLE_PARSER(UPPERCASE, LOWERCASE, VISITOR, ...)													\
	TestParser<LOWERCASE::ModuleParser, LOWERCASE::json_visitor::VISITOR##Visitor>(						\
		L#UPPERCASE,																						\
		&LOWERCASE::UPPERCASE##TypeName,																	\
		&LOWERCASE::UPPERCASE##FieldName,																	\
		&LOWERCASE::UPPERCASE##TokenId,																		\
		&LOWERCASE::ModuleParserRuleName,																	\
		&LOWERCASE::ModuleParserStateLabel,																	\
		__VA_ARGS__																							\
		)																									\

#define ENABLE_FEATURE(UPPERCASE, LOWERCASE)																\
	TestParser<feature_##LOWERCASE::ModuleParser, feature_##LOWERCASE::json_visitor::FeatureAstVisitor>(	\
		L"Feature_" #UPPERCASE,																				\
		&feature_##LOWERCASE::Feature_##UPPERCASE##TypeName,												\
		&feature_##LOWERCASE::Feature_##UPPERCASE##FieldName,												\
		&feature_##LOWERCASE::Feature_##UPPERCASE##TokenId,													\
		&feature_##LOWERCASE::ModuleParserRuleName,															\
		&feature_##LOWERCASE::ModuleParserStateLabel														\
		)																									\

	ENABLE_PARSER(Calculator,				calculator,				ExprAst);
	ENABLE_PARSER(IfElseAmbiguity,			ifelseambiguity,		StatAst,	L"TestCase_IfElseAmbiguity");
	ENABLE_PARSER(IfElseAmbiguity2,			ifelseambiguity2,		StatAst,	L"TestCase_IfElseAmbiguity");
	ENABLE_PARSER(IfElseAmbiguityOnStat,	ifelseambiguityonstat,	StatAst,	L"TestCase_IfElseAmbiguityOnStat");
	ENABLE_PARSER(IfElseAmbiguityOnStat2,	ifelseambiguityonstat2,	StatAst,	L"TestCase_IfElseAmbiguityOnStat");
	ENABLE_PARSER(IfElsePriority,			ifelsepriority,			StatAst,	L"TestCase_IfElse");
	ENABLE_PARSER(IfElseManual,				ifelsemanual,			StatAst,	L"TestCase_IfElse");
	ENABLE_PARSER(IfElseSwitch,				ifelseswitch,			StatAst,	L"TestCase_IfElse");
	ENABLE_PARSER(GenericAmbiguity,			genericambiguity,		ExprAst);
	ENABLE_PARSER(BinaryOp,					binaryop,				ExprAst);

	ENABLE_FEATURE(BO, bo);
	ENABLE_FEATURE(CL, cl);
	ENABLE_FEATURE(ERO, ero);
	ENABLE_FEATURE(FA, fa);
	ENABLE_FEATURE(NOMinus, nominus);
	ENABLE_FEATURE(NOPlus, noplus);
	ENABLE_FEATURE(NO, no);
	ENABLE_FEATURE(OptMinus, optminus);
	ENABLE_FEATURE(OptPlus, optplus);
	ENABLE_FEATURE(Opt, opt);
	ENABLE_FEATURE(Pba, pba);
	ENABLE_FEATURE(Pwa, pwa);
	ENABLE_FEATURE(Pwl, pwl);

	//ENABLE_PARSER(PrefixMerge5_Pm,			prefixmerge5_pm,			TypeOrExpr,		L"TestCase_Cpp/Basic", L"TestCase_Cpp/Ambiguous2", L"TestCase_Cpp/CtorExpr");
	//ENABLE_PARSER(PrefixMerge6_Pm2,			prefixmerge6_pm2,			TypeOrExpr,		L"TestCase_Cpp/Basic", L"TestCase_Cpp/Ambiguous2", L"TestCase_Cpp/CtorExpr", L"TestCase_Cpp/ThrowComma");
	//ENABLE_PARSER(PrefixMerge7_PmSwitch,		prefixmerge7_pmswitch,		TypeOrExpr,		L"TestCase_Cpp/Basic", L"TestCase_Cpp/Ambiguous2", L"TestCase_Cpp/CtorExpr", L"TestCase_Cpp/ThrowComma", L"TestCase_Cpp/Generic");
	//ENABLE_PARSER(PrefixMerge8_PmVariadic,	prefixmerge8_pmvariadic,	TypeOrExpr,		L"TestCase_Cpp/Basic", L"TestCase_Cpp/Ambiguous2", L"TestCase_Cpp/CtorExpr", L"TestCase_Cpp/Variadic");
	//ENABLE_PARSER(PrefixMerge9_PmLoop,		prefixmerge9_pmloop,		File);

#undef ENABLE_FEATURE
#undef ENABLE_PARSER

	using namespace TestParser_Generated_TestObjects;

	TEST_CASE(L"Ensure all cases have baseline")
	{
		unittest::UnitTest::PrintMessage(L"Input discovered: " + itow(inputDiscovered), unittest::UnitTest::MessageKind::Info);
		unittest::UnitTest::PrintMessage(L"Parsed successfully: " + itow(parsedSuccessfully), unittest::UnitTest::MessageKind::Info);
		unittest::UnitTest::PrintMessage(L"Compared with baseline: " + itow(comparedWithBaseline), unittest::UnitTest::MessageKind::Info);
		TEST_ASSERT(parsedSuccessfully == comparedWithBaseline);
	});
}