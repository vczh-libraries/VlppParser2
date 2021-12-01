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

	TEST_CASE(L"CreateParserGenLexer")
	{
		ParserSymbolManager global;

		SyntaxSymbolManager typeSyntaxManager(global);
		Executable typeExecutable;
		Metadata typeMetadata;

		SyntaxSymbolManager ruleSyntaxManager(global);
		Executable ruleExecutable;
		Metadata ruleMetadata;

		InitializeParserSymbolManager(global);
		CreateParserGenTypeSyntax(typeSyntaxManager);
		CreateParserGenRuleSyntax(ruleSyntaxManager);
		TEST_ASSERT(global.Errors().Count() == 0);
		{
			LogSyntaxWithPath(
				typeSyntaxManager,
				GetOutputDir(L"ParserGen") / L"NFA[1][ParserGen_TypeParser].txt",
				typeName,
				fieldName,
				tokenName);

			typeSyntaxManager.BuildCompactNFA();
			TEST_ASSERT(global.Errors().Count() == 0);

			LogSyntaxWithPath(
				typeSyntaxManager,
				GetOutputDir(L"ParserGen") / L"NFA[2][ParserGen_TypeParser].txt",
				typeName,
				fieldName,
				tokenName);

			typeSyntaxManager.BuildCrossReferencedNFA();
			TEST_ASSERT(global.Errors().Count() == 0);

			LogSyntaxWithPath(
				typeSyntaxManager,
				GetOutputDir(L"ParserGen") / L"NFA[3][ParserGen_TypeParser].txt",
				typeName,
				fieldName,
				tokenName);

			typeSyntaxManager.BuildAutomaton(ParserGenTokenCount, typeExecutable, typeMetadata);

			LogAutomatonWithPath(
				GetOutputDir(L"ParserGen") / L"Automaton[ParserGen_TypeParser].txt",
				typeExecutable,
				typeMetadata,
				typeName,
				fieldName,
				tokenName);
		}
		{
			LogSyntaxWithPath(
				ruleSyntaxManager,
				GetOutputDir(L"ParserGen") / L"NFA[1][ParserGen_RuleParser].txt",
				typeName,
				fieldName,
				tokenName);

			ruleSyntaxManager.BuildCompactNFA();
			TEST_ASSERT(global.Errors().Count() == 0);

			LogSyntaxWithPath(
				ruleSyntaxManager,
				GetOutputDir(L"ParserGen") / L"NFA[2][ParserGen_RuleParser].txt",
				typeName,
				fieldName,
				tokenName);

			ruleSyntaxManager.BuildCrossReferencedNFA();
			TEST_ASSERT(global.Errors().Count() == 0);

			LogSyntaxWithPath(
				ruleSyntaxManager,
				GetOutputDir(L"ParserGen") / L"NFA[3][ParserGen_RuleParser].txt",
				typeName,
				fieldName,
				tokenName);

			ruleSyntaxManager.BuildAutomaton(ParserGenTokenCount, ruleExecutable, ruleMetadata);

			LogAutomatonWithPath(
				GetOutputDir(L"ParserGen") / L"Automaton[ParserGen_RuleParser].txt",
				ruleExecutable,
				ruleMetadata,
				typeName,
				fieldName,
				tokenName);
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