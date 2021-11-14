#include "../../../Source/Lexer/LexerSymbol.h"

using namespace vl;
using namespace vl::glr::parsergen;

void GenerateCalculatorLexer(LexerSymbolManager& manager)
{
	manager.CreateToken(L"ADD")->regex = L"/+";
	manager.CreateToken(L"SUB")->regex = L"-";
	manager.CreateToken(L"MUL")->regex = L"/*";
	manager.CreateToken(L"DIV")->regex = L"//";
	manager.CreateToken(L"GT")->regex = L"/>";
	manager.CreateToken(L"GE")->regex = L"/>=";
	manager.CreateToken(L"LT")->regex = L"/<";
	manager.CreateToken(L"LE")->regex = L"/<=";
	manager.CreateToken(L"EQ")->regex = L"==";
	manager.CreateToken(L"NE")->regex = L"!=";
	manager.CreateToken(L"OPEN_BRACE")->regex = L"/(";
	manager.CreateToken(L"CLOSE_BRACE")->regex = L"/)";
	manager.CreateToken(L"COMMA")->regex = L",";
	manager.CreateToken(L"INFER")->regex = L"-/>";
	manager.CreateToken(L"ASSIGN")->regex = L"/<-";

	manager.CreateToken(L"TRUE")->regex = L"true";
	manager.CreateToken(L"FALSE")->regex = L"false";
	manager.CreateToken(L"LET")->regex = L"let";
	manager.CreateToken(L"IN")->regex = L"let";
	manager.CreateToken(L"IMPORT")->regex = L"import";
	manager.CreateToken(L"EXPORT")->regex = L"export";

	manager.CreateToken(L"NUM")->regex = L"/d+(/d+.)?";
	manager.CreateToken(L"ID")->regex = L"[a-zA-Z_][a-zA-Z0-9_]*";

	{
		auto _token = manager.CreateToken(L"SPACE");
		_token->regex = L"/s+";
		_token->discarded = true;
	}
}