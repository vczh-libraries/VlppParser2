#include "../../../Source/Syntax/SyntaxSymbol.h"

using namespace vl;
using namespace vl::glr::parsergen;

void GenerateCalculatorSyntax(SyntaxSymbolManager& manager)
{
	auto _arg = manager.CreateRule(L"Arg");
	auto _exp0 = manager.CreateRule(L"Exp0");
	auto _exp1 = manager.CreateRule(L"Exp1");
	auto _exp2 = manager.CreateRule(L"Exp2");
	auto _exp3 = manager.CreateRule(L"Exp3");
	auto _exp4 = manager.CreateRule(L"Exp4");
	auto _exp5 = manager.CreateRule(L"Exp5");
	auto _exp = manager.CreateRule(L"Exp");
	auto _import = manager.CreateRule(L"Import");
	auto _module = manager.CreateRule(L"Module");
}