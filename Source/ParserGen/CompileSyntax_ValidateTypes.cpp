#include "Compiler.h"

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
SearchForLrpVisitor
***********************************************************************/

			class SearchForLrpVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				WString						flagToSearch;
				AstClassSymbol*				typeToMatch = nullptr;

				vint						counter = 0;
				SortedList<GlrRule*>		searchedRules;

				bool						couldBeEmpty = false;
				RuleSymbol*					currentPlaceholderRule = nullptr;

				void SearchInRuleInternal(const WString& ruleName)
				{
					vint index = context.syntaxManager.Rules().Keys().IndexOf(ruleName);
					if (index != -1)
					{
						auto ruleSymbol = context.syntaxManager.Rules().Values()[index];
						auto ruleAst = context.astRules[ruleSymbol];

						if (searchedRules.Contains(ruleAst)) return;
						searchedRules.Add(ruleAst);

						auto oldRule = currentPlaceholderRule;
						currentPlaceholderRule = ruleSymbol;
						for (auto clause : ruleAst->clauses)
						{
							clause->Accept(this);
						}
						currentPlaceholderRule = oldRule;
					}
				}
			public:
				List<WString>				unmatchedPlaceholderRuleNames;

				SearchForLrpVisitor(
					VisitorContext& _context,
					const WString& _flag,
					AstClassSymbol* _type
				)
					: context(_context)
					, flagToSearch(_flag)
					, typeToMatch(_type)
				{
				}

				vint SearchInRule(const WString& ruleName)
				{
					counter = 0;
					searchedRules.Clear();
					unmatchedPlaceholderRuleNames.Clear();
					SearchInRuleInternal(ruleName);
					return counter;
				}
			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefSyntax* node) override
				{
					if (node->refType == GlrRefType::Id)
					{
						SearchInRuleInternal(node->literal.value);
					}
					couldBeEmpty = false;
				}

				void Visit(GlrUseSyntax* node) override
				{
					SearchInRuleInternal(node->name.value);
					couldBeEmpty = false;
				}

				void Visit(GlrLoopSyntax* node) override
				{
					node->syntax->Accept(this);
					couldBeEmpty = true;
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					node->syntax->Accept(this);
					couldBeEmpty = true;
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->first->Accept(this);
					if (couldBeEmpty) node->second->Accept(this);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					node->first->Accept(this);
					bool firstCouldBeEmpty = couldBeEmpty;
					node->second->Accept(this);
					bool secondCouldBeEmpty = couldBeEmpty;
					couldBeEmpty = firstCouldBeEmpty || secondCouldBeEmpty;
				}

				void Visit(GlrPushConditionSyntax* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrTestConditionSyntax* node) override
				{
					bool emptyBranch = false;
					for (auto branch : node->branches)
					{
						if (branch->syntax)
						{
							branch->syntax->Accept(this);
						}
						else
						{
							emptyBranch = true;
						}
					}
					couldBeEmpty = emptyBranch;
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
					for (auto flag : node->flags)
					{
						if (flag->flag.value == flagToSearch)
						{
							counter++;
							if (!ConvertibleTo(typeToMatch, currentPlaceholderRule->ruleType))
							{
								unmatchedPlaceholderRuleNames.Add(currentPlaceholderRule->Name());
							}
							break;
						}
					}
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
					node->rule->Accept(this);
				}
			};

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
					{
						auto rule = context.syntaxManager.Rules()[node->rule->literal.value];
						if (rule->isPartial)
						{
							context.syntaxManager.AddError(
								ParserErrorType::PartialRuleInLeftRecursionInject,
								node->rule->codeRange,
								ruleSymbol->Name(),
								node->rule->literal.value
								);
						}
					}

					SearchForLrpVisitor visitor(context, node->continuation->flag->flag.value, ruleSymbol->ruleType);
					for (auto lriTarget : node->continuation->injectionTargets)
					{
						auto target = lriTarget->rule;
						vint counter = visitor.SearchInRule(target->literal.value);
						if (counter == 0)
						{
							context.syntaxManager.AddError(
								ParserErrorType::LeftRecursionPlaceholderNotFoundInRule,
								target->codeRange,
								ruleSymbol->Name(),
								node->continuation->flag->flag.value,
								target->literal.value
								);
						}
						else if (counter > 1)
						{
							context.syntaxManager.AddError(
								ParserErrorType::LeftRecursionPlaceholderNotUnique,
								target->codeRange,
								ruleSymbol->Name(),
								node->continuation->flag->flag.value,
								target->literal.value
								);
						}
						else
						{
							for (auto ruleName : visitor.unmatchedPlaceholderRuleNames)
							{
								context.syntaxManager.AddError(
									ParserErrorType::LeftRecursionPlaceholderTypeMismatched,
									target->codeRange,
									ruleSymbol->Name(),
									node->continuation->flag->flag.value,
									target->literal.value,
									ruleName
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
							visitor.ValidateClause(clause);
						}
					}
				}
			}
		}
	}
}