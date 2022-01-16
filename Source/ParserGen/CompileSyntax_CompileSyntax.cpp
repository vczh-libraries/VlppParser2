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
AutomatonBuilder
***********************************************************************/

			class AutomatonBuilder
			{
			protected:
				struct StatePair
				{
					StateSymbol* begin;
					StateSymbol* end;
				};

				using StatePosMap = Dictionary<StateSymbol*, vint>;
				using StateBuilder = Func<StatePair()>;
				using AssignmentBuilder = Func<StatePair(StatePair)>;
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;

				WString						clauseDisplayText;
				StatePosMap					startPoses;
				StatePosMap					endPoses;

				StateSymbol* CreateState()
				{
					return ruleSymbol->Owner()->CreateState(ruleSymbol, ruleSymbol->CurrentClauseId());
				}

				EdgeSymbol* CreateEdge(StateSymbol* from, StateSymbol* to)
				{
					return ruleSymbol->Owner()->CreateEdge(from, to);
				}
			public:
				AutomatonBuilder(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				////////////////////////////////////////////////////////
				// Syntax
				////////////////////////////////////////////////////////

				StatePair BuildTokenSyntax(vint32_t tokenId, const WString& displayText, vint32_t field)
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					{
						auto edge = CreateEdge(pair.begin, pair.end);
						edge->input.type = EdgeInputType::Token;
						edge->input.token = tokenId;
						if (field != -1)
						{
							edge->insAfterInput.Add({ AstInsType::Token });
							edge->insAfterInput.Add({ AstInsType::Field,field });
						}
					}

					clauseDisplayText += displayText;
					endPoses.Add(pair.end, clauseDisplayText.Length());
					return pair;
				}

				StatePair BuildRuleSyntax(RuleSymbol* rule, vint32_t field)
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					{
						auto edge = CreateEdge(pair.begin, pair.end);
						edge->input.type = EdgeInputType::Rule;
						edge->input.rule = rule;
						if (field != -1)
						{
							edge->insAfterInput.Add({ AstInsType::Field,field });
						}
						else if (!rule->isPartial)
						{
							edge->insAfterInput.Add({ AstInsType::DiscardValue });
						}
					}

					clauseDisplayText += rule->Name();
					endPoses.Add(pair.end, clauseDisplayText.Length());
					return pair;
				}

				StatePair BuildUseSyntax(RuleSymbol* rule)
				{
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
					return pair;
				}

				StatePair BuildLoopSyntax(const StateBuilder& loopBody, const StateBuilder& loopDelimiter, bool hasDelimiter)
				{
					StatePair pair, bodyPair, delimiterPair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"{ ";
					bodyPair = loopBody();
					if (hasDelimiter)
					{
						clauseDisplayText += L" ; ";
						delimiterPair = loopDelimiter();
					}
					clauseDisplayText += L" }";

					CreateEdge(pair.begin, bodyPair.begin);
					CreateEdge(bodyPair.end, pair.end);
					CreateEdge(pair.begin, pair.end);
					if (hasDelimiter)
					{
						CreateEdge(bodyPair.end, delimiterPair.begin);
						CreateEdge(delimiterPair.end, bodyPair.begin);
					}
					else
					{
						CreateEdge(bodyPair.end, bodyPair.begin);
					}

					endPoses.Add(pair.end, clauseDisplayText.Length());
					return pair;
				}

				StatePair BuildOptionalSyntax(GlrOptionalPriority priority, const StateBuilder& optionalBody)
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					switch (priority)
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
					default:;
					}
					auto bodyPair = optionalBody();
					clauseDisplayText += L" ]";

					auto takeEdge = CreateEdge(pair.begin, bodyPair.begin);
					CreateEdge(bodyPair.end, pair.end);
					auto skipEdge = CreateEdge(pair.begin, pair.end);

					if (priority == GlrOptionalPriority::PreferTake)
					{
						takeEdge->important = true;
					}
					if (priority == GlrOptionalPriority::PreferSkip)
					{
						skipEdge->important = true;
					}

					endPoses.Add(pair.end, clauseDisplayText.Length());
					return pair;
				}

				StatePair BuildSequenceSyntax(const StateBuilder& firstSequence, const StateBuilder& secondSequence)
				{
					auto firstPair = firstSequence();
					clauseDisplayText += L" ";
					auto secondPair = secondSequence();
					CreateEdge(firstPair.end, secondPair.begin);
					return { firstPair.begin,secondPair.end };
				}

				StatePair BuildAlternativeSyntax(const StateBuilder& firstBranch, const StateBuilder& secondBranch)
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"( ";
					auto firstPair = firstBranch();
					clauseDisplayText += L" | ";
					auto secondPair = secondBranch();
					clauseDisplayText += L" )";

					CreateEdge(pair.begin, firstPair.begin);
					CreateEdge(firstPair.end, pair.end);
					CreateEdge(pair.begin, secondPair.begin);
					CreateEdge(secondPair.end, pair.end);

					endPoses.Add(pair.end, clauseDisplayText.Length());
					return pair;
				}

				////////////////////////////////////////////////////////
				// Clauses
				////////////////////////////////////////////////////////

				StatePair BuildClause(const StateBuilder& compileSyntax)
				{
					ruleSymbol->NewClause();
					clauseDisplayText = L"";
					startPoses.Clear();
					endPoses.Clear();

					auto pair = compileSyntax();

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

					return pair;
				}

				StatePair BuildAssignment(StatePair pair, vint32_t enumItem, vint32_t field)
				{
					auto withState = CreateState();
					auto edge = CreateEdge(pair.end, withState);
					edge->insBeforeInput.Add({ AstInsType::EnumItem,enumItem });
					edge->insBeforeInput.Add({ AstInsType::Field,field });
					endPoses.Add(withState, clauseDisplayText.Length());
					return { pair.begin,withState };
				}

				StatePair BuildCreateClause(AstClassSymbol* clauseType, const StateBuilder& compileSyntax, const AssignmentBuilder& compileAssignments)
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"< ";
					auto bodyPair = compileAssignments(compileSyntax());
					clauseDisplayText += L" >";
					{
						auto edge = CreateEdge(pair.begin, bodyPair.begin);
						edge->insBeforeInput.Add({ AstInsType::BeginObject,context.output->classIds[clauseType] });
					}
					{
						auto edge = CreateEdge(bodyPair.end, pair.end);
						edge->insBeforeInput.Add({ AstInsType::EndObject });
					}
					endPoses.Add(pair.end, clauseDisplayText.Length());
					return pair;
				}

				StatePair BuildPartialClause(const StateBuilder& compileSyntax, const AssignmentBuilder& compileAssignments)
				{
					return compileAssignments(compileSyntax());
				}

				StatePair BuildReuseClause(const StateBuilder& compileSyntax, const AssignmentBuilder& compileAssignments)
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"<< ";
					auto bodyPair = compileAssignments(compileSyntax());
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
					return pair;
				}
			};

/***********************************************************************
CompileSyntaxVisitor
***********************************************************************/

			class CompileSyntaxVisitor
				: public Object
				, protected AutomatonBuilder
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				AstClassSymbol*				clauseType;
				StatePair					result;

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
					: AutomatonBuilder(_context, _ruleSymbol)
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
					{
						vint index = context.lexerManager.TokenOrder().IndexOf(node->name.value);
						if (index != -1)
						{
							auto token = context.lexerManager.Tokens()[node->name.value];
							auto displayText = token->displayText == L"" ? token->Name() : L"\"" + token->displayText + L"\"";
							result = BuildTokenSyntax((vint32_t)index, displayText, field);
							return;
						}
					}
					{
						vint index = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
						if (index != -1)
						{
							auto rule = context.syntaxManager.Rules().Values()[index];
							result = BuildRuleSyntax(rule, field);
							return;
						}
					}
					CHECK_FAIL(L"Should not reach here!");
				}

				void Visit(GlrLiteralSyntax* node) override
				{
					vint index = context.literalTokens[node];
					auto token = context.lexerManager.Tokens()[context.lexerManager.TokenOrder()[index]];
					auto displayText = token->displayText == L"" ? token->Name() : L"\"" + token->displayText + L"\"";
					result = BuildTokenSyntax((vint32_t)index, displayText, -1);
				}

				void Visit(GlrUseSyntax* node) override
				{
					auto rule = context.syntaxManager.Rules()[node->name.value];
					result = BuildUseSyntax(rule);
				}

				void Visit(GlrLoopSyntax* node) override
				{
					result = BuildLoopSyntax(
						[this, node]() { return Build(node->syntax); },
						[this, node]() { return Build(node->delimiter); },
						node->delimiter
						);
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					result = BuildOptionalSyntax(
						node->priority,
						[this, node]() { return Build(node->syntax); }
						);
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					result = BuildSequenceSyntax(
						[this, node]() { return Build(node->first); },
						[this, node]() { return Build(node->second); }
						);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					result = BuildAlternativeSyntax(
						[this, node]() { return Build(node->first); },
						[this, node]() { return Build(node->second); }
						);
				}

				StatePair CompileAssignments(StatePair pair, List<Ptr<GlrAssignment>>& assignments)
				{
					for (auto node : assignments)
					{
						auto propSymbol = FindPropSymbol(clauseType, node->field.value);
						auto enumSymbol = dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol);
						auto enumItem = (vint32_t)enumSymbol->ItemOrder().IndexOf(node->value.value);
						auto field = context.output->fieldIds[propSymbol];
						pair = BuildAssignment(pair, enumItem, field);
					}
					return pair;
				}

				void Visit(GlrCreateClause* node) override
				{
					clauseType = context.clauseTypes[node];
					result = BuildClause([this, node]()
					{
						return BuildCreateClause(
							clauseType,
							[this, node]() { return Build(node->syntax); },
							[this, node](StatePair pair) { return CompileAssignments(pair, node->assignments); }
							);
					});
				}

				void Visit(GlrPartialClause* node) override
				{
					clauseType = context.clauseTypes[node];
					result = BuildClause([this, node]()
					{
						return BuildPartialClause(
							[this, node]() { return Build(node->syntax); },
							[this, node](StatePair pair) { return CompileAssignments(pair, node->assignments); }
							);
					});
				}

				void Visit(GlrReuseClause* node) override
				{
					clauseType = context.clauseTypes[node];
					result = BuildClause([this, node]()
					{
						return BuildReuseClause(
							[this, node]() { return Build(node->syntax); },
							[this, node](StatePair pair) { return CompileAssignments(pair, node->assignments); }
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