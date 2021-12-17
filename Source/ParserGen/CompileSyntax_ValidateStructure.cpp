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
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					clause = nullptr;

				vint						optionalCounter = 0;
				vint						loopCounter = 0;
				bool						lastSyntaxPiece = true;
				bool						prioritySyntaxOccurred = false;

				vint						syntaxMinLength = 0;
				vint						syntaxMinUseRuleCount = 0;
				vint						syntaxMaxUseRuleCount = 0;

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
								context.global.AddError(
									ParserErrorType::NonArrayFieldAssignedInLoop,
									ruleSymbol->Name(),
									clauseType->Name(),
									prop->Name()
									);
							}
						}

						vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
						if (ruleIndex != -1)
						{
							auto fieldRule = context.syntaxManager.Rules().Values()[ruleIndex];
							if (fieldRule->isPartial && fieldRule->assignedNonArrayField)
							{
								context.global.AddError(
									ParserErrorType::NonLoopablePartialRuleUsedInLoop,
									ruleSymbol->Name(),
									clauseType->Name(),
									fieldRule->Name()
									);
							}
						}
					}
				}

				void Visit(GlrLiteralSyntax* node) override
				{
					syntaxMinLength = 1;
					syntaxMinUseRuleCount = 0;
					syntaxMaxUseRuleCount = 0;
				}

				void Visit(GlrUseSyntax* node) override
				{
					syntaxMinLength = 1;
					syntaxMinUseRuleCount = 1;
					syntaxMaxUseRuleCount = 1;

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
						context.global.AddError(
							ParserErrorType::LoopBodyCouldExpandToEmptySequence,
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
							context.global.AddError(
								ParserErrorType::MultiplePrioritySyntaxInAClause,
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
						context.global.AddError(
							ParserErrorType::OptionalBodyCouldExpandToEmptySequence,
							ruleSymbol->Name()
							);
					}

					if (node->priority == GlrOptionalPriority::PreferSkip && lastSyntaxPiece)
					{
						context.global.AddError(
							ParserErrorType::NegativeOptionalEndsAClause,
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

				void CheckAfterClause(bool reuseClause)
				{
					if (syntaxMinLength == 0)
					{
						context.global.AddError(
							ParserErrorType::ClauseCouldExpandToEmptySequence,
							ruleSymbol->Name()
							);
					}
					if (reuseClause)
					{
						if (syntaxMinUseRuleCount == 0)
						{
							context.global.AddError(
								ParserErrorType::ClauseNotCreateObject,
								ruleSymbol->Name()
							);
						}
						if (syntaxMaxUseRuleCount > 1)
						{
							context.global.AddError(
								ParserErrorType::ClauseTooManyUseRule,
								ruleSymbol->Name()
							);
						}
					}
				}

				void Visit(GlrCreateClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					CheckAfterClause(false);
				}

				void Visit(GlrPartialClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					CheckAfterClause(false);
				}

				void Visit(GlrReuseClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					CheckAfterClause(true);
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
				struct Link
				{
					GlrRefSyntax*			ref = nullptr;
					Link*					prev = nullptr;
					Link*					next = nullptr;

					Link(GlrRefSyntax* _ref) : ref(_ref) {}
				};

				struct LinkPair
				{
					Link*					first = nullptr;
					Link*					last = nullptr;

					void EnsureComplete()
					{
						CHECK_ERROR(!first || !first->prev, L"Illegal Operation!");
						CHECK_ERROR(!last || !last->next, L"Illegal Operation!");
					}

					LinkPair Append(Link* link)
					{
						EnsureComplete();
						if (first)
						{
							link->prev = last;
							last->next = link;
							return { first,link };
						}
						else
						{
							return { link,link };
						}
					}

					LinkPair Connect(LinkPair pair)
					{
						EnsureComplete();
						pair.EnsureComplete();
						if (!first && !pair.first)
						{
							return {};
						}
						else if (!first)
						{
							return pair;
						}
						else if (!pair.first)
						{
							return *this;
						}
						else
						{
							last->next = pair.first;
							pair.first->prev = last;
							return { first,pair.last };
						}
					}

					void CutAfter(Link* link, LinkPair& l1, LinkPair& l2)
					{
						EnsureComplete();
						if (!first)
						{
							CHECK_ERROR(!link, L"Illegal Operation!");
							l1 = {};
							l2 = {};
						}
						else if (!link)
						{
							l2 = *this;
							l1 = {};
						}
						else if (link == last)
						{
							l1 = *this;
							l2 = {};
						}
						else
						{
							auto a = first;
							auto b = last;
							l1 = { a,link };
							l2 = { link->next,b };
							l1.last->next = nullptr;
							l2.first->prev = nullptr;
						}
					}

					operator bool() const
					{
						return last;
					}
				};
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					clause = nullptr;

				LinkPair					existingFields;
				LinkPair					existingPartials;
			public:
				ValidateStructureRelationshipVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				~ValidateStructureRelationshipVisitor()
				{
					{
						auto c = existingFields.first;
						while (c)
						{
							auto n = c->next;
							delete c;
							c = n;
						}
					}
					{
						auto c = existingPartials.first;
						while (c)
						{
							auto n = c->next;
							delete c;
							c = n;
						}
					}
				}

				void ValidateClause(Ptr<GlrClause> clause)
				{
					clause->Accept(this);
				}

			protected:
				void Visit(GlrRefSyntax* node) override
				{
					if (node->field)
					{
						existingFields = existingFields.Append(new Link(node));
					}

					vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (ruleIndex != -1)
					{
						auto fieldRule = context.syntaxManager.Rules().Values()[ruleIndex];
						if (fieldRule->isPartial)
						{
							existingPartials= existingPartials.Append(new Link(node));
						}
					}
				}

				void Visit(GlrLiteralSyntax* node) override
				{
				}

				void Visit(GlrUseSyntax* node) override
				{
					if (existingFields)
					{
						context.global.AddError(
							ParserErrorType::UseRuleAppearAfterField,
							ruleSymbol->Name(),
							node->name.value,
							existingFields.last->ref->field.value
							);
					}
					else if (existingPartials)
					{
						context.global.AddError(
							ParserErrorType::UseRuleAppearAfterPartialRule,
							ruleSymbol->Name(),
							node->name.value,
							existingPartials.last->ref->name.value
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
					auto currentFields = existingFields;
					auto currentPartials = existingPartials;

					node->first->Accept(this);

					LinkPair firstFields, firstPartials;
					existingFields.CutAfter(currentFields.last, existingFields, firstFields);
					existingPartials.CutAfter(currentPartials.last, existingPartials, firstPartials);

					node->second->Accept(this);

					existingFields = existingFields.Connect(firstFields);
					existingPartials = existingPartials.Connect(firstPartials);
				}

				void CheckAfterClause()
				{
					Dictionary<WString, vint> counters;
					auto c = existingFields.first;
					while (c)
					{
						auto fieldName = c->ref->field.value;
						vint index = counters.Keys().IndexOf(fieldName);
						if (index == -1)
						{
							counters.Add(fieldName, 1);
						}
						else
						{
							counters.Set(fieldName, counters.Values()[index] + 1);
						}
						c = c->next;
					}

					auto clauseType = context.clauseTypes[clause];
					for (auto [key, value] : counters)
					{
						auto prop = FindPropSymbol(clauseType, key);
						if (prop->propType != AstPropType::Array && value > 1)
						{
							context.global.AddError(
								ParserErrorType::FieldAssignedMoreThanOnce,
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
					CheckAfterClause();
				}

				void Visit(GlrPartialClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					CheckAfterClause();
				}

				void Visit(GlrReuseClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					CheckAfterClause();
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
}