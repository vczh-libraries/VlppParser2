#include "ParserSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{

/***********************************************************************
Utility
***********************************************************************/

			void InitializeParserSymbolManager(ParserSymbolManager& manager)
			{
				manager.name = L"ParserGen";
				Fill(manager.cppNss, L"vl", L"glr", L"parsergen");
				manager.headerGuard = L"VCZH_PARSER2_PARSERGEN";
			}
		}
	}
}