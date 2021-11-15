#include "../../Source/Calculator/Parser/CalculatorAst.h"
#include "../../Source/Calculator/Parser/Calculator_Lexer.h"
#include "../../../Source/Ast/AstCppGen.h"
#include "../../../Source/Lexer/LexerCppGen.h"
#include "../../../Source/Syntax/SyntaxCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::regex;
using namespace calculator;
using namespace vl::glr::parsergen;

extern WString GetExePath();

TEST_FILE
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
	TEST_CASE_ASSERT(global.Errors().Count() == 0);

	MemoryStream lexerData;
	CalculatorLexerData(lexerData);
	lexerData.SeekFromBegin(0);
	RegexLexer lexer(lexerData);

	TEST_CASE(L"Launch Calculator Syntax")
	{
		WString input = LR"(
export 1
)";
		List<RegexToken> tokens;
		lexer.Parse(input).ReadToEnd(tokens, CalculatorTokenDeleter);
	});

#undef LEXER
}