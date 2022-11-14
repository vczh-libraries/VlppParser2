#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

			namespace rewritesyntax_switch
			{
			}

/***********************************************************************
RewriteSyntax
***********************************************************************/

			Ptr<GlrSyntaxFile> RewriteSyntax_Switch(VisitorContext& context, SyntaxSymbolManager& syntaxManager, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				using namespace rewritesyntax_switch;

				// merge files to single syntax file
				auto rewritten = MakePtr<GlrSyntaxFile>();
				for (auto file : files)
				{
					CopyFrom(rewritten->switches, file->switches, true);
					CopyFrom(rewritten->rules, file->rules, true);
				}

				return rewritten;
			}
		}
	}
}