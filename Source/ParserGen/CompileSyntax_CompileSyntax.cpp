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
				AutomatonBuilder						automatonBuilder;
				VisitorContext&							context;
				Ptr<CppParserGenOutput>					output;
				AstClassSymbol*							clauseType;
				StatePair								result;

				StatePair Build(GlrSyntax* node)
				{
					node->Accept(this);
					return result;
				}

				StatePair Build(const Ptr<GlrSyntax>& node)
				{
					node->Accept(this);
					return result;
				}
			public:
				CompileSyntaxVisitor(
					VisitorContext& _context,
					Ptr<CppParserGenOutput> _output,
					RuleSymbol* _ruleSymbol
				)
					: automatonBuilder(_ruleSymbol)
					, context(_context)
					, output(_output)
				{
				}

				void AssignClause(const Ptr<GlrClause>& node)
				{
					node->Accept(this);
				}

			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefSyntax* node) override
				{
					vint32_t field = -1;
					if (node->field)
					{
						auto propSymbol = FindPropSymbol(clauseType, node->field.value);
						field = output->fieldIds[propSymbol];
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
								result = automatonBuilder.BuildTokenSyntax((vint32_t)index, displayText, {}, field);
								return;
							}
						}
						{
							vint index = context.syntaxManager.Rules().Keys().IndexOf(node->literal.value);
							if (index != -1)
							{
								auto rule = context.syntaxManager.Rules().Values()[index];
								if (rule->isPartial)
								{
									result = automatonBuilder.BuildPartialRuleSyntax(rule);
								}
								else if (field == -1)
								{
									result = automatonBuilder.BuildDiscardRuleSyntax(rule);
								}
								else
								{
									result = automatonBuilder.BuildFieldRuleSyntax(rule, field);
								}
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
							result = automatonBuilder.BuildTokenSyntax((vint32_t)index, displayText, {}, field);
						}
						break;
					case GlrRefType::ConditionalLiteral:
						{
							vint index = context.literalTokens[node];
							auto token = context.lexerManager.Tokens()[context.lexerManager.TokenOrder()[index]];
							auto condition = UnescapeLiteral(node->literal.value, L'\'');
							auto displayText = token->displayText == L"" ? token->Name() : L"\"" + token->displayText + L"\"";
							result = automatonBuilder.BuildTokenSyntax((vint32_t)index, displayText, condition, field);
						}
						break;
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

				template<typename T>
				void CollectElements(GlrSyntax* node, List<Func<StatePair()>>& elements)
				{
					if (auto pair = dynamic_cast<T*>(node))
					{
						CollectElements<T>(pair->first.Obj(), elements);
						CollectElements<T>(pair->second.Obj(), elements);
					}
					else
					{
						elements.Add([this, node]() { return Build(node); });
					}
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					List<Func<StatePair()>> elements;
					CollectElements<GlrSequenceSyntax>(node, elements);
					result = automatonBuilder.BuildSequenceSyntax(elements);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					List<Func<StatePair()>> elements;
					CollectElements<GlrAlternativeSyntax>(node, elements);
					result = automatonBuilder.BuildAlternativeSyntax(elements);
				}

				void Visit(GlrPushConditionSyntax* node) override
				{
					CHECK_FAIL(L"GlrPushConditionSyntax should have been removed after RewriteSyntax_Switch()!");
				}

				void Visit(GlrTestConditionSyntax* node) override
				{
					CHECK_FAIL(L"GlrTestConditionSyntax should have been removed after RewriteSyntax_Switch()!");
				}

				////////////////////////////////////////////////////////////////////////
				// GlrClause::IVisitor
				////////////////////////////////////////////////////////////////////////

				StatePair BuildAssignments(StatePair pair, List<Ptr<GlrAssignment>>& assignments)
				{
					for (auto node : assignments)
					{
						auto propSymbol = FindPropSymbol(clauseType, node->field.value);
						auto enumSymbol = dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol);
						auto enumItem = (vint32_t)enumSymbol->ItemOrder().IndexOf(node->value.value);
						auto field = output->fieldIds[propSymbol];
						pair = automatonBuilder.BuildAssignment(pair, enumItem, field, (node->type == GlrAssignmentType::Weak));
					}
					return pair;
				}

				void Visit(GlrCreateClause* node) override
				{
					clauseType = context.clauseTypes[node];
					result = automatonBuilder.BuildClause([this, node]()
					{
						return automatonBuilder.BuildCreateClause(
							output->classIds[clauseType],
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

				void Visit(GlrLeftRecursionPlaceholderClause* node) override
				{
					List<vint32_t> flags;
					CopyFrom(
						flags,
						From(node->flags)
							.Select([this](Ptr<GlrLeftRecursionPlaceholder> flag)
							{
								return (vint32_t)context.syntaxManager.lrpFlags.IndexOf(flag->flag.value);
							})
							.Distinct()
						);

					result = automatonBuilder.BuildClause([this, &flags]()
					{
						return automatonBuilder.BuildLrpClause(flags, [&](vint32_t flag) { return context.syntaxManager.lrpFlags[flag]; });
					});
				}

				using StateBuilder = Func<AutomatonBuilder::StatePair()>;

				StateBuilder CompileLriTarget(vint32_t parentFlag, GlrLeftRecursionInjectClause* lriTarget)
				{
					StateBuilder useOrLriSyntax;
					auto rule = context.syntaxManager.Rules()[lriTarget->rule->literal.value];
					if (parentFlag == -1)
					{
						useOrLriSyntax = [this, rule]() { return automatonBuilder.BuildUseSyntax(rule); };
					}
					else
					{
						useOrLriSyntax = [this, rule, parentFlag]() { return automatonBuilder.BuildLriSyntax(parentFlag, rule); };
					}

					if (!lriTarget->continuation)
					{
						return useOrLriSyntax;
					}
					else
					{
						auto cont = lriTarget->continuation;
						return [this, useOrLriSyntax, cont]()
						{
							bool optional = cont->type == GlrLeftRecursionInjectContinuationType::Optional;
							List<StateBuilder> targetRules;

							for (auto lriFlag : cont->flags)
							{
								auto flag = (vint32_t)context.syntaxManager.lrpFlags.IndexOf(lriFlag->flag.value);
								for (auto lriTarget : cont->injectionTargets)
								{
									targetRules.Add(CompileLriTarget(flag, lriTarget.Obj()));
								}
							}

							return automatonBuilder.BuildLriClauseSyntax(
								useOrLriSyntax,
								optional,
								std::move(targetRules));
						};
					}
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
					result = automatonBuilder.BuildClause([this, node]()
					{
						return automatonBuilder.BuildReuseClause(CompileLriTarget(-1, node));
					});
				}

				void Visit(GlrPrefixMergeClause* node) override
				{
					CHECK_FAIL(L"GlrPrefixMergeClause should have been removed after RewriteSyntax_PrefixMerge()!");
				}
			};

/***********************************************************************
CompileSyntax
***********************************************************************/

			void CompileSyntax(VisitorContext& context, Ptr<CppParserGenOutput> output, Ptr<GlrSyntaxFile> syntaxFile)
			{
				for (auto rule : syntaxFile->rules)
				{
					auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
					CompileSyntaxVisitor visitor(context, output, ruleSymbol);
					for (auto clause : rule->clauses)
					{
						visitor.AssignClause(clause);
					}
				}
			}
		}
	}
}