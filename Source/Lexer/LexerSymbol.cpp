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

			TokenSymbol::TokenSymbol(LexerSymbolManager* _ownerManager, const WString& _name)
				: ownerManager(_ownerManager)
				, name(_name)
			{
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
				auto token = new TokenSymbol(this, _name);
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

					Array<char32_t> buffer(_regex.Length() + 1);
					memset(&buffer[0], 0, sizeof(char32_t) * buffer.Count());
					vint writing = 0;

					List<regex_internal::Expression*> expanding;
					expanding.Add(expr->expression.Obj());
					while (expanding.Count() > 0)
					{
						auto current = expanding[expanding.Count() - 1];
						if (auto charset = dynamic_cast<regex_internal::CharSetExpression*>(current))
						{
							if (charset->ranges.Count() == 1 && !charset->reverse)
							{
								auto range = charset->ranges[0];
								if (range.begin == range.end)
								{
									expanding.RemoveAt(expanding.Count() - 1);
									buffer[writing++] = range.begin;
									continue;
								}
							}
						}
						else if (auto sequence = dynamic_cast<regex_internal::SequenceExpression*>(current))
						{
							expanding.RemoveAt(expanding.Count() - 1);
							expanding.Add(sequence->right.Obj());
							expanding.Add(sequence->left.Obj());
							continue;
						}
						goto FINISHED_CALCULATING_DISPLAY_TEXT;
					}

				FINISHED_CALCULATING_DISPLAY_TEXT:
					if (expanding.Count() == 0)
					{
						token->displayText = u32tow(U32String::Unmanaged(&buffer[0]));
						if (tokensByDisplayText.Keys().Contains(token->displayText))
						{
							global.AddError(
								ParserErrorType::DuplicatedTokenByDisplayText,
								_name
								);
						}
						else
						{
							tokensByDisplayText.Add(token->displayText, token);
						}
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