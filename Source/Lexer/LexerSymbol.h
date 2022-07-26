/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_LEXER_LEXERSYMBOL
#define VCZH_PARSER2_LEXER_LEXERSYMBOL

#include <VlppRegex.h>
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
				LexerSymbolManager*			ownerManager;
				WString						name;

				TokenSymbol(LexerSymbolManager* _ownerManager, const WString& _name);
			public:
				WString						regex;
				WString						displayText;
				bool						discarded = false;
				
				LexerSymbolManager*			Owner() { return ownerManager; }
				const WString&				Name() { return name; }
			};

			class LexerSymbolManager : public Object
			{
				using TokenMap = collections::Dictionary<WString, TokenSymbol*>;
			protected:
				MappedOwning<TokenSymbol>	tokens;
				TokenMap					tokensByDisplayText;
				ParserSymbolManager&		global;

			public:
				LexerSymbolManager(ParserSymbolManager& _global);

				TokenSymbol*				CreateToken(const WString& _name, const WString& _regex, ParsingTextRange codeRange = {});
				TokenSymbol*				CreateDiscardedToken(const WString& _name, const WString& _regex, ParsingTextRange codeRange = {});

				const ParserSymbolManager&	Global() const { return global; }
				const auto&					Tokens() const { return tokens.map; }
				const auto&					TokensByDisplayText() const { return tokensByDisplayText; }
				const auto&					TokenOrder() const { return tokens.order; }

				template<typename ...TArgs>
				void AddError(ParserErrorType type, ParsingTextRange codeRange, TArgs&&... args) const
				{
					global.AddError(type, { ParserDefFileType::Lexer,WString::Empty,codeRange }, std::forward<TArgs&&>(args)...);
				}
			};

			extern void						CreateParserGenLexer(LexerSymbolManager& manager);
		}
	}
}

#endif