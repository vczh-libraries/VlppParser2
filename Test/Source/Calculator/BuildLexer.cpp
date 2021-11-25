#include "../../../Source/Lexer/LexerSymbol.h"

using namespace vl;
using namespace vl::glr::parsergen;

void GenerateCalculatorLexer(LexerSymbolManager& manager)
{
	manager.CreateToken(L"ADD", L"/+");
	manager.CreateToken(L"SUB", L"-");
	manager.CreateToken(L"MUL", L"/*");
	manager.CreateToken(L"DIV", L"//");

	manager.CreateToken(L"GT", L"/>");
	manager.CreateToken(L"GE", L"/>=");
	manager.CreateToken(L"LT", L"/<");
	manager.CreateToken(L"LE", L"/<=");
	manager.CreateToken(L"EQ", L"==");
	manager.CreateToken(L"NE", L"!=");

	manager.CreateToken(L"OPEN_BRACE", L"/(");
	manager.CreateToken(L"CLOSE_BRACE", L"/)");
	manager.CreateToken(L"COMMA", L",");
	manager.CreateToken(L"INFER", L"-/>");
	manager.CreateToken(L"ASSIGN", L"/<-");

	manager.CreateToken(L"TRUE", L"true");
	manager.CreateToken(L"FALSE", L"false");
	manager.CreateToken(L"LET", L"let");
	manager.CreateToken(L"IN", L"in");
	manager.CreateToken(L"IMPORT", L"import");
	manager.CreateToken(L"EXPORT", L"export");

	manager.CreateToken(L"NUM", L"/d+(./d+)?");
	manager.CreateToken(L"ID", L"[a-zA-Z_][a-zA-Z0-9_]*");

	manager.CreateDiscardedToken(L"SPACE", L"/s+");
}