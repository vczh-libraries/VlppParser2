#ifndef VCZH_PARSER2_PARSERGEN_ASTTOCODE
#define VCZH_PARSER2_PARSERGEN_ASTTOCODE

#include "../../Source/ParserGen_Generated/ParserGenTypeAst.h"
#include "../../Source/ParserGen_Generated/ParserGenRuleAst.h"
#include "../../Source/Ast/AstSymbol.h"

namespace vl::glr::parsergen
{
	extern Ptr<GlrAstFile> TypeSymbolToAst(
		const AstSymbolManager& manager,
		bool createGeneratedTypes
	);

	extern void TypeAstToCode(
		Ptr<GlrAstFile> file,
		stream::TextWriter& writer
	);

	extern void SyntaxAstToCode(
		Ptr<GlrSyntaxFile> file,
		stream::TextWriter& writer
	);
}

#endif