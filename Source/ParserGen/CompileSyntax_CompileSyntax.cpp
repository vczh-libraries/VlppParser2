#include "Compiler.h"
#include "../Syntax/SyntaxSymbolWriter.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

/***********************************************************************
CompileSyntaxVisitor
***********************************************************************/

			class CompileSyntaxVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
				using StatePair = AutomatonBuilder::StatePair;
			protected:
				AutomatonBuilder	automatonBuilder;
				VisitorContext&		context;
				AstClassSymbol*		clauseType;
				StatePair			result;

				StatePair Build(const Ptr<GlrSyntax>& node)
				{
					node->Accept(this);
					return result;
				}
			public:
				CompileSyntaxVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: automatonBuilder(_ruleSymbol)
					, context(_context)
				{
				}

				void AssignClause(const Ptr<GlrClause>& node)
				{
					node->Accept(this);
				}

			protected:
				void Visit(GlrRefSyntax* node) override
				{
					vint32_t field = -1;
					if (node->field)
					{
						auto propSymbol = FindPropSymbol(clauseType, node->field.value);
						field = context.output->fieldIds[propSymbol];
					}

					switch (node->refType)
					{
					case GlrRefType::Id:
						{
							vint index = context.lexerManager.TokenOrder().IndexOf(node->literal.value);
							if (index != -1)
							{
								auto token = context.lexerManager.Tokens()[node->literal.value];
								auto displayText = token->displayText == L"" ? token->Name() : L"\"" + token->displayText + L"\"";
								result = automatonBuilder.BuildTokenSyntax((vint32_t)index, displayText, field);
								return;
							}
						}
						{
							vint index = context.syntaxManager.Rules().Keys().IndexOf(node->literal.value);
							if (index != -1)
							{
								auto rule = context.syntaxManager.Rules().Values()[index];
								result = automatonBuilder.BuildRuleSyntax(rule, field);
								return;
							}
						}
						CHECK_FAIL(L"Should not reach here!");
						break;
					case GlrRefType::Literal:
						{
							vint index = context.literalTokens[node];
							auto token = context.lexerManager.Tokens()[context.lexerManager.TokenOrder()[index]];
							auto displayText = token->displayText == L"" ? token->Name() : L"\"" + token->displayText + L"\"";
							result = automatonBuilder.BuildTokenSyntax((vint32_t)index, displayText, field);
						}
						break;
					case GlrRefType::ConditionalLiteral:
						throw 0;
					default:;
					}
				}

				void Visit(GlrUseSyntax* node) override
				{
					auto rule = context.syntaxManager.Rules()[node->name.value];
					result = automatonBuilder.BuildUseSyntax(rule);
				}

				void Visit(GlrLoopSyntax* node) override
				{
					result = automatonBuilder.BuildLoopSyntax(
						[this, node]() { return Build(node->syntax); },
						[this, node]() { return Build(node->delimiter); },
						node->delimiter
						);
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					result = automatonBuilder.BuildOptionalSyntax(
						node->priority == GlrOptionalPriority::PreferTake,
						node->priority == GlrOptionalPriority::PreferSkip,
						[this, node]() { return Build(node->syntax); }
						);
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					result = automatonBuilder.BuildSequenceSyntax(
						[this, node]() { return Build(node->first); },
						[this, node]() { return Build(node->second); }
						);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					result = automatonBuilder.BuildAlternativeSyntax(
						[this, node]() { return Build(node->first); },
						[this, node]() { return Build(node->second); }
						);
				}

				StatePair BuildAssignments(StatePair pair, List<Ptr<GlrAssignment>>& assignments)
				{
					for (auto node : assignments)
					{
						auto propSymbol = FindPropSymbol(clauseType, node->field.value);
						auto enumSymbol = dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol);
						auto enumItem = (vint32_t)enumSymbol->ItemOrder().IndexOf(node->value.value);
						auto field = context.output->fieldIds[propSymbol];
						pair = automatonBuilder.BuildAssignment(pair, enumItem, field);
					}
					return pair;
				}

				void Visit(GlrCreateClause* node) override
				{
					clauseType = context.clauseTypes[node];
					result = automatonBuilder.BuildClause([this, node]()
					{
						return automatonBuilder.BuildCreateClause(
							context.output->classIds[clauseType],
							[this, node]() { return BuildAssignments(Build(node->syntax), node->assignments); }
							);
					});
				}

				void Visit(GlrPartialClause* node) override
				{
					clauseType = context.clauseTypes[node];
					result = automatonBuilder.BuildClause([this, node]()
					{
						return automatonBuilder.BuildPartialClause(
							[this, node]() { return BuildAssignments(Build(node->syntax), node->assignments); }
							);
					});
				}

				void Visit(GlrReuseClause* node) override
				{
					clauseType = context.clauseTypes[node];
					result = automatonBuilder.BuildClause([this, node]()
					{
						return automatonBuilder.BuildReuseClause(
							[this, node]() { return BuildAssignments(Build(node->syntax), node->assignments); }
							);
					});
				}
			};

/***********************************************************************
CompileSyntax
***********************************************************************/

			void CompileSyntax(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files)
			{
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
						CompileSyntaxVisitor visitor(context, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							visitor.AssignClause(clause);
						}
					}
				}
			}
		}
	}
}