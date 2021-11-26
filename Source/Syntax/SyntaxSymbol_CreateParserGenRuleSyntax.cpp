#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
CreateParserGenRuleSyntax
***********************************************************************/

			void CreateParserGenRuleSyntax(SyntaxSymbolManager& manager)
			{
				manager.name = L"RuleParser";
			}
		}
	}
}