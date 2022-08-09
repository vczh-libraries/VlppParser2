#include "Compiler.h"
#include "../ParserGen_Generated/ParserGenRuleAst_Empty.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

			bool ConvertibleTo(AstClassSymbol* from, AstClassSymbol* to)
			{
				while (from)
				{
					if (from == to) return true;
					from = from->baseClass;
				}
				return false;
			}

/***********************************************************************
ValidateTypesVisitor
***********************************************************************/

			class ValidateTypesVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					clause = nullptr;
				
				AstClassPropSymbol* FindField(AstClassSymbol*& clauseType, ParsingToken& name)
				{
					clauseType = context.clauseTypes[clause];
					if (auto prop = FindPropSymbol(clauseType, name.value))
					{
						if (prop->propType != AstPropType::Array)
						{
							ruleSymbol->assignedNonArrayField = true;
						}
						return prop;
					}
					else
					{
						context.syntaxManager.AddError(
							ParserErrorType::FieldNotExistsInClause,
							name.codeRange,
							ruleSymbol->Name(),
							clauseType->Name(),
							name.value
							);
						return nullptr;
					}
				}

			public:
				ValidateTypesVisitor(
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

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefSyntax* node) override
				{
					vint ruleIndex = node->refType == GlrRefType::Id ? context.syntaxManager.Rules().Keys().IndexOf(node->literal.value) : -1;
					auto clauseType = context.clauseTypes[clause];
					auto fieldRule = ruleIndex == -1 ? nullptr : context.syntaxManager.Rules().Values()[ruleIndex];

					if (fieldRule && fieldRule->isPartial)
					{
						if (!ConvertibleTo(clauseType, fieldRule->ruleType))
						{
							context.syntaxManager.AddError(
								ParserErrorType::ClauseTypeMismatchedToPartialRule,
								node->codeRange,
								ruleSymbol->Name(),
								clauseType->Name(),
								fieldRule->Name(),
								fieldRule->ruleType->Name()
								);
						}
					}

					if (node->field)
					{
						AstClassSymbol* clauseType = nullptr;
						if (auto prop = FindField(clauseType, node->field))
						{
							if (fieldRule)
							{
								if (fieldRule->isPartial)
								{
									context.syntaxManager.AddError(
										ParserErrorType::PartialRuleUsedOnField,
										node->codeRange,
										ruleSymbol->Name(),
										clauseType->Name(),
										fieldRule->Name(),
										node->field.value
										);
								}

								if (auto propClassSymbol = dynamic_cast<AstClassSymbol*>(prop->propSymbol))
								{
									if (ConvertibleTo(fieldRule->ruleType, propClassSymbol))
									{
										goto PASS_FIELD_TYPE;
									}
								}
								context.syntaxManager.AddError(
									ParserErrorType::RuleTypeMismatchedToField,
									node->codeRange,
									ruleSymbol->Name(),
									clauseType->Name(),
									node->field.value,
									fieldRule->ruleType->Name()
									);
							PASS_FIELD_TYPE:;
							}
							else
							{
								if (prop->propType != AstPropType::Token)
								{
									context.syntaxManager.AddError(
										ParserErrorType::RuleTypeMismatchedToField,
										node->codeRange,
										ruleSymbol->Name(),
										clauseType->Name(),
										node->field.value,
										L"token"
										);
								}
							}
						}
					}
				}

				void Visit(GlrUseSyntax* node) override
				{
					auto fieldRule = context.syntaxManager.Rules()[node->name.value];
					if (fieldRule->isPartial)
					{
						context.syntaxManager.AddError(
							ParserErrorType::UseRuleWithPartialRule,
							node->codeRange,
							ruleSymbol->Name(),
							node->name.value
							);
					}
					if (!dynamic_cast<GlrReuseClause*>(clause))
					{
						context.syntaxManager.AddError(
							ParserErrorType::UseRuleInNonReuseClause,
							node->codeRange,
							ruleSymbol->Name(),
							node->name.value
							);
					}
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

				void Visit(GlrPushConditionSyntax* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrTestConditionSyntax* node) override
				{
					for (auto&& branch : node->branches)
					{
						if (branch->syntax)
						{
							branch->syntax->Accept(this);
						}
					}
				}

				////////////////////////////////////////////////////////////////////////
				// GlrClause::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrAssignment* node)
				{
					AstClassSymbol* clauseType = nullptr;
					if (auto prop = FindField(clauseType, node->field))
					{
						if (auto enumPropSymbol = dynamic_cast<AstEnumSymbol*>(prop->propSymbol))
						{
							if (!enumPropSymbol->Items().Keys().Contains(node->value.value))
							{
								context.syntaxManager.AddError(
									ParserErrorType::EnumItemMismatchedToField,
									node->codeRange,
									ruleSymbol->Name(),
									clauseType->Name(),
									node->field.value,
									node->value.value
									);
							}
						}
						else
						{
							context.syntaxManager.AddError(
								ParserErrorType::AssignmentToNonEnumField,
								node->codeRange,
								ruleSymbol->Name(),
								clauseType->Name(),
								node->field.value
								);
						}
					}
				}

				void Visit(GlrCreateClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					for (auto assignment : node->assignments)
					{
						Visit(assignment.Obj());
					}
				}

				void Visit(GlrPartialClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					for (auto assignment : node->assignments)
					{
						Visit(assignment.Obj());
					}
				}

				void Visit(GlrReuseClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					for (auto assignment : node->assignments)
					{
						Visit(assignment.Obj());
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
					auto prefixRule = context.syntaxManager.Rules()[node->rule->literal.value];
					if (prefixRule->isPartial)
					{
						context.syntaxManager.AddError(
							ParserErrorType::PartialRuleInPrefixMerge,
							node->rule->codeRange,
							ruleSymbol->Name(),
							node->rule->literal.value
							);
					}
				}
			};

/***********************************************************************
LriVerifyTypesVisitor
***********************************************************************/

			class LriVerifyTypesVisitor
				: public empty_visitor::ClauseVisitor
			{
			protected:
				VisitorContext&									context;
				RuleSymbol*										ruleSymbol;

			public:
				LriVerifyTypesVisitor(
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

				////////////////////////////////////////////////////////////////////////
				// GlrClause::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
					auto prefixRule = context.syntaxManager.Rules()[node->rule->literal.value];
					if (prefixRule->isPartial)
					{
						context.syntaxManager.AddError(
							ParserErrorType::PartialRuleInLeftRecursionInject,
							node->rule->codeRange,
							ruleSymbol->Name(),
							node->rule->literal.value
							);
					}

					for (auto lriTarget : node->continuation->injectionTargets)
					{
						auto target = lriTarget->rule;
						List<GlrLeftRecursionPlaceholderClause*> lrpClauses;
						{
							vint index = context.indirectLrpClauses.Keys().IndexOf(context.syntaxManager.Rules()[target->literal.value]);
							if (index != -1)
							{
								CopyFrom(
									lrpClauses,
									From(context.indirectLrpClauses.GetByIndex(index))
										.Where([node](auto&& lrp)
										{
											return !From(lrp->flags)
												.Where([node](auto&& flag) { return flag->flag.value == node->continuation->flag->flag.value; })
												.IsEmpty();
										}));
							}
						}

						if (lrpClauses.Count() == 0)
						{
							context.syntaxManager.AddError(
								ParserErrorType::LeftRecursionPlaceholderNotFoundInRule,
								target->codeRange,
								ruleSymbol->Name(),
								node->continuation->flag->flag.value,
								target->literal.value
								);
						}
						else if (lrpClauses.Count() > 1 && node->continuation->configuration == GlrLeftRecursionConfiguration::Single)
						{
							context.syntaxManager.AddError(
								ParserErrorType::LeftRecursionPlaceholderNotUnique,
								target->codeRange,
								ruleSymbol->Name(),
								node->continuation->flag->flag.value,
								target->literal.value
								);
						}

						for (auto lrpClause : lrpClauses)
						{
							auto lrpClauseRule = context.clauseToRules[lrpClause];
							if (!ConvertibleTo(prefixRule->ruleType, lrpClauseRule->ruleType))
							{
								context.syntaxManager.AddError(
									ParserErrorType::LeftRecursionPlaceholderTypeMismatched,
									target->codeRange,
									ruleSymbol->Name(),
									node->continuation->flag->flag.value,
									target->literal.value,
									lrpClauseRule->Name()
									);
							}
						}

						if (lriTarget->continuation)
						{
							Visit(lriTarget.Obj());
						}
					}
				}
			};

/***********************************************************************
LriPrefixTestingVisitor
***********************************************************************/

			class LriPrefixTestingVisitor
				: public empty_visitor::ClauseVisitor
			{
			protected:
				VisitorContext&									context;
				RuleSymbol*										ruleSymbol;
				Group<GlrLeftRecursionInjectClause*, WString>	lriEndings;

			public:
				LriPrefixTestingVisitor(
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

				////////////////////////////////////////////////////////////////////////
				// GlrClause::IVisitor
				////////////////////////////////////////////////////////////////////////

				void SearchLriEndings(List<GlrLeftRecursionInjectClause*>& visiting, GlrLeftRecursionInjectClause* node)
				{
					visiting.Add(node);
					if (!node->continuation || node->continuation->type == GlrLeftRecursionInjectContinuationType::Optional)
					{
						for (auto lri : visiting)
						{
							if (!lriEndings.Contains(lri, node->rule->literal.value))
							{
								lriEndings.Add(lri, node->rule->literal.value);
							}
						}
					}

					if (node->continuation)
					{
						for (auto target : node->continuation->injectionTargets)
						{
							SearchLriEndings(visiting, target.Obj());
						}
					}
					visiting.RemoveAt(visiting.Count() - 1);
				}

				void VerifyPrefix(GlrLeftRecursionInjectClause* node)
				{
					if (node->continuation)
					{
						for (auto t1 : node->continuation->injectionTargets)
						{
							auto k1 = context.syntaxManager.Rules()[t1->rule->literal.value];
							vint i1 = lriEndings.Keys().IndexOf(t1.Obj());
							if (i1 == -1) continue;
							for (auto t2 : node->continuation->injectionTargets)
							{
								auto k2 = context.syntaxManager.Rules()[t2->rule->literal.value];
								vint i2 = lriEndings.Keys().IndexOf(t2.Obj());
								if (i2 == -1) continue;

								if (t1 != t2 && context.indirectStartPathToLastRules.Keys().Contains({ k2, k1 }))
								{
									auto&& e1 = lriEndings.GetByIndex(i1);
									auto&& e2 = lriEndings.GetByIndex(i2);
									if (!From(e1).Intersect(e2).IsEmpty())
									{
										context.syntaxManager.AddError(
											ParserErrorType::LeftRecursionInjectTargetIsPrefixOfAnotherSameEnding,
											node->codeRange,
											ruleSymbol->Name(),
											node->continuation->flag->flag.value,
											k1->Name(),
											k2->Name()
											);
									}
								}
							}
						}
					}
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
					{
						List<GlrLeftRecursionInjectClause*> visiting;
						SearchLriEndings(visiting, node);
					}
					VerifyPrefix(node);
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
						ValidateTypesVisitor vtVisitor(context, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							vtVisitor.ValidateClause(clause);
							{
								LriVerifyTypesVisitor lvtVisitor(context, ruleSymbol);
								lvtVisitor.ValidateClause(clause);
							}
							{
								LriPrefixTestingVisitor lptVisitor(context, ruleSymbol);
								lptVisitor.ValidateClause(clause);
							}
						}
					}
				}
			}
		}
	}
}