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
CompileSyntaxVisitor
***********************************************************************/

			class CompileSyntaxVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				struct StatePair
				{
					StateSymbol* begin;
					StateSymbol* end;
				};

				using StatePosMap = Dictionary<StateSymbol*, vint>;
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				AstClassSymbol*				clauseType;

				WString						clauseDisplayText;
				StatePosMap					startPoses;
				StatePosMap					endPoses;
				StatePair					result;

				StateSymbol* CreateState()
				{
					return ruleSymbol->Owner()->CreateState(ruleSymbol, ruleSymbol->CurrentClauseId());
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
				CompileSyntaxVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void AssignClause(const Ptr<GlrClause>& node)
				{
					ruleSymbol->NewClause();

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

			protected:
				void Visit(GlrRefSyntax* node) override
				{
					{
						vint index = context.lexerManager.TokenOrder().IndexOf(node->name.value);
						if (index != -1)
						{
							auto token = context.lexerManager.Tokens()[node->name.value];
							StatePair pair;
							pair.begin = CreateState();
							pair.end = CreateState();
							startPoses.Add(pair.begin, clauseDisplayText.Length());

							{
								auto edge = CreateEdge(pair.begin, pair.end);
								edge->input.type = EdgeInputType::Token;
								edge->input.token = (vint32_t)index;
								if (node->field)
								{
									auto propSymbol = FindPropSymbol(clauseType, node->field.value);
									edge->insAfterInput.Add({ AstInsType::Token });
									edge->insAfterInput.Add({ AstInsType::Field,context.output->fieldIds[propSymbol] });
								}
							}

							clauseDisplayText += token->displayText == L"" ? token->Name() : L"\"" + token->displayText + L"\"";
							endPoses.Add(pair.end, clauseDisplayText.Length());
							result = pair;
							return;
						}
					}
					{
						vint index = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
						if (index != -1)
						{
							auto rule = context.syntaxManager.Rules().Values()[index];
							StatePair pair;
							pair.begin = CreateState();
							pair.end = CreateState();
							startPoses.Add(pair.begin, clauseDisplayText.Length());

							{
								auto edge = CreateEdge(pair.begin, pair.end);
								edge->input.type = EdgeInputType::Rule;
								edge->input.rule = rule;
								if (node->field)
								{
									auto propSymbol = FindPropSymbol(clauseType, node->field.value);
									edge->insAfterInput.Add({ AstInsType::Field,context.output->fieldIds[propSymbol] });
								}
								else if (!rule->isPartial)
								{
									edge->insAfterInput.Add({ AstInsType::DiscardValue });
								}
							}

							clauseDisplayText += rule->Name();
							endPoses.Add(pair.end, clauseDisplayText.Length());
							result = pair;
							return;
						}
					}
					CHECK_FAIL(L"Should not reach here!");
				}

				void Visit(GlrLiteralSyntax* node) override
				{
					vint index = context.literalTokens[node];
					auto token = context.lexerManager.Tokens()[context.lexerManager.TokenOrder()[index]];
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					{
						auto edge = CreateEdge(pair.begin, pair.end);
						edge->input.type = EdgeInputType::Token;
						edge->input.token = context.literalTokens[node];
					}

					clauseDisplayText += token->displayText == L"" ? token->Name() : L"\"" + token->displayText + L"\"";
					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				void Visit(GlrUseSyntax* node) override
				{
					vint index = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (index == -1)
					{
						context.syntaxManager.AddError(
							ParserErrorType::TokenOrRuleNotExistsInRule,
							node->codeRange,
							node->name.value
							);
						return;
					}

					auto rule = context.syntaxManager.Rules().Values()[index];
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

					switch (node->priority)
					{
					case GlrOptionalPriority::Equal:
						clauseDisplayText += L"[ ";
						break;
					case GlrOptionalPriority::PreferTake:
						clauseDisplayText += L"+[ ";
						break;
					case GlrOptionalPriority::PreferSkip:
						clauseDisplayText += L"-[ ";
						break;
					}
					auto bodyPair = Build(node->syntax);
					clauseDisplayText += L" ]";

					auto takeEdge = CreateEdge(pair.begin, bodyPair.begin);
					CreateEdge(bodyPair.end, pair.end);
					auto skipEdge = CreateEdge(pair.begin, pair.end);

					if (node->priority == GlrOptionalPriority::PreferTake)
					{
						takeEdge->important = true;
					}
					if (node->priority == GlrOptionalPriority::PreferSkip)
					{
						skipEdge->important = true;
					}

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

				StatePair Visit(StatePair pair, GlrAssignment* node)
				{
					auto withState = CreateState();
					auto edge = CreateEdge(pair.end, withState);

					auto propSymbol = FindPropSymbol(clauseType, node->field.value);
					auto enumSymbol = dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol);
					edge->insBeforeInput.Add({ AstInsType::EnumItem,(vint32_t)enumSymbol->ItemOrder().IndexOf(node->value.value) });
					edge->insBeforeInput.Add({ AstInsType::Field,context.output->fieldIds[propSymbol] });

					endPoses.Add(withState, clauseDisplayText.Length());
					return { pair.begin,withState };
				}

				void Visit(GlrCreateClause* node) override
				{
					clauseType = context.clauseTypes[node];
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"< ";
					auto bodyPair = Build(node->syntax);
					for (auto assignment : node->assignments)
					{
						bodyPair = Visit(bodyPair, assignment.Obj());
					}
					clauseDisplayText += L" >";
					{
						auto classSymbol = dynamic_cast<AstClassSymbol*>(context.astManager.Symbols()[node->type.value]);
						auto edge = CreateEdge(pair.begin, bodyPair.begin);
						edge->insBeforeInput.Add({ AstInsType::BeginObject,context.output->classIds[classSymbol] });
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
					clauseType = context.clauseTypes[node];
					auto bodyPair = Build(node->syntax);
					for (auto assignment : node->assignments)
					{
						bodyPair = Visit(bodyPair, assignment.Obj());
					}
					result = bodyPair;
				}

				void Visit(GlrReuseClause* node) override
				{
					clauseType = context.clauseTypes[node];
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"<< ";
					auto bodyPair = Build(node->syntax);
					for (auto assignment : node->assignments)
					{
						bodyPair = Visit(bodyPair, assignment.Obj());
					}
					clauseDisplayText += L" >>";
					{
						auto edge = CreateEdge(pair.begin, bodyPair.begin);
						edge->insBeforeInput.Add({ AstInsType::DelayFieldAssignment });
					}
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