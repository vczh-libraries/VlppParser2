#ifndef VCZH_VLPPPARSER2_UNITTEST_SYNTAXASTTOCODE
#define VCZH_VLPPPARSER2_UNITTEST_SYNTAXASTTOCODE

#include "../../Source/ParserGen_Generated/ParserGenRuleAst.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::glr::parsergen;

extern void SyntaxAstToCode(
	Ptr<GlrSyntaxFile> file,
	TextWriter& writer
	);

#endif