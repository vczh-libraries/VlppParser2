#include "../../../Source/Ast/AstCppGen.h"
#include "../../../Source/Lexer/LexerCppGen.h"
#include "../../../Source/Syntax/SyntaxCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;

extern WString GetExePath();

TEST_FILE
{
	TEST_CASE(L"CreateParserGenLexer")
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);
		LexerSymbolManager lexerManager(global);
		SyntaxSymbolManager syntaxManager(global);

		InitializeParserSymbolManager(global);
		CreateParserGenTypeAst(astManager);
		CreateParserGenSyntaxAst(astManager);
		CreateParserGenLexer(lexerManager);
		CreateParserGenSyntax(syntaxManager);
		TEST_ASSERT(global.Errors().Count() == 0);

		auto output = GenerateParserFileNames(global);

		Dictionary<WString, WString> files;
		WriteSyntaxFiles(syntaxManager, output, files);

		auto outputDir = FilePath(GetExePath()) / L"../../../Source/ParserGen_Generated/";
		for (auto [key, index] : indexed(files.Keys()))
		{
			//File(outputDir / key).WriteAllText(files.Values()[index], false, BomEncoder::Utf8);
		}
	});
}