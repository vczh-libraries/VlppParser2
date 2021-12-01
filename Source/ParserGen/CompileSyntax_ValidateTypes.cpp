#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

/***********************************************************************
ValidateTypesVisitor
***********************************************************************/

			class ValidateTypesVisitor
				: public Object
				, public virtual GlrSyntax::IVisitor
				, public virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					clause = nullptr;

			public:
				ValidateTypesVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void Visit(GlrRefSyntax* node) override
				{
					if (node->field)
					{
						auto clauseType = context.clauseTypes[clause];
						auto currentType = clauseType;
						AstClassPropSymbol* prop = nullptr;
						while (currentType)
						{
							vint index = currentType->Props().Keys().IndexOf(node->field.value);
							if (index != -1)
							{
								prop = currentType->Props().Values()[index];
								break;
							}
							currentType = currentType->baseClass;
						}

						if (!prop)
						{
							context.global.AddError(
								ParserErrorType::FieldNotExistsInClause,
								ruleSymbol->Name(),
								clauseType->Name(),
								node->field.value
								);
						}
					}
				}

				void Visit(GlrLiteralSyntax* node) override
				{
				}

				void Visit(GlrUseSyntax* node) override
				{
				}

				void Visit(GlrLoopSyntax* node) override
				{
					node->syntax->Accept(this);
					if (node->delimiter)
					{
						node->delimiter->Accept(this);
					}
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				void Visit(GlrCreateClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
				}

				void Visit(GlrPartialClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
				}

				void Visit(GlrReuseClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
				}
			};

/***********************************************************************
ValidateTypes
***********************************************************************/

			void ValidateTypes(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files)
			{
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
						ValidateTypesVisitor visitor(context, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							clause->Accept(&visitor);
						}
					}
				}
			}
		}
	}
}