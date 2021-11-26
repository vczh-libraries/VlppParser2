#include "../../../Source/ParserGen/ParserSymbol.h"

using namespace vl;
using namespace vl::glr::parsergen;

void InitializeCalculatorParserSymbolManager(ParserSymbolManager& manager)
{
	manager.name = L"Calculator";
	Fill(manager.includes, L"../../../../Source/AstBase.h");
	manager.cppNss.Add(L"calculator");
	manager.headerGuard = L"VCZH_PARSER2_UNITTEST_CALCULATOR";
}