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
ValidateStructureCountingVisitor
***********************************************************************/

			class ValidateStructureCountingVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&							context;
				RuleSymbol*								ruleSymbol;
				GlrClause*								clause = nullptr;
				GlrLeftRecursionPlaceholderClause*		lrpClause = nullptr;

				vint									optionalCounter = 0;
				vint									loopCounter = 0;
				bool									lastSyntaxPiece = true;
				bool									prioritySyntaxOccurred = false;

				vint									syntaxMinLength = 0;
				vint									syntaxMinUseRuleCount = 0;
				vint									syntaxMaxUseRuleCount = 0;

			public:
				ValidateStructureCountingVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void ValidateClause(Ptr<GlrClause> clause)
				{
					optionalCounter = 0;
					loopCounter = 0;
					lastSyntaxPiece = true;
					prioritySyntaxOccurred = false;
					clause->Accept(this);
				}

			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefSyntax* node) override
				{
					syntaxMinLength = 1;
					syntaxMinUseRuleCount = 0;
					syntaxMaxUseRuleCount = 0;

					if (loopCounter > 0)
					{
						auto clauseType = context.clauseTypes[clause];

						if (node->field)
						{
							auto prop = FindPropSymbol(clauseType, node->field.value);
							if (prop->propType != AstPropType::Array)
							{
								context.syntaxManager.AddError(
									ParserErrorType::NonArrayFieldAssignedInLoop,
									node->codeRange,
									ruleSymbol->Name(),
									clauseType->Name(),
									prop->Name()
									);
							}
						}

						if (node->refType == GlrRefType::Id)
						{
							vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(node->literal.value);
							if (ruleIndex != -1)
							{
								auto fieldRule = context.syntaxManager.Rules().Values()[ruleIndex];
								if (fieldRule->isPartial && fieldRule->assignedNonArrayField)
								{
									context.syntaxManager.AddError(
										ParserErrorType::NonLoopablePartialRuleUsedInLoop,
										node->codeRange,
										ruleSymbol->Name(),
										clauseType->Name(),
										fieldRule->Name()
									);
								}
							}
						}
					}
				}

				void Visit(GlrUseSyntax* node) override
				{
					syntaxMinLength = 1;
					syntaxMinUseRuleCount = 1;
					syntaxMaxUseRuleCount = 1;

					if (loopCounter > 0)
					{
						context.syntaxManager.AddError(
							ParserErrorType::UseRuleUsedInLoopBody,
							node->codeRange,
							ruleSymbol->Name(),
							node->name.value
							);
					}
					if (optionalCounter > 0)
					{
						context.syntaxManager.AddError(
							ParserErrorType::UseRuleUsedInOptionalBody,
							node->codeRange,
							ruleSymbol->Name(),
							node->name.value
							);
					}
				}

				void Visit(GlrLoopSyntax* node) override
				{
					vint bodyMinLength = 0, bodyMaxUseRuleCount = 0;
					vint delimiterMinLength = 0, delimiterMaxUseRuleCount = 0;

					loopCounter++;

					node->syntax->Accept(this);
					bodyMinLength = syntaxMinLength;
					bodyMaxUseRuleCount = syntaxMaxUseRuleCount;

					if (node->delimiter)
					{
						node->delimiter->Accept(this);
						delimiterMinLength = syntaxMinLength;
						delimiterMaxUseRuleCount = syntaxMaxUseRuleCount;
					}

					if (delimiterMinLength + bodyMinLength == 0)
					{
						context.syntaxManager.AddError(
							ParserErrorType::LoopBodyCouldExpandToEmptySequence,
							node->codeRange,
							ruleSymbol->Name()
							);
					}
					syntaxMinLength = 0;
					syntaxMinUseRuleCount = 0;
					syntaxMaxUseRuleCount = bodyMaxUseRuleCount * 2 + delimiterMaxUseRuleCount * 2;

					loopCounter--;
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					if (node->priority != GlrOptionalPriority::Equal)
					{
						if (prioritySyntaxOccurred)
						{
							context.syntaxManager.AddError(
								ParserErrorType::MultiplePrioritySyntaxInAClause,
								node->codeRange,
								ruleSymbol->Name()
								);
						}
						else
						{
							prioritySyntaxOccurred = true;
						}
					}
					optionalCounter++;

					node->syntax->Accept(this);

					if (syntaxMinLength == 0)
					{
						context.syntaxManager.AddError(
							ParserErrorType::OptionalBodyCouldExpandToEmptySequence,
							node->codeRange,
							ruleSymbol->Name()
							);
					}

					if (node->priority == GlrOptionalPriority::PreferSkip && lastSyntaxPiece)
					{
						context.syntaxManager.AddError(
							ParserErrorType::NegativeOptionalEndsAClause,
							node->codeRange,
							ruleSymbol->Name()
							);
					}
					syntaxMinLength = 0;
					syntaxMinUseRuleCount = 0;

					optionalCounter--;
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->second->Accept(this);
					vint secondMinLength = syntaxMinLength;
					vint secondMinUseRuleCount = syntaxMinUseRuleCount;
					vint secondMaxUseRuleCount = syntaxMaxUseRuleCount;

					bool last = lastSyntaxPiece;
					lastSyntaxPiece = last && secondMinLength == 0;
					node->first->Accept(this);
					vint firstMinLength = syntaxMinLength;
					vint firstMinUseRuleCount = syntaxMinUseRuleCount;
					vint firstMaxUseRuleCount = syntaxMaxUseRuleCount;
					lastSyntaxPiece = last;

					syntaxMinLength = firstMinLength + secondMinLength;
					syntaxMinUseRuleCount = firstMinUseRuleCount + secondMinUseRuleCount;
					syntaxMaxUseRuleCount = firstMaxUseRuleCount + secondMaxUseRuleCount;
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					node->first->Accept(this);
					vint firstMinLength = syntaxMinLength;
					vint firstMinUseRuleCount = syntaxMinUseRuleCount;
					vint firstMaxUseRuleCount = syntaxMaxUseRuleCount;

					node->second->Accept(this);
					vint secondMinLength = syntaxMinLength;
					vint secondMinUseRuleCount = syntaxMinUseRuleCount;
					vint secondMaxUseRuleCount = syntaxMaxUseRuleCount;

					syntaxMinLength = firstMinLength < secondMinLength ? firstMinLength : secondMinLength;
					syntaxMinUseRuleCount = firstMinUseRuleCount < secondMinUseRuleCount ? firstMinUseRuleCount : secondMinUseRuleCount;
					syntaxMaxUseRuleCount = firstMaxUseRuleCount > secondMaxUseRuleCount ? firstMaxUseRuleCount : secondMaxUseRuleCount;
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

				void CheckAfterClause(GlrClause* node, bool reuseClause)
				{
					if (syntaxMinLength == 0)
					{
						context.syntaxManager.AddError(
							ParserErrorType::ClauseCouldExpandToEmptySequence,
							node->codeRange,
							ruleSymbol->Name()
							);
					}
					if (reuseClause)
					{
						if (syntaxMinUseRuleCount == 0)
						{
							context.syntaxManager.AddError(
								ParserErrorType::ClauseNotCreateObject,
								node->codeRange,
								ruleSymbol->Name()
								);
						}
						if (syntaxMaxUseRuleCount > 1)
						{
							context.syntaxManager.AddError(
								ParserErrorType::ClauseTooManyUseRule,
								node->codeRange,
								ruleSymbol->Name()
								);
						}
					}
				}

				void Visit(GlrCreateClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					CheckAfterClause(node, false);
				}

				void Visit(GlrPartialClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					CheckAfterClause(node, false);
				}

				void Visit(GlrReuseClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					CheckAfterClause(node, true);
				}

				void Visit(GlrLeftRecursionPlaceholderClause* node) override
				{
					if (!lrpClause)
					{
						lrpClause = node;
					}
					else
					{
						context.syntaxManager.AddError(
							ParserErrorType::TooManyLeftRecursionPlaceholderClauses,
							node->codeRange,
							ruleSymbol->Name()
							);
					}
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
				}

				void Visit(GlrPrefixMergeClause* node) override
				{
				}
			};

/***********************************************************************
ValidateStructureRelationshipVisitor
***********************************************************************/

			class ValidateStructureRelationshipVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					clause = nullptr;
				Dictionary<WString, vint>	fieldCounters;

			public:
				ValidateStructureRelationshipVisitor(
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

				void AddFieldCounter(const WString& name, vint counter)
				{
					vint index = fieldCounters.Keys().IndexOf(name);
					if (index == -1)
					{
						fieldCounters.Add(name, counter);
					}
					else
					{
						const_cast<List<vint>&>(fieldCounters.Values())[index] += counter;
					}
				}

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefSyntax* node) override
				{
					if (node->field)
					{
						AddFieldCounter(node->field.value, 1);
					}
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
					Dictionary<WString, vint> currentCounters(std::move(fieldCounters));

					node->first->Accept(this);
					Dictionary<WString, vint> firstCounters(std::move(fieldCounters));

					node->second->Accept(this);
					Dictionary<WString, vint> secondCounters(std::move(fieldCounters));

					vint firstIndex = 0;
					vint secondIndex = 0;
					fieldCounters = std::move(currentCounters);
					while (true)
					{
						bool firstAvailable = firstIndex < firstCounters.Count();
						bool secondAvailable = secondIndex < secondCounters.Count();

						if (firstAvailable && secondAvailable)
						{
							auto firstKey = firstCounters.Keys()[firstIndex];
							auto secondKey = secondCounters.Keys()[secondIndex];
							auto compare = firstKey <=> secondKey;

							if (compare == std::strong_ordering::less)
							{
								secondAvailable = false;
							}
							else if (compare == std::strong_ordering::greater)
							{
								firstAvailable = false;
							}
						}

						if (firstAvailable && secondAvailable)
						{
							vint firstValue = firstCounters.Values()[firstIndex];
							vint secondValue = secondCounters.Values()[secondIndex];
							AddFieldCounter(firstCounters.Keys()[firstIndex], (firstValue > secondValue ? firstValue : secondValue));
							firstIndex++;
							secondIndex++;
						}
						else if (firstAvailable)
						{
							AddFieldCounter(firstCounters.Keys()[firstIndex], firstCounters.Values()[firstIndex]);
							firstIndex++;
						}
						else if (secondAvailable)
						{
							AddFieldCounter(secondCounters.Keys()[secondIndex], secondCounters.Values()[secondIndex]);
							firstIndex++;
						}
						else
						{
							break;
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

				void CheckAfterClause(GlrClause* node)
				{
					auto clauseType = context.clauseTypes[clause];
					for (auto [key, value] : fieldCounters)
					{
						auto prop = FindPropSymbol(clauseType, key);
						if (prop->propType != AstPropType::Array && value > 1)
						{
							context.syntaxManager.AddError(
								ParserErrorType::FieldAssignedMoreThanOnce,
								node->codeRange,
								ruleSymbol->Name(),
								clauseType->Name(),
								prop->Name()
								);
						}
					}
				}

				void Visit(GlrCreateClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					CheckAfterClause(node);
				}

				void Visit(GlrPartialClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					CheckAfterClause(node);
				}

				void Visit(GlrReuseClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					CheckAfterClause(node);
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
ValidateStructure
***********************************************************************/

			void ValidateStructure(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile)
			{
				for (auto rule : syntaxFile->rules)
				{
					auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
					ValidateStructureCountingVisitor visitor1(context, ruleSymbol);
					for (auto clause : rule->clauses)
					{
						ValidateStructureRelationshipVisitor visitor2(context, ruleSymbol);
						visitor1.ValidateClause(clause);
						visitor2.ValidateClause(clause);
					}
				}
			}
		}
	}
}