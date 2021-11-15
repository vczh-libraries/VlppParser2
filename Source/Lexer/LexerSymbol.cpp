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

			TokenSymbol* LexerSymbolManager::CreateToken(const WString& _name, const WString& _regex)
			{
				auto token = new TokenSymbol(_name);
				token->regex = _regex;
				if (!tokens.Add(_name, token))
				{
					global.AddError(
						ParserErrorType::DuplicatedToken,
						_name
						);
				}
				try
				{
					auto expr = regex_internal::ParseRegexExpression(wtou32(_regex));
					if (!expr->expression->HasNoExtension())
					{
					global.AddError(
						ParserErrorType::TokenRegexNotPure,
						_name
						);
					}
				}
				catch (const regex_internal::RegexException& e)
				{
					global.AddError(
						ParserErrorType::InvalidTokenRegex,
						_name,
						(e.Message() + L" : " + itow(e.position) + L" : " + _regex)
						);
				}
				return token;
			}

			TokenSymbol* LexerSymbolManager::CreateDiscardedToken(const WString& _name, const WString& _regex)
			{
				auto token = CreateToken(_name, _regex);
				token->discarded = true;
				return token;
			}
		}
	}
}