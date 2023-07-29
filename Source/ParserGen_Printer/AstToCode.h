#ifndef VCZH_PARSER2_PARSERGEN_ASTTOCODE
#define VCZH_PARSER2_PARSERGEN_ASTTOCODE

#include "../../Source/ParserGen_Generated/ParserGenRuleAst.h"

namespace vl::glr::parsergen
{
	extern void SyntaxAstToCode(
		Ptr<GlrSyntaxFile> file,
		stream::TextWriter& writer
	);
}

#endif