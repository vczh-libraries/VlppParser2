#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
CreateParserGenTypeSyntax
***********************************************************************/

			void CreateParserGenTypeSyntax(SyntaxSymbolManager& manager)
			{
				manager.name = L"TypeParser";
			}
		}
	}
}