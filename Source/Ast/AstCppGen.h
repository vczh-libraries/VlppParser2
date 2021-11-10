/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_AST_ASTCPPGEN
#define VCZH_PARSER2_AST_ASTCPPGEN

#include <VlppOS.h>
#include "AstSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{

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
				collections::Dictionary<AstDefFile*, Ptr<CppAstGenOutput>>	files;
			};

			extern Ptr< CppParserGenOutput>		GenerateFileNames(AstSymbolManager& manager);

			extern void			WriteAstHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void			WriteAstCppFile(AstDefFile* file, stream::StreamWriter& writer);

			extern void			WriteUtilityHeaderFile(AstDefFile* file, const WString& guardPostfix, const WString& nss, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void			WriteUtilityCppFile(AstDefFile* file, const WString& fileNamePostfix, const WString& nss, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void			WriteVisitorHeaderFile(AstDefFile* file, const WString& visitorName, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void			WriteVisitorCppFile(AstDefFile* file, const WString& visitorName, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void			WriteParserHeaderFile(AstSymbolManager& manager, const WString& guardPostfix, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void			WriteParserCppFile(AstSymbolManager& manager, const WString& fileNamePostfix, stream::StreamWriter& writer, Func<void(const WString&)> callback);

			extern void			WriteAstBuilderHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void			WriteAstBuilderCppFile(AstDefFile* file, stream::StreamWriter& writer);

			extern void			WriteEmptyVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void			WriteEmptyVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void			WriteCopyVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void			WriteCopyVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void			WriteTraverseVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void			WriteTraverseVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void			WriteJsonVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void			WriteJsonVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer);

			extern void			WriteAstAssemblerHeaderFile(AstSymbolManager& manager, stream::StreamWriter& writer);
			extern void			WriteAstAssemblerCppFile(AstSymbolManager& manager, stream::StreamWriter& writer);

			extern void			WriteAstFiles(AstDefFile* file, Ptr<CppAstGenOutput> output, collections::Dictionary<WString, WString>& files);
			extern void			WriteAstFiles(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, collections::Dictionary<WString, WString>& files);
		}
	}
}

#endif