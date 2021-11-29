#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace stream;
			using namespace regex;

/***********************************************************************
CompileAst
***********************************************************************/

			void CompileLexer(LexerSymbolManager& lexerManager, const WString& input)
			{
				Regex regexToken(L"^(/s*|(<discard>discard/s+)?(<name>[a-zA-Z_]/w*):(<regex>/.+))$");
				vint _discard = regexToken.CaptureNames().IndexOf(L"discard");
				vint _name = regexToken.CaptureNames().IndexOf(L"name");
				vint _regex = regexToken.CaptureNames().IndexOf(L"regex");

				StringReader reader(input);
				while (!reader.IsEnd())
				{
					auto line = reader.ReadLine();
					if (auto match = regexToken.MatchHead(line))
					{
						if (match->Groups().Keys().Contains(_name))
						{
							auto tokenName = match->Groups()[_name][0].Value();
							auto tokenRegex = match->Groups()[_regex][0].Value();
							if (match->Groups().Keys().Contains(_discard))
							{
								lexerManager.CreateDiscardedToken(tokenName, tokenRegex);
							}
							else
							{
								lexerManager.CreateToken(tokenName, tokenRegex);
							}
						}
					}
					else
					{
						lexerManager.Global().AddError(ParserErrorType::InvalidTokenDefinition, line);
					}
				}
			}
		}
	}
}