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
			class AstDefFile;
			class AstClassPropSymbol;
			class AstClassSymbol;
			class TokenSymbol;
			class SyntaxSymbolManager;

/***********************************************************************
Output
***********************************************************************/

			struct CppAstGenOutput
			{
				WString			astH;
				WString			astCpp;
				WString			builderH;
				WString			builderCpp;
				WString			emptyH;
				WString			emptyCpp;
				WString			copyH;
				WString			copyCpp;
				WString			traverseH;
				WString			traverseCpp;
				WString			jsonH;
				WString			jsonCpp;
			};

			struct CppSyntaxGenOutput
			{
				WString			syntaxH;
				WString			syntaxCpp;
			};

			struct CppParserGenOutput
			{
				WString																	assemblyH;
				WString																	assemblyCpp;
				WString																	lexerH;
				WString																	lexerCpp;
				collections::Dictionary<AstDefFile*, Ptr<CppAstGenOutput>>				astOutputs;
				collections::Dictionary<SyntaxSymbolManager*, Ptr<CppSyntaxGenOutput>>	syntaxOutputs;

				collections::Dictionary<AstClassSymbol*, vint32_t>						classIds;
				collections::Dictionary<AstClassPropSymbol*, vint32_t>					fieldIds;
				collections::Dictionary<TokenSymbol*, vint32_t>							tokenIds;
			};

/***********************************************************************
Utility
***********************************************************************/

			extern Ptr<CppParserGenOutput>		GenerateParserFileNames(ParserSymbolManager& manager);

			extern void							WriteCppStringBody(const WString& body, stream::StreamWriter& writer);
			extern void							WriteFileComment(const WString& name, stream::StreamWriter& writer);
			extern void							WriteNssName(const collections::List<WString>& cppNss, stream::StreamWriter& writer);
			extern WString						WriteNssBegin(const collections::List<WString>& cppNss, stream::StreamWriter& writer);
			extern void							WriteNssEnd(const collections::List<WString>& cppNss, stream::StreamWriter& writer);
			extern void							WriteLoadDataFunctionHeader(const WString& prefix, const WString& functionName, stream::StreamWriter& writer);
			extern void							WriteLoadDataFunctionCpp(const WString& prefix, const WString& functionName, stream::MemoryStream& rawData, bool compressData, stream::StreamWriter& writer);
		}
	}
}

#endif