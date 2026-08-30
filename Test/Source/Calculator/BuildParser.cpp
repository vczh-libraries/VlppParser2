#include "../../../Source/ParserGen_Global/ParserSymbol.h"

using namespace vl;
using namespace vl::glr::parsergen;

void InitializeCalculatorParserSymbolManager(ParserSymbolManager& manager)
{
	manager.name = L"Calculator";
	Fill(manager.astIncludes, L"../../../../Source/AstBase.h");
	Fill(manager.jsonIncludes, L"../../../../Source/Json/Generated/JsonAst.h");
	Fill(manager.syntaxIncludes, L"../../../../Source/SyntaxBase.h");
	manager.cppNss.Add(L"calculator");
	manager.headerGuard = L"VCZH_PARSER2_UNITTEST_CALCULATOR";
}
