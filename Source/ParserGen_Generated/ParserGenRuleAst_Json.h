/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:RuleAst
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_PARSERGEN_RULEAST_AST_JSON_VISITOR
#define VCZH_PARSER2_PARSERGEN_RULEAST_AST_JSON_VISITOR

#include "ParserGenRuleAst.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			namespace json_visitor
			{
				/// <summary>A JSON visitor, overriding all abstract methods with AST to JSON serialization code.</summary>
				class RuleAstVisitor
					: public vl::glr::JsonVisitorBase
					, protected virtual GlrSyntax::IVisitor
					, protected virtual GlrClause::IVisitor
				{
				protected:
					virtual void PrintFields(GlrAlternativeSyntax* node);
					virtual void PrintFields(GlrAssignment* node);
					virtual void PrintFields(GlrClause* node);
					virtual void PrintFields(GlrCreateClause* node);
					virtual void PrintFields(GlrLiteralSyntax* node);
					virtual void PrintFields(GlrLoopSyntax* node);
					virtual void PrintFields(GlrOptionalSyntax* node);
					virtual void PrintFields(GlrPartialClause* node);
					virtual void PrintFields(GlrRefSyntax* node);
					virtual void PrintFields(GlrRule* node);
					virtual void PrintFields(GlrSequenceSyntax* node);
					virtual void PrintFields(GlrSyntax* node);
					virtual void PrintFields(GlrSyntaxFile* node);
					virtual void PrintFields(GlrUseSyntax* node);
					virtual void PrintFields(Glr_ReuseClause* node);

				protected:
					void Visit(GlrRefSyntax* node) override;
					void Visit(GlrLiteralSyntax* node) override;
					void Visit(GlrLoopSyntax* node) override;
					void Visit(GlrOptionalSyntax* node) override;
					void Visit(GlrSequenceSyntax* node) override;
					void Visit(GlrAlternativeSyntax* node) override;

					void Visit(GlrCreateClause* node) override;
					void Visit(GlrPartialClause* node) override;
					void Visit(Glr_ReuseClause* node) override;

				public:
					RuleAstVisitor(vl::stream::StreamWriter& _writer);

					void Print(GlrSyntax* node);
					void Print(GlrClause* node);
					void Print(GlrUseSyntax* node);
					void Print(GlrAssignment* node);
					void Print(GlrRule* node);
					void Print(GlrSyntaxFile* node);
				};
			}
		}
	}
}
#endif