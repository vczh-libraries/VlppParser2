#include "../../../Source/Syntax/SyntaxCppGen.h"
#include "../../../Source/Ast/AstSymbol.h"
#include "../../Source/Calculator/Parser/Calculator_Assembler.h"
#include "../../Source/Calculator/Parser/Calculator_Lexer.h"
#include "../../Source/LogParser.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;
using namespace vl::glr::automaton;
using namespace calculator;

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);
extern void WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files);
extern void InitializeCalculatorParserSymbolManager(ParserSymbolManager& manager);
extern void GenerateCalculatorAst(AstSymbolManager& manager);
extern void GenerateCalculatorSyntax(AstSymbolManager& ast, SyntaxSymbolManager& manager);

TEST_FILE
{
	auto typeName = [](vint32_t type) { return WString::Unmanaged(CalculatorTypeName((CalculatorClasses)type)); };
	auto fieldName = [](vint32_t field) { return WString::Unmanaged(CalculatorFieldName((CalculatorFields)field)); };
	auto tokenName = [](vint32_t token)
	{
		auto n = CalculatorTokenId((CalculatorTokens)token);
		auto d = CalculatorTokenDisplayText((CalculatorTokens)token);
		return d ? L"\"" + WString::Unmanaged(d) + L"\"" : WString::Unmanaged(n);
	};

	TEST_CASE(L"CreateCalculatorLexer")
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);
		SyntaxSymbolManager syntaxManager(global);
		Executable executable;
		Metadata metadata;

		InitializeCalculatorParserSymbolManager(global);
		GenerateCalculatorAst(astManager);
		GenerateCalculatorSyntax(astManager, syntaxManager);
		TEST_ASSERT(global.Errors().Count() == 0);
		{
			syntaxManager.BuildCompactNFA();
			TEST_ASSERT(global.Errors().Count() == 0);
			syntaxManager.BuildCrossReferencedNFA();
			TEST_ASSERT(global.Errors().Count() == 0);
			syntaxManager.BuildAutomaton(CalculatorTokenCount, executable, metadata);

			LogAutomatonWithPath(
				GetOutputDir(L"ParserGen") / L"Automaton[ParserGen_Calculator].txt",
				executable,
				metadata,
				typeName,
				fieldName,
				tokenName
				);
		}

		auto output = GenerateParserFileNames(global);
		GenerateSyntaxFileNames(syntaxManager, output);

		Dictionary<WString, WString> files;
		WriteSyntaxFiles(syntaxManager, executable, metadata, output, files);

		auto outputDir = FilePath(GetTestParserInputPath(L"Calculator")) / L"Parser";
		WriteFilesIfChanged(outputDir, files);
	});
}