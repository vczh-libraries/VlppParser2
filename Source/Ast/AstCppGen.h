/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_AST_ASTCPPGEN
#define VCZH_PARSER2_AST_ASTCPPGEN

#include "../ParserGen_Global/ParserCppGen.h"
#include "AstSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			extern void											GenerateAstFileNames(AstSymbolManager& manager, Ptr<CppParserGenOutput> parserOutput);

			extern void			WriteAstHeaderFile				(AstDefFileGroup* group, stream::StreamWriter& writer);
			extern void			WriteAstCppFile					(AstDefFileGroup* group, const WString& astHeaderName, stream::StreamWriter& writer);

			extern void			WriteAstUtilityHeaderFile		(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, const WString& extraNss, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void			WriteAstUtilityCppFile			(AstDefFileGroup* group, const WString& utilityHeaderFile, const WString& extraNss, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void			WriteParserUtilityHeaderFile	(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, const WString& guardPostfix, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void			WriteParserUtilityCppFile		(AstSymbolManager& manager, const WString& utilityHeaderFile, stream::StreamWriter& writer, Func<void(const WString&)> callback);

			extern void			WriteAstBuilderHeaderFile		(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteAstBuilderCppFile			(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteEmptyVisitorHeaderFile		(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteEmptyVisitorCppFile		(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteCopyVisitorHeaderFile		(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteCopyVisitorCppFile			(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteTraverseVisitorHeaderFile	(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteTraverseVisitorCppFile		(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteJsonVisitorHeaderFile		(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteJsonVisitorCppFile			(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer);

			extern void			WriteAstAssemblerHeaderFile		(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer);
			extern void			WriteAstAssemblerCppFile		(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer);

			extern void			WriteAstFiles					(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, collections::Dictionary<WString, WString>& files);
			extern void			WriteAstFiles					(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, collections::Dictionary<WString, WString>& files);
		}
	}
}

#endif