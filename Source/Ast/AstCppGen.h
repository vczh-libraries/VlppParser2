/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_AST_ASTCPPGEN
#define VCZH_PARSER2_AST_ASTCPPGEN

#include "../ParserGen/ParserCppGen.h"
#include "AstSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			extern void											GenerateAstFileNames(AstSymbolManager& manager, Ptr<CppParserGenOutput> parserOutput);

			extern void			WriteAstHeaderFile				(AstDefFile* file, stream::StreamWriter& writer);
			extern void			WriteAstCppFile					(AstDefFile* file, const WString& astHeaderName, stream::StreamWriter& writer);

			extern void			WriteAstUtilityHeaderFile		(AstDefFile* file, Ptr<CppAstGenOutput> output, const WString& extraNss, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void			WriteAstUtilityCppFile			(AstDefFile* file, const WString& utilityHeaderFile, const WString& extraNss, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void			WriteParserUtilityHeaderFile	(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, const WString& guardPostfix, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void			WriteParserUtilityCppFile		(AstSymbolManager& manager, const WString& utilityHeaderFile, stream::StreamWriter& writer, Func<void(const WString&)> callback);

			extern void			WriteAstBuilderHeaderFile		(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteAstBuilderCppFile			(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteEmptyVisitorHeaderFile		(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteEmptyVisitorCppFile		(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteCopyVisitorHeaderFile		(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteCopyVisitorCppFile			(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteTraverseVisitorHeaderFile	(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteTraverseVisitorCppFile		(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteJsonVisitorHeaderFile		(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteJsonVisitorCppFile			(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);

			extern void			WriteAstAssemblerHeaderFile		(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteAstAssemblerCppFile		(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer);

			extern void			WriteAstFiles					(AstDefFile* file, Ptr<CppAstGenOutput> output, collections::Dictionary<WString, WString>& files);
			extern void			WriteAstFiles					(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, collections::Dictionary<WString, WString>& files);
		}
	}
}

#endif