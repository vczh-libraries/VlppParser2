/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAX_SYNTAXSYMBOL
#define VCZH_PARSER2_SYNTAX_SYNTAXSYMBOL

#include <VlppRegex.h>
#include "../ParserGen/ParserSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			class SyntaxSymbolManager;

/***********************************************************************
SyntaxSymbolManager
***********************************************************************/

			class SyntaxSymbolManager : public Object
			{
			protected:
				ParserSymbolManager&		global;

			public:
				SyntaxSymbolManager(ParserSymbolManager& _global);

				ParserSymbolManager&		Global() { return global; }
			};
		}
	}
}

#endif