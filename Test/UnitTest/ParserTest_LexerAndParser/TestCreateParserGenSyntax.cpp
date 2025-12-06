#include "../../../Source/Ast/AstSymbol.h"
#include "../../../Source/Syntax/SyntaxCppGen.h"
#include "../../../Source/ParserGen_Generated/ParserGen_Assembler.h"
#include "../../../Source/ParserGen_Generated/ParserGen_Lexer.h"
#include "../../Source/LogParser.h"

using namespace vl::glr::parsergen;

extern WString GetParserGenGeneratedOutputPath();
extern FilePath GetOutputDir(const WString& parserName);
extern void WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files);

TEST_FILE
{
	auto typeName = [](vint32_t type) { return WString::Unmanaged(ParserGenTypeName((ParserGenClasses)type)); };
	auto fieldName = [](vint32_t field) { return WString::Unmanaged(ParserGenFieldName((ParserGenFields)field)); };
	auto tokenName = [](vint32_t token)
	{
		auto n = ParserGenTokenId((ParserGenTokens)token);
		auto d = ParserGenTokenDisplayText((ParserGenTokens)token);
		return d ? L"\"" + WString::Unmanaged(d) + L"\"" : WString::Unmanaged(n);
	};

	TEST_CASE(L"CreateParserGenSyntax")
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);

		SyntaxSymbolManager typeSyntaxManager(global);
		Executable typeExecutable;
		Metadata typeMetadata;

		SyntaxSymbolManager ruleSyntaxManager(global);
		Executable ruleExecutable;
		Metadata ruleMetadata;

		InitializeParserSymbolManager(global);
		CreateParserGenTypeAst(astManager);
		CreateParserGenRuleAst(astManager);
		CreateParserGenTypeSyntax(astManager, typeSyntaxManager);
		CreateParserGenRuleSyntax(astManager, ruleSyntaxManager);
		TEST_ASSERT(global.Errors().Count() == 0);

		auto logSyntax = [&](const WString& parserName, vint phase, SyntaxSymbolManager& syntaxManager)
		{
			LogSyntaxWithPath(
				syntaxManager,
				GetOutputDir(L"ParserGen") / (L"NFA[" + parserName + L"][" + itow(phase) + L"].txt"),
				typeName,
				fieldName,
				tokenName
				);
		};

		auto logAutomaton = [&](const WString& parserName, Executable& executable, Metadata& metadata)
		{
			LogAutomatonWithPath(
				GetOutputDir(L"ParserGen") / (L"Automaton[" + parserName + L"].txt"),
				executable,
				metadata,
				typeName,
				fieldName,
				tokenName
				);
		};

		{
			logSyntax(L"ParserGen_TypeParser", 1, typeSyntaxManager);

			typeSyntaxManager.BuildCompactNFA();
			TEST_ASSERT(global.Errors().Count() == 0);
			logSyntax(L"ParserGen_TypeParser", 2, typeSyntaxManager);

			typeSyntaxManager.BuildCrossReferencedNFA();
			TEST_ASSERT(global.Errors().Count() == 0);
			logSyntax(L"ParserGen_TypeParser", 3, typeSyntaxManager);

			typeSyntaxManager.BuildAutomaton(ParserGenTokenCount, typeExecutable, typeMetadata);
			logAutomaton(L"ParserGen_TypeParser", typeExecutable, typeMetadata);
		}
		{
			logSyntax(L"ParserGen_RuleParser", 1, ruleSyntaxManager);

			ruleSyntaxManager.BuildCompactNFA();
			TEST_ASSERT(global.Errors().Count() == 0);
			logSyntax(L"ParserGen_RuleParser", 2, ruleSyntaxManager);

			ruleSyntaxManager.BuildCrossReferencedNFA();
			TEST_ASSERT(global.Errors().Count() == 0);
			logSyntax(L"ParserGen_RuleParser", 3, ruleSyntaxManager);

			ruleSyntaxManager.BuildAutomaton(ParserGenTokenCount, ruleExecutable, ruleMetadata);
			logAutomaton(L"ParserGen_RuleParser", ruleExecutable, ruleMetadata);
		}
		auto output = GenerateParserFileNames(global);
		GenerateSyntaxFileNames(typeSyntaxManager, output);
		GenerateSyntaxFileNames(ruleSyntaxManager, output);

		Dictionary<WString, WString> files;
		WriteSyntaxFiles(typeSyntaxManager, typeExecutable, typeMetadata, output, files);
		WriteSyntaxFiles(ruleSyntaxManager, ruleExecutable, ruleMetadata, output, files);

		auto outputDir = FilePath(GetParserGenGeneratedOutputPath());
		WriteFilesIfChanged(outputDir, files);
	});
}