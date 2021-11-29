#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{

/***********************************************************************
CheckSyntaxVisitor
***********************************************************************/

/***********************************************************************
CompileSyntaxVisitor
***********************************************************************/

			class CompileSyntaxVisitor
				: public Object
				, public virtual GlrSyntax::IVisitor
				, public virtual GlrClause::IVisitor
			{
			protected:
				struct StatePair
				{
					StateSymbol* begin;
					StateSymbol* end;
				};

				using StatePosMap = collections::Dictionary<StateSymbol*, vint>;
			protected:
				ParserSymbolManager&		global;
				AstSymbolManager&			astManager;
				LexerSymbolManager&			lexerManager;
				SyntaxSymbolManager&		syntaxManager;
				RuleSymbol*					ruleSymbol;
				WString						clauseDisplayText;
				StatePosMap					startPoses;
				StatePosMap					endPoses;
				StatePair					result;

				StateSymbol* CreateState()
				{
					return ruleSymbol->Owner()->CreateState(ruleSymbol);
				}

				EdgeSymbol* CreateEdge(StateSymbol* from, StateSymbol* to)
				{
					return ruleSymbol->Owner()->CreateEdge(from, to);
				}

				StatePair Build(const Ptr<GlrSyntax>& node)
				{
					node->Accept(this);
					return result;
				}

				StatePair Build(const Ptr<GlrClause>& node)
				{
					node->Accept(this);
					return result;
				}
			public:
				CompileSyntaxVisitor(AstSymbolManager& _astManager, LexerSymbolManager& _lexerManager, SyntaxSymbolManager& _syntaxManager, RuleSymbol* _ruleSymbol)
					: global(syntaxManager.Global())
					, astManager(_astManager)
					, lexerManager(_lexerManager)
					, syntaxManager(_syntaxManager)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void AssignClause(const Ptr<GlrClause>& node)
				{
					clauseDisplayText = L"";
					startPoses.Clear();
					endPoses.Clear();

					auto pair = Build(node);
					ruleSymbol->startStates.Add(pair.begin);
					pair.end->endingState = true;

					vint l = clauseDisplayText.Length();
					for (auto [state, pos] : startPoses)
					{
						state->label = clauseDisplayText.Left(pos) + L"@ " + clauseDisplayText.Right(l - pos);
					}
					for (auto [state, pos] : endPoses)
					{
						state->label = clauseDisplayText.Left(pos) + L" @" + clauseDisplayText.Right(l - pos);
					}
				}

				void Visit(GlrRefSyntax* node) override
				{
					{
						vint index = lexerManager.TokenOrder().IndexOf(node->name.value);
						if (index != -1)
						{
							StatePair pair;
							pair.begin = CreateState();
							pair.end = CreateState();
							startPoses.Add(pair.begin, clauseDisplayText.Length());

							{
								auto edge = CreateEdge(pair.begin, pair.end);
								edge->input.type = EdgeInputType::Token;
								edge->input.token = index;
								if (node->field)
								{
									edge->insAfterInput.Add({ AstInsType::Token });
									edge->insAfterInput.Add({ AstInsType::Field,clause.field });
								}
							}

							clauseDisplayText += lexerManager.Tokens()[node->name.value]->displayText;
							endPoses.Add(pair.end, clauseDisplayText.Length());
							result = pair;
							return;
						}
					}
					{
						vint index = syntaxManager.Rules().Keys().IndexOf(node->name.value);
						if (index != -1)
						{
							auto rule = syntaxManager.Rules().Values()[index];
							StatePair pair;
							pair.begin = CreateState();
							pair.end = CreateState();
							startPoses.Add(pair.begin, clauseDisplayText.Length());

							{
								auto edge = CreateEdge(pair.begin, pair.end);
								edge->input.type = EdgeInputType::Rule;
								edge->input.rule = rule;
								switch (clause.field)
								{
								case Rule::Partial:
									break;
								case Rule::Discard:
									edge->insAfterInput.Add({ AstInsType::DiscardValue });
									break;
								default:
									edge->insAfterInput.Add({ AstInsType::Field,clause.field });
								}
							}

							clauseDisplayText += rule->Name();
							endPoses.Add(pair.end, clauseDisplayText.Length());
							result = pair;
							return;
						}
					}
					global.AddError(ParserErrorType::TokenOrRuleNotExistsInRule, node->name.value);
				}

				void Visit(GlrLiteralSyntax* node) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				void Visit(GlrUseSyntax* node) override
				{
					vint index = syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (index == -1)
					{
						global.AddError(ParserErrorType::TokenOrRuleNotExistsInRule, node->name.value);
						return;
					}

					auto rule = syntaxManager.Rules().Values()[index];
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					{
						auto edge = CreateEdge(pair.begin, pair.end);
						edge->input.type = EdgeInputType::Rule;
						edge->input.rule = rule;
						edge->insAfterInput.Add({ AstInsType::ReopenObject });
					}

					clauseDisplayText += L"!" + rule->Name();
					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				void Visit(GlrLoopSyntax* node) override
				{
					StatePair pair, bodyPair, delimiterPair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"{ ";
					bodyPair = Build(node->syntax);
					if (node->delimiter)
					{
						clauseDisplayText += L" ; ";
						delimiterPair = Build(node->delimiter);
					}
					clauseDisplayText += L" }";

					CreateEdge(pair.begin, bodyPair.begin);
					CreateEdge(bodyPair.end, pair.end);
					CreateEdge(pair.begin, pair.end);
					if (node->delimiter)
					{
						CreateEdge(bodyPair.end, delimiterPair.begin);
						CreateEdge(delimiterPair.end, bodyPair.begin);
					}
					else
					{
						CreateEdge(bodyPair.end, bodyPair.begin);
					}

					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"[ ";
					auto bodyPair = Build(node->syntax);
					clauseDisplayText += L" ]";

					CreateEdge(pair.begin, bodyPair.begin);
					CreateEdge(bodyPair.end, pair.end);
					CreateEdge(pair.begin, pair.end);

					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					auto firstPair = Build(node->first);
					clauseDisplayText += L" ";
					auto secondPair = Build(node->second);
					CreateEdge(firstPair.end, secondPair.begin);
					result = { firstPair.begin,secondPair.end };
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"( ";
					auto firstPair = Build(node->first);
					clauseDisplayText += L" | ";
					auto secondPair = Build(node->second);
					clauseDisplayText += L" )";

					CreateEdge(pair.begin, firstPair.begin);
					CreateEdge(firstPair.end, pair.end);
					CreateEdge(pair.begin, secondPair.begin);
					CreateEdge(secondPair.end, pair.end);

					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				void Visit(GlrCreateClause* node) override
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"< ";
					auto bodyPair = Build(node->syntax);
					clauseDisplayText += L" >";
					{
						auto edge = CreateEdge(pair.begin, bodyPair.begin);
						edge->insBeforeInput.Add({ AstInsType::BeginObject,clause.type });
					}
					{
						auto edge = CreateEdge(bodyPair.end, pair.end);
						edge->insBeforeInput.Add({ AstInsType::EndObject });
					}
					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				void Visit(GlrPartialClause* node) override
				{
					Build(node->syntax);
				}

				void Visit(Glr_ReuseClause* node) override
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"<< ";
					auto bodyPair = Build(node->syntax);
					clauseDisplayText += L" >>";
					CreateEdge(pair.begin, bodyPair.begin);
					{
						auto edge = CreateEdge(bodyPair.end, pair.end);
						edge->insBeforeInput.Add({ AstInsType::EndObject });
					}
					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}
			};

/***********************************************************************
CompileSyntax
***********************************************************************/

			void CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						if (lexerManager.Tokens().Keys().Contains(rule->name.value))
						{
							syntaxManager.Global().AddError(ParserErrorType::RuleNameConflictedWithToken, rule->name.value);
						}
						else
						{
							syntaxManager.CreateRule(rule->name.value);
						}
					}
				}
				if (syntaxManager.Global().Errors().Count() > 0) return;

				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = syntaxManager.Rules()[rule->name.value];
						CompileSyntaxVisitor visitor(astManager, lexerManager, syntaxManager, ruleSymbol);
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