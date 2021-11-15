/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_LEXER_LEXERCPPGEN
#define VCZH_PARSER2_LEXER_LEXERCPPGEN

#include "../ParserGen/ParserCppGen.h"
#include "LexerSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			extern void			WriteLexerHeaderFile			(LexerSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteLexerCppFile				(LexerSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteLexerFiles					(LexerSymbolManager& manager, Ptr<CppParserGenOutput> output, collections::Dictionary<WString, WString>& files);
		}
	}
}

#endif