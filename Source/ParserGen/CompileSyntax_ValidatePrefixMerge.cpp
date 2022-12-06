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
ValidateDirectPrefixMergeRuleVisitor
***********************************************************************/

			class ValidateDirectPrefixMergeRuleVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;

			public:
				ValidateDirectPrefixMergeRuleVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void ValidateClause(Ptr<GlrClause> clause)
				{
					clause->Accept(this);
				}

			protected:

				void NotBeginWithARule(ParsingAstBase* node)
				{
					// TODO: Should accept and generate !prefix_merge equivalent effect for this clause automatically
					context.syntaxManager.AddError(
						ParserErrorType::RuleMixedPrefixMergeWithClauseNotSyntacticallyBeginWithARule,
						node->codeRange,
						ruleSymbol->Name()
						);
				}

				void VerifyStartRule(ParsingAstBase* node, RuleSymbol* startRule)
				{
					if (ruleSymbol != startRule && !context.indirectPmClauses.Keys().Contains(startRule))
					{
						// TODO: Should accept and generate !prefix_merge equivalent effect for this clause automatically
						//       When this is not a left-recursive clause
						context.syntaxManager.AddError(
							ParserErrorType::RuleMixedPrefixMergeWithClauseNotBeginWithIndirectPrefixMerge,
							node->codeRange,
							ruleSymbol->Name(),
							startRule->Name()
							);
					}
				}

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefSyntax* node) override
				{
					if (node->refType != GlrRefType::Id)
					{
						NotBeginWithARule(node);
					}
					else
					{
						vint index = context.syntaxManager.Rules().Keys().IndexOf(node->literal.value);
						if (index == -1)
						{
							NotBeginWithARule(node);
						}
						else
						{
							VerifyStartRule(node, context.syntaxManager.Rules().Values()[index]);
						}
					}
				}

				void Visit(GlrUseSyntax* node) override
				{
					VerifyStartRule(node, context.syntaxManager.Rules()[node->name.value]);
				}

				void Visit(GlrLoopSyntax* node) override
				{
					NotBeginWithARule(node);
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					NotBeginWithARule(node);
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->first->Accept(this);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					NotBeginWithARule(node);
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

				void Visit(GlrCreateClause* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrPartialClause* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrReuseClause* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrLeftRecursionPlaceholderClause* node) override
				{
					NotBeginWithARule(node);
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
					NotBeginWithARule(node);
				}

				void Visit(GlrPrefixMergeClause* node) override
				{
				}
			};

/***********************************************************************
ValidateIndirectPrefixMergeRuleVisitor
***********************************************************************/

			class ValidateIndirectPrefixMergeRuleVisitor
				: public Object
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				RuleSymbol*					pmRuleSymbol;

			public:
				ValidateIndirectPrefixMergeRuleVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol,
					RuleSymbol* _pmRuleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
					, pmRuleSymbol(_pmRuleSymbol)
				{
				}

				void ValidateClause(Ptr<GlrClause> clause)
				{
					clause->Accept(this);
				}

			protected:

				void NotSimpleUsingRule(GlrClause* node)
				{
					if (context.leftRecursiveClauses.Contains(ruleSymbol, node))
					{
						goto SKIP_ADDING;
					}

					// safe to add the clause since errors have been detected in ClausePartiallyIndirectlyBeginsWithPrefixMerge
					{
						vint index = context.clauseToStartRules.Keys().IndexOf(node);
						if (index != -1)
						{
							for (auto ruleSymbol : context.clauseToStartRules.GetByIndex(index))
							{
								if (context.indirectPmClauses.Keys().IndexOf(ruleSymbol) != -1)
								{
									goto SKIP_ADDING;
								}
							}
						}
						context.clauseToConvertedToPrefixMerge.Add(node);
					}
				SKIP_ADDING:;
				}

				////////////////////////////////////////////////////////////////////////
				// GlrClause::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrCreateClause* node) override
				{
					NotSimpleUsingRule(node);
				}

				void Visit(GlrPartialClause* node) override
				{
					NotSimpleUsingRule(node);
				}

				void Visit(GlrReuseClause* node) override
				{
					if (!dynamic_cast<GlrUseSyntax*>(node->syntax.Obj()))
					{
						NotSimpleUsingRule(node);
					}
					else if (node->assignments.Count() > 0)
					{
						NotSimpleUsingRule(node);
					}
				}

				void Visit(GlrLeftRecursionPlaceholderClause* node) override
				{
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
				}

				void Visit(GlrPrefixMergeClause* node) override
				{
				}
			};

/***********************************************************************
ValidateDeducingPrefixMergeRuleVisitor
***********************************************************************/

			class ValidateDeducingPrefixMergeRuleVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				using NonSimpleUseDeduction = Pair<const ParsingToken*, RuleSymbol*>;

				VisitorContext&						context;
				RuleSymbol*							ruleSymbol;

				Ptr<List<NonSimpleUseDeduction>>	result;
				bool								couldBeEmpty = false;

			public:
				ValidateDeducingPrefixMergeRuleVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void ValidateClause(Ptr<GlrClause> clause)
				{
					clause->Accept(this);
					if (result)
					{
						for (auto [refName, refRuleSymbol] : *result.Obj())
						{
							context.syntaxManager.AddError(
								ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,
								clause->codeRange,
								ruleSymbol->Name(),
								refRuleSymbol->Name(),
								refName->value
								);
						}
					}
				}

			protected:

				void TryAppendResult(const ParsingToken& name, RuleSymbol* refRuleSymbol)
				{
					if (context.directPmClauses.Keys().Contains(refRuleSymbol))
					{
						if (!result) result = Ptr(new List<NonSimpleUseDeduction>);
						result->Add({ &name,refRuleSymbol });
					}
				}

				void VisitRuleRef(const ParsingToken& name)
				{
					vint index = context.syntaxManager.Rules().Keys().IndexOf(name.value);
					if (index == -1) return;

					auto refRuleSymbol = context.syntaxManager.Rules().Values()[index];
					TryAppendResult(name, refRuleSymbol);

					index = context.indirectSimpleUseRules.Keys().IndexOf(refRuleSymbol);
					if (index == -1) return;

					for (auto [simpleUseSymbol, _] : context.indirectSimpleUseRules.GetByIndex(index))
					{
						TryAppendResult(name, simpleUseSymbol);
					}
				}

				void VisitPotentialEmptySyntax()
				{
					// a clause is not allowed to produce an empty sequence
					// since this syntax could produce an empty sequence
					// therefore there should be other non-parallel part of syntax in this clause that produces a non-empty sequence
					// it doesn't need to check the error here
					result = nullptr;
					couldBeEmpty = true;
				}

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefSyntax* node) override
				{
					result = nullptr;
					couldBeEmpty = false;

					if (node->refType != GlrRefType::Id) return;
					VisitRuleRef(node->literal);
				}

				void Visit(GlrUseSyntax* node) override
				{
					result = nullptr;
					couldBeEmpty = false;

					VisitRuleRef(node->name);
				}

				void Visit(GlrLoopSyntax* node) override
				{
					VisitPotentialEmptySyntax();
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					VisitPotentialEmptySyntax();
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->first->Accept(this);
					auto firstResult = result;
					bool firstEmpty = couldBeEmpty;

					node->second->Accept(this);
					auto secondResult = result;
					auto secondEmpty = couldBeEmpty;

					couldBeEmpty = firstEmpty && secondEmpty;

					if (!firstEmpty && !secondEmpty)
					{
						result = nullptr;
					}
					else if (!firstEmpty)
					{
						result = firstResult;
					}
					else if (!secondEmpty)
					{
						result = secondResult;
					}
					else
					{
						result = nullptr;
					}
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					node->first->Accept(this);
					auto firstResult = result;
					bool firstEmpty = couldBeEmpty;

					node->second->Accept(this);
					auto secondResult = result;
					auto secondEmpty = couldBeEmpty;

					if (firstEmpty || secondEmpty)
					{
						VisitPotentialEmptySyntax();
					}
					else
					{
						result = nullptr;
						couldBeEmpty = true;

						if (firstResult && secondResult)
						{
							CopyFrom(*firstResult.Obj(), *secondResult.Obj(), true);
							result = firstResult;
						}
						else if (firstResult)
						{
							result = firstResult;
						}
						else if (secondResult)
						{
							result = secondResult;
						}
					}
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

				void Visit(GlrCreateClause* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrPartialClause* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrReuseClause* node) override
				{
					if (!dynamic_cast<GlrUseSyntax*>(node->syntax.Obj()))
					{
						node->syntax->Accept(this);
					}
				}

				void Visit(GlrLeftRecursionPlaceholderClause* node) override
				{
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
					if (!node->continuation || node->continuation->type == GlrLeftRecursionInjectContinuationType::Optional)
					{
						VisitRuleRef(node->rule->literal);
					}
				}

				void Visit(GlrPrefixMergeClause* node) override
				{
				}
			};

/***********************************************************************
ValidatePrefixMerge
***********************************************************************/

			void ValidatePrefixMerge(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile)
			{
				for (auto rule : syntaxFile->rules)
				{
					auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];

					if (context.directPmClauses.Keys().Contains(ruleSymbol))
					{
						ValidateDirectPrefixMergeRuleVisitor visitor(context, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							visitor.ValidateClause(clause);
						}
					}

					{
						ValidateDeducingPrefixMergeRuleVisitor visitor(context, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							visitor.ValidateClause(clause);
						}
					}

					vint indexPm = context.indirectPmClauses.Keys().IndexOf(ruleSymbol);
					vint indexLrp = context.indirectLrpClauses.Keys().IndexOf(ruleSymbol);
					vint indexLri = context.indirectLriClauses.Keys().IndexOf(ruleSymbol);

					if (indexPm != -1)
					{
						auto rulePm = context.clauseToRules[context.indirectPmClauses.GetByIndex(indexPm)[0]];
						if (indexLrp != -1)
						{
							auto ruleLrp = context.clauseToRules[context.indirectLrpClauses.GetByIndex(indexLrp)[0]];
							context.syntaxManager.AddError(
								ParserErrorType::RuleIndirectlyBeginsWithPrefixMergeMixedLeftRecursionMarkers,
								rule->name.codeRange,
								ruleSymbol->Name(),
								rulePm->Name(),
								ruleLrp->Name()
								);
						}
						if (indexLri != -1)
						{
							auto ruleLri = context.clauseToRules[context.indirectLriClauses.GetByIndex(indexLri)[0]];
							context.syntaxManager.AddError(
								ParserErrorType::RuleIndirectlyBeginsWithPrefixMergeMixedLeftRecursionMarkers,
								rule->name.codeRange,
								ruleSymbol->Name(),
								rulePm->Name(),
								ruleLri->Name()
								);
						}
						if (ruleSymbol->isPartial)
						{
							context.syntaxManager.AddError(
								ParserErrorType::PartialRuleIndirectlyBeginsWithPrefixMerge,
								rule->name.codeRange,
								ruleSymbol->Name(),
								rulePm->Name()
								);
						}

						if (!context.directPmClauses.Keys().Contains(ruleSymbol))
						{
							ValidateIndirectPrefixMergeRuleVisitor visitor3(context, ruleSymbol, rulePm);
							for (auto clause : rule->clauses)
							{
								ParsingToken firstDirectLiteral;
								RuleSymbol* firstIndirectNonPmRule = nullptr;
								GlrPrefixMergeClause* firstIndirectlyPmClause = nullptr;

								// test if this clause directly begins with and literal
								vint index = context.clauseBeginsWithLiteral.Keys().IndexOf(clause.Obj());
								if (index != -1)
								{
									firstDirectLiteral = context.clauseBeginsWithLiteral.GetByIndex(index)[0];
								}

								// find all direct start rules from this clause
								index = context.clauseToStartRules.Keys().IndexOf(clause.Obj());
								if (index != -1)
								{
									List<RuleSymbol*> visiting;
									SortedList<RuleSymbol*> visited;
									CopyFrom(visiting, context.clauseToStartRules.GetByIndex(index));

									for (vint i = 0; i < visiting.Count(); i++)
									{
										auto visitingRule = visiting[i];
										if (visited.Contains(visitingRule)) continue;
										visited.Add(visitingRule);

										// test if the visiting rule begins with prefix_merge
										index = context.indirectPmClauses.Keys().IndexOf(visitingRule);
										if (index != -1)
										{
											if (!firstIndirectlyPmClause)
											{
												firstIndirectlyPmClause = context.indirectPmClauses.GetByIndex(index)[0];
											}
										}
										else
										{
											// if the visiting rule itself partial begins with prefix_merge
											// then an errors should be generated for this rule
											// here we assume it never happens in order to reduce the number of errors
											if (!firstIndirectNonPmRule)
											{
												firstIndirectNonPmRule = visitingRule;
											}
										}
										if ((firstDirectLiteral || firstIndirectNonPmRule) && firstIndirectlyPmClause) break;
									}
								}

								if (firstDirectLiteral && firstIndirectlyPmClause)
								{
									context.syntaxManager.AddError(
										ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral,
										clause->codeRange,
										ruleSymbol->Name(),
										context.clauseToRules[firstIndirectlyPmClause]->Name(),
										firstDirectLiteral.value
										);
								}
								else if (firstIndirectNonPmRule && firstIndirectlyPmClause)
								{
									context.syntaxManager.AddError(
										ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule,
										clause->codeRange,
										ruleSymbol->Name(),
										context.clauseToRules[firstIndirectlyPmClause]->Name(),
										firstIndirectNonPmRule->Name()
										);
								}
								else
								{
									visitor3.ValidateClause(clause);
								}
							}
						}
					}
				}
			}
		}
	}
}