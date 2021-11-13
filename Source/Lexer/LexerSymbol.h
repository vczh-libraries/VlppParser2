/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_LEXER_LEXERSYMBOL
#define VCZH_PARSER2_LEXER_LEXERSYMBOL

#include "../ParserGen/ParserSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			class LexerSymbolManager;

/***********************************************************************
LexerSymbolManager
***********************************************************************/

			class TokenSymbol : public Object
			{
				friend class LexerSymbolManager;
			protected:
				WString						name;

				TokenSymbol(const WString& _name);
			public:
				WString						regex;
				bool						discarded = false;

				const WString&				Name();
			};

			class LexerSymbolManager : public Object
			{
			protected:
				MappedOwning<TokenSymbol>	tokens;
				ParserSymbolManager&		global;

			public:
				LexerSymbolManager(ParserSymbolManager& _global);

				TokenSymbol*				CreateToken(const WString& name);

				ParserSymbolManager&		Global() { return global; }
				const auto&					Tokens() { return tokens.map; }
				const auto&					TokenOrder() { return tokens.order; }
			};
		}
	}
}

#endif