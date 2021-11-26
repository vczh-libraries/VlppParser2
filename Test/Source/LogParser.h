#ifndef VCZH_VLPPPARSER2_UNITTEST_LOGPARSER
#define VCZH_VLPPPARSER2_UNITTEST_LOGPARSER

#include "LogAutomaton.h"
#include "../../../Source/Syntax/SyntaxSymbol.h"

using namespace vl::glr::parsergen;

FilePath LogSyntax(
	SyntaxSymbolManager& manager,
	const WString& parserName,
	const WString& phase,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
	);

#endif