#include "LexerSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
TokenSymbol
***********************************************************************/

			TokenSymbol::TokenSymbol(const WString& _name)
				:name(_name)
			{
			}

			const WString& TokenSymbol::Name()
			{
				return name;
			}

/***********************************************************************
LexerSymbolManager
***********************************************************************/

			LexerSymbolManager::LexerSymbolManager(ParserSymbolManager& _global)
				: global(_global)
			{
			}

			TokenSymbol* LexerSymbolManager::CreateToken(const WString& name)
			{
				auto token = new TokenSymbol(name);
				if (!tokens.Add(name, token))
				{
					global.AddError(
						ParserErrorType::DuplicatedToken,
						name
						);
				}
				return token;
			}
		}
	}
}