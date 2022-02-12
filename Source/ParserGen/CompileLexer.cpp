#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;
			using namespace regex;

/***********************************************************************
UnescapeLiteral
***********************************************************************/

			WString UnescapeLiteral(const WString& literal, wchar_t quot)
			{
				Array<wchar_t> buffer(literal.Length());
				wchar_t* writing = &buffer[0];

				for (vint i = 1; i < literal.Length() - 1; i++)
				{
					wchar_t c = literal[i];
					*writing++ = c;
					if (c == quot)
					{
						i++;
					}
				}
				*writing = 0;

				return &buffer[0];
			}

/***********************************************************************
CompileLexer
***********************************************************************/

			void CompileLexer(LexerSymbolManager& lexerManager, const WString& input)
			{
				Regex regexToken(L"^(/s*|(<discard>discard/s)?/s*(<name>/$?[a-zA-Z_]/w*)/s*:(<regex>/.+))$");
				Regex regexFragment(L"/{(<fragment>/$[a-zA-Z_]/w*)/}");
				vint _discard = regexToken.CaptureNames().IndexOf(L"discard");
				vint _name = regexToken.CaptureNames().IndexOf(L"name");
				vint _regex = regexToken.CaptureNames().IndexOf(L"regex");
				vint _fragment = regexFragment.CaptureNames().IndexOf(L"fragment");

				Dictionary<WString, WString> fragments;

				StringReader reader(input);
				vint lineIndex = 0;
				while (!reader.IsEnd())
				{
					auto line = reader.ReadLine();
					ParsingTextRange codeRange = { {lineIndex,0}, {lineIndex,0} };
					if (auto match = regexToken.MatchHead(line))
					{
						if (match->Groups().Keys().Contains(_name))
						{
							auto tokenName = match->Groups()[_name][0].Value();
							auto tokenRegex = match->Groups()[_regex][0].Value();
							auto tokenDiscard = match->Groups().Keys().Contains(_discard);

							if (tokenName[0] == L'$')
							{
								if (tokenDiscard)
								{
									lexerManager.AddError(
										ParserErrorType::InvalidTokenDefinition,
										codeRange,
										line
										);
								}
								else if (fragments.Keys().Contains(tokenName))
								{
									lexerManager.AddError(
										ParserErrorType::DuplicatedTokenFragment,
										codeRange,
										tokenName
										);
								}
								else
								{
									fragments.Add(tokenName, tokenRegex);
								}
							}
							else
							{
								WString resolvedRegex;
								List<Ptr<RegexMatch>> matches;
								regexFragment.Cut(tokenRegex, false, matches);
								for (auto&& fragment : matches)
								{
									if (fragment->Success())
									{
										auto fragmentName = fragment->Groups()[_fragment][0].Value();
										vint index = fragments.Keys().IndexOf(fragmentName);
										if (index == -1)
										{
											lexerManager.AddError(
												ParserErrorType::TokenFragmentNotExists,
												codeRange,
												fragmentName
												);
										}
										else
										{
											resolvedRegex += fragments.Values()[index];
										}
									}
									else
									{
										resolvedRegex += fragment->Result().Value();
									}
								}

								if (tokenDiscard)
								{
									lexerManager.CreateDiscardedToken(tokenName, resolvedRegex, codeRange);
								}
								else
								{
									lexerManager.CreateToken(tokenName, resolvedRegex, codeRange);
								}
							}
						}
					}
					else if (line.Length() < 2 || line.Left(2) != L"//")
					{
						lexerManager.AddError(
							ParserErrorType::InvalidTokenDefinition,
							codeRange,
							line
							);
					}
					lineIndex++;
				}
			}
		}
	}
}