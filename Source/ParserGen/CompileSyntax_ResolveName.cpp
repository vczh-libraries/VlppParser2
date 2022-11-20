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
ResolveNameVisitor
***********************************************************************/

			class ResolveNameVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrCondition::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				VisitorSwitchContext&		sContext;
				SortedList<WString>&		accessedSwitches;
				RuleSymbol*					ruleSymbol;
				GlrReuseClause*				reuseClause = nullptr;

			public:
				ResolveNameVisitor(
					VisitorContext& _context,
					VisitorSwitchContext& _sContext,
					SortedList<WString>& _accessedSwitches,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, sContext(_sContext)
					, accessedSwitches(_accessedSwitches)
					, ruleSymbol(_ruleSymbol)
				{
				}

				AstClassSymbol* GetRuleClass(ParsingToken& typeName)
				{
					vint index = context.astManager.Symbols().Keys().IndexOf(typeName.value);
					if (index == -1)
					{
						context.syntaxManager.AddError(
							ParserErrorType::TypeNotExistsInRule,
							typeName.codeRange,
							ruleSymbol->Name(),
							typeName.value
							);
						return nullptr;
					}

					auto classSymbol = dynamic_cast<AstClassSymbol*>(context.astManager.Symbols().Values()[index]);
					if (!classSymbol)
					{
						context.syntaxManager.AddError(
							ParserErrorType::TypeNotClassInRule,
							typeName.codeRange,
							ruleSymbol->Name(),
							typeName.value
							);
					}
					return classSymbol;
				}

				void ResolveClause(Ptr<GlrClause> clause)
				{
					context.clauseToRules.Add(clause.Obj(), ruleSymbol);
					clause->Accept(this);
				}

			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefSyntax* node) override
				{
					switch (node->refType)
					{
					case GlrRefType::Id:
						{
							vint tokenIndex = context.lexerManager.TokenOrder().IndexOf(node->literal.value);
							vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(node->literal.value);
							if (tokenIndex == -1 && ruleIndex == -1)
							{
								context.syntaxManager.AddError(
									ParserErrorType::TokenOrRuleNotExistsInRule,
									node->codeRange,
									ruleSymbol->Name(),
									node->literal.value
									);
							}
						}
						break;
					case GlrRefType::Literal:
						{
							if (node->literal.value.Length() > 2)
							{
								auto literalValue = UnescapeLiteral(node->literal.value, L'\"');
								for (auto&& [tokenName, tokenIndex] : indexed(context.lexerManager.TokenOrder()))
								{
									auto tokenSymbol = context.lexerManager.Tokens()[tokenName];
									if (tokenSymbol->displayText == literalValue)
									{
										if (tokenSymbol->discarded)
										{
											context.syntaxManager.AddError(
												ParserErrorType::LiteralIsDiscardedToken,
												node->codeRange,
												ruleSymbol->Name(),
												node->literal.value
												);
										}
										else
										{
											context.literalTokens.Add(node, (vint32_t)tokenIndex);
										}
										return;
									}
								}
							}
							context.syntaxManager.AddError(
								ParserErrorType::LiteralNotValidToken,
								node->codeRange,
								ruleSymbol->Name(),
								node->literal.value
								);
						}
						break;
					case GlrRefType::ConditionalLiteral:
						{
							if (node->literal.value.Length() > 2)
							{
								auto literalValue = UnescapeLiteral(node->literal.value, L'\'');
								auto&& lexer = context.GetCachedLexer();
								List<regex::RegexToken> tokens;
								lexer.Parse(literalValue).ReadToEnd(tokens);
								if (tokens.Count() == 1 && tokens[0].token != -1 && tokens[0].completeToken)
								{
									auto tokenSymbol = context.lexerManager.Tokens()[context.lexerManager.TokenOrder()[tokens[0].token]];
									if (tokenSymbol->displayText != L"")
									{
										context.syntaxManager.AddError(
											ParserErrorType::ConditionalLiteralIsDisplayText,
											node->codeRange,
											ruleSymbol->Name(),
											node->literal.value
											);
									}
									if (tokenSymbol->discarded)
									{
										context.syntaxManager.AddError(
											ParserErrorType::ConditionalLiteralIsDiscardedToken,
											node->codeRange,
											ruleSymbol->Name(),
											node->literal.value
											);
									}
									context.literalTokens.Add(node, (vint32_t)tokens[0].token);
									return;
								}
							}
							context.syntaxManager.AddError(
								ParserErrorType::ConditionalLiteralNotValidToken,
								node->codeRange,
								ruleSymbol->Name(),
								node->literal.value
								);
						}
						break;
					default:;
					}
				}

				void VisitReuseSyntax(ParsingToken& name, bool addRuleReuseDependency)
				{
					vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(name.value);
					if (ruleIndex == -1)
					{
						context.syntaxManager.AddError(
							ParserErrorType::TokenOrRuleNotExistsInRule,
							name.codeRange,
							ruleSymbol->Name(),
							name.value
							);
					}
					else
					{
						auto usedRuleSymbol = context.syntaxManager.Rules().Values()[ruleIndex];
						if (addRuleReuseDependency)
						{
							if (!context.ruleReuseDependencies.Contains(ruleSymbol, usedRuleSymbol))
							{
								context.ruleReuseDependencies.Add(ruleSymbol, usedRuleSymbol);
							}
						}
						if (reuseClause)
						{
							if (!context.clauseReuseDependencies.Contains(reuseClause, usedRuleSymbol))
							{
								context.clauseReuseDependencies.Add(reuseClause, usedRuleSymbol);
							}
						}
					}
				}

				void Visit(GlrUseSyntax* node) override
				{
					VisitReuseSyntax(node->name, reuseClause != nullptr);
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

				void VisitSwitchItems(List<Ptr<GlrSwitchItem>>& switches)
				{
					for (auto&& switchItem : switches)
					{
						if (!sContext.switches.Keys().Contains(switchItem->name.value))
						{
							context.syntaxManager.AddError(
								ParserErrorType::SwitchNotExists,
								switchItem->name.codeRange,
								ruleSymbol->Name(),
								switchItem->name.value
							);
						}
						else if (!accessedSwitches.Contains(switchItem->name.value))
						{
							accessedSwitches.Add(switchItem->name.value);
						}
					}
				}

				void Visit(GlrPushConditionSyntax* node) override
				{
					VisitSwitchItems(node->switches);
					node->syntax->Accept(this);
				}

				void Visit(GlrTestConditionSyntax* node) override
				{
					for (auto&& branch : node->branches)
					{
						branch->condition->Accept(this);
						if (branch->syntax)
						{
							branch->syntax->Accept(this);
						}
					}
				}

				////////////////////////////////////////////////////////////////////////
				// GlrCondition::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefCondition* node) override
				{
					if (!sContext.switches.Keys().Contains(node->name.value))
					{
						context.syntaxManager.AddError(
							ParserErrorType::SwitchNotExists,
							node->name.codeRange,
							ruleSymbol->Name(),
							node->name.value
							);
					}
					else if (!accessedSwitches.Contains(node->name.value))
					{
						accessedSwitches.Add(node->name.value);
					}
				}

				void Visit(GlrNotCondition* node) override
				{
					node->condition->Accept(this);
				}

				void Visit(GlrAndCondition* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				void Visit(GlrOrCondition* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				////////////////////////////////////////////////////////////////////////
				// GlrClause::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrCreateClause* node) override
				{
					if (auto classSymbol = GetRuleClass(node->type))
					{
						context.ruleKnownTypes.Add(ruleSymbol, classSymbol);
						context.clauseTypes.Add(node, classSymbol);
					}
					node->syntax->Accept(this);
				}

				void Visit(GlrPartialClause* node) override
				{
					if (auto classSymbol = GetRuleClass(node->type))
					{
						context.ruleKnownTypes.Add(ruleSymbol, classSymbol);
						context.clauseTypes.Add(node, classSymbol);
					}
					node->syntax->Accept(this);
				}

				void Visit(GlrReuseClause* node) override
				{
					reuseClause = node;
					node->syntax->Accept(this);
				}

				void Visit(GlrLeftRecursionPlaceholderClause* node) override
				{
					for (auto flag : node->flags)
					{
						auto name = flag->flag.value;
						if (!ruleSymbol->lrFlags.Contains(name))
						{
							ruleSymbol->lrFlags.Add(name);
						}
					}
					context.directLrpClauses.Add(ruleSymbol, node);
				}

				void VisitLriClause(GlrLeftRecursionInjectClause* node)
				{
					VisitReuseSyntax(node->rule->literal, true);
					if (node->continuation)
					{
						for (auto lriTarget : node->continuation->injectionTargets)
						{
							VisitLriClause(lriTarget.Obj());
						}
					}
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
					VisitLriClause(node);
					context.directLriClauses.Add(ruleSymbol, node);
				}

				void Visit(GlrPrefixMergeClause* node) override
				{
					VisitReuseSyntax(node->rule->literal, true);
					context.directPmClauses.Add(ruleSymbol, node);
				}
			};

/***********************************************************************
ResolveName
***********************************************************************/

			bool IsLegalNameBeforeWithSwitch(const WString& name)
			{
				if (wcsstr(name.Buffer(), L"_SWITCH")) return false;
				if (wcsstr(name.Buffer(), L"SWITCH_")) return false;
				return true;
			}

			bool IsLegalNameBeforeWithPrefixMerge(const WString& name)
			{
				if (wcsstr(name.Buffer(), L"_LRI")) return false;
				if (wcsstr(name.Buffer(), L"_LRIP")) return false;
				if (wcsstr(name.Buffer(), L"LRI_")) return false;
				if (wcsstr(name.Buffer(), L"LRIP_")) return false;
				return true;
			}

			void ResolveName(VisitorContext& context, VisitorSwitchContext& sContext, Ptr<GlrSyntaxFile> syntaxFile)
			{
				for (auto switchItem : syntaxFile->switches)
				{
					if (sContext.switches.Keys().Contains(switchItem->name.value))
					{
						context.syntaxManager.AddError(
							ParserErrorType::DuplicatedSwitch,
							switchItem->name.codeRange,
							switchItem->name.value
							);
					}
					else
					{
						sContext.switches.Add(
							switchItem->name.value, {
								(switchItem->value == GlrSwitchValue::True),
								switchItem.Obj()
							});
					}
				}

				for (auto rule : syntaxFile->rules)
				{
					context.astRules.Add(context.syntaxManager.Rules()[rule->name.value], rule.Obj());
				}

				SortedList<WString> accessedSwitches;
				for (auto rule : syntaxFile->rules)
				{
					auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
					ResolveNameVisitor visitor(context, sContext, accessedSwitches, ruleSymbol);
					if (rule->type)
					{
						ruleSymbol->ruleType = visitor.GetRuleClass(rule->type);
					}
					for (auto clause : rule->clauses)
					{
						visitor.ResolveClause(clause);
					}
				}

				{
					vint index = 0;
					for (auto&& switchName : sContext.switches.Keys())
					{
						if (index == accessedSwitches.Count() || switchName != accessedSwitches[index])
						{
							context.syntaxManager.AddError(
								ParserErrorType::UnusedSwitch,
								sContext.switches[switchName].value->codeRange,
								switchName
								);
						}
						else
						{
							index++;
						}
					}
				}

				if (sContext.switches.Count() > 0)
				{
					for (auto rule : syntaxFile->rules)
					{
						if (!IsLegalNameBeforeWithSwitch(rule->name.value))
						{
							context.syntaxManager.AddError(
								ParserErrorType::SyntaxInvolvesSwitchWithIllegalRuleName,
								rule->name.codeRange,
								rule->name.value
								);
						}
					}
				}

				if (context.directPmClauses.Count() > 0)
				{
					for (auto rule : syntaxFile->rules)
					{
						if (!IsLegalNameBeforeWithPrefixMerge(rule->name.value))
						{
							context.syntaxManager.AddError(
								ParserErrorType::SyntaxInvolvesPrefixMergeWithIllegalRuleName,
								rule->name.codeRange,
								rule->name.value
								);
						}

						for (auto lrp : From(rule->clauses).FindType<GlrLeftRecursionPlaceholderClause>())
						{
							for (auto p : lrp->flags)
							{
								if (!IsLegalNameBeforeWithPrefixMerge(p->flag.value))
								{
									context.syntaxManager.AddError(
										ParserErrorType::SyntaxInvolvesPrefixMergeWithIllegalPlaceholderName,
										p->flag.codeRange,
										rule->name.value,
										p->flag.value
										);
								}
							}
						}
					}
				}
			}
		}
	}
}