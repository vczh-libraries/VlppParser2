/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_PARSERGEN_PARSERCPPGEN
#define VCZH_PARSER2_PARSERGEN_PARSERCPPGEN

#include <VlppOS.h>
#include "ParserSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			class AstClassPropSymbol;
			class AstClassSymbol;
			class AstDefFile;

/***********************************************************************
Output
***********************************************************************/

			struct CppAstGenOutput
			{
				WString														astH;
				WString														astCpp;
				WString														builderH;
				WString														builderCpp;
				WString														emptyH;
				WString														emptyCpp;
				WString														copyH;
				WString														copyCpp;
				WString														traverseH;
				WString														traverseCpp;
				WString														jsonH;
				WString														jsonCpp;
			};

			struct CppParserGenOutput
			{
				WString														assemblyH;
				WString														assemblyCpp;
				WString														lexerH;
				WString														lexerCpp;
				collections::Dictionary<AstDefFile*, Ptr<CppAstGenOutput>>	files;

				collections::Dictionary<AstClassSymbol*, vint>				classIds;
				collections::Dictionary<AstClassPropSymbol*, vint>			fieldIds;
			};

/***********************************************************************
Utility
***********************************************************************/

			extern Ptr<CppParserGenOutput>		GenerateParserFileNames(ParserSymbolManager& manager);

			extern void							WriteFileComment(const WString& name, stream::StreamWriter& writer);
			extern WString						WriteNssBegin(collections::List<WString>& cppNss, stream::StreamWriter& writer);
			extern void							WriteNssEnd(collections::List<WString>& cppNss, stream::StreamWriter& writer);
		}
	}
}

#endif