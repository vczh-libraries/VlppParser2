#ifndef VCZH_VLPPPARSER2_UNITTEST_LOGPARSER
#define VCZH_VLPPPARSER2_UNITTEST_LOGPARSER

#include "../../../Source/Syntax/SyntaxSymbol.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;

void LogSyntax(SyntaxSymbolManager& manager, const WString& parserName, const WString& phase);

#endif