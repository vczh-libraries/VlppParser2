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
ValidateStructureVisitor
***********************************************************************/

			class ValidateStructureVisitor
				: public Object
				, public virtual GlrSyntax::IVisitor
				, public virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					clause = nullptr;

				vint						optionalCounter = 0;
				vint						loopCounter = 0;

				vint						syntaxMinLength = 0;
				vint						syntaxUseRuleCount = 0;

			public:
				ValidateStructureVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void Visit(GlrRefSyntax* node) override
				{
					syntaxMinLength = 1;
				}

				void Visit(GlrLiteralSyntax* node) override
				{
					syntaxMinLength = 1;
				}

				void Visit(GlrUseSyntax* node) override
				{
					syntaxMinLength = 1;
					if (loopCounter > 0)
					{
						context.global.AddError(
							ParserErrorType::UseRuleUsedInLoopBody,
							ruleSymbol->Name(),
							node->name.value
							);
					}
					if (optionalCounter > 0)
					{
						context.global.AddError(
							ParserErrorType::UseRuleUsedInOptionalBody,
							ruleSymbol->Name(),
							node->name.value
							);
					}
				}

				void Visit(GlrLoopSyntax* node) override
				{
					vint bodyMinLength = 0;
					vint delimiterMinLength = 0;
					loopCounter++;
					node->syntax->Accept(this);
					bodyMinLength = syntaxMinLength;
					if (node->delimiter)
					{
						node->delimiter->Accept(this);
						delimiterMinLength = syntaxMinLength;
					}
					if (delimiterMinLength + bodyMinLength == 0)
					{
						context.global.AddError(
							ParserErrorType::LoopBodyCouldExpandToEmptySequence,
							ruleSymbol->Name()
							);
					}
					syntaxMinLength = 0;
					loopCounter--;
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					optionalCounter++;
					node->syntax->Accept(this);
					if (syntaxMinLength == 0)
					{
						context.global.AddError(
							ParserErrorType::OptionalBodyCouldExpandToEmptySequence,
							ruleSymbol->Name()
							);
					}
					syntaxMinLength = 0;
					optionalCounter--;
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->first->Accept(this);
					vint firstMinLength = syntaxMinLength;
					node->second->Accept(this);
					vint secondMinLength = syntaxMinLength;
					syntaxMinLength = firstMinLength + secondMinLength;
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					node->first->Accept(this);
					vint firstMinLength = syntaxMinLength;
					node->second->Accept(this);
					vint secondMinLength = syntaxMinLength;
					syntaxMinLength = firstMinLength < secondMinLength ? firstMinLength : secondMinLength;
				}

				void Visit(GlrCreateClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					if (syntaxMinLength == 0)
					{
						context.global.AddError(
							ParserErrorType::ClauseCouldExpandToEmptySequence,
							ruleSymbol->Name()
							);
					}
				}

				void Visit(GlrPartialClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					if (syntaxMinLength == 0)
					{
						context.global.AddError(
							ParserErrorType::ClauseCouldExpandToEmptySequence,
							ruleSymbol->Name()
							);
					}
				}

				void Visit(GlrReuseClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					if (syntaxMinLength == 0)
					{
						context.global.AddError(
							ParserErrorType::ClauseCouldExpandToEmptySequence,
							ruleSymbol->Name()
							);
					}
				}
			};

/***********************************************************************
ValidateStructure
***********************************************************************/

			void ValidateStructure(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files)
			{
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
						ValidateStructureVisitor visitor(context, ruleSymbol);
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