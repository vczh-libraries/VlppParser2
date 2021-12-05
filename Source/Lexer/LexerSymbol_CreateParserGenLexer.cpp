#include "LexerSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
CreateParserGenAst
***********************************************************************/

			void CreateParserGenLexer(LexerSymbolManager& manager)
			{
				manager.CreateToken(L"AMBIGUOUS", L"ambiguous");
				manager.CreateToken(L"CLASS", L"class");
				manager.CreateToken(L"ENUM", L"enum");
				manager.CreateToken(L"VAR", L"var");
				manager.CreateToken(L"TOKEN", L"token");
				manager.CreateToken(L"AS", L"as");
				manager.CreateToken(L"PARTIAL", L"partial");

				manager.CreateToken(L"OPEN_ROUND", L"/(");
				manager.CreateToken(L"CLOSE_ROUND", L"/)");
				manager.CreateToken(L"OPEN_SQUARE", L"/[");
				manager.CreateToken(L"CLOSE_SQUARE", L"/]");
				manager.CreateToken(L"OPEN_CURLY", L"/{");
				manager.CreateToken(L"CLOSE_CURLY", L"/}");

				manager.CreateToken(L"COMMA", L",");
				manager.CreateToken(L"COLON", L":");
				manager.CreateToken(L"SEMICOLON", L";");

				manager.CreateToken(L"INFER", L"::=");
				manager.CreateToken(L"ALTERNATIVE", L"/|");
				manager.CreateToken(L"USE", L"!");
				manager.CreateToken(L"ASSIGN", L"=");
				manager.CreateToken(L"POSITIVE", L"/+");
				manager.CreateToken(L"NEGATIVE", L"-");

				manager.CreateToken(L"ID", L"[a-zA-Z_][a-zA-Z0-9_]*");
				manager.CreateToken(L"STRING", L"(\"[^\"]*\")+");

				manager.CreateDiscardedToken(L"SPACE", L"/s+");
			}
		}
	}
}