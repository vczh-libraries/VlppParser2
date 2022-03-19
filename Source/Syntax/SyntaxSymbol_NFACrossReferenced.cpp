#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
SyntaxSymbolManager::FixCrossReferencedRuleEdge
***********************************************************************/

			void SyntaxSymbolManager::FixCrossReferencedRuleEdge(StateSymbol* startState, collections::Group<StateSymbol*, EdgeSymbol*>& orderedEdges, collections::List<EdgeSymbol*>& accumulatedEdges)
			{
				auto lastEdge = accumulatedEdges[accumulatedEdges.Count() - 1];
				auto lastRule = lastEdge->input.rule;
				auto ruleBegin = lastRule->startStates[0];
				vint index = orderedEdges.Keys().IndexOf(ruleBegin);
				if (index == -1) return;

				for (auto edge : orderedEdges.GetByIndex(index))
				{
					switch (edge->input.type)
					{
					case EdgeInputType::Token:
					case EdgeInputType::LrPlaceholder:
						// multiple Rule edges followed by one Token or LrPlaceholder edge create a cross-referenced edge
						if (edge->returnEdges.Count() == 0)
						{
							auto newEdge = new EdgeSymbol(startState, edge->To());
							edges.Add(newEdge);

							newEdge->input = edge->input;
							newEdge->importancy = edge->importancy;
							for (auto acc : accumulatedEdges)
							{
								CopyFrom(newEdge->insSwitch, acc->insSwitch, true);
								CopyFrom(newEdge->insBeforeInput, acc->insBeforeInput, true);
								newEdge->returnEdges.Add(acc);
							}
							CopyFrom(newEdge->insSwitch, edge->insSwitch, true);
							CopyFrom(newEdge->insBeforeInput, edge->insBeforeInput, true);
							CopyFrom(newEdge->insAfterInput, edge->insAfterInput, true);
						}
						break;
					case EdgeInputType::Rule:
						if (accumulatedEdges.Contains(edge))
						{
							AddError(
								ParserErrorType::RuleIsIndirectlyLeftRecursive,
								{},
								edge->input.rule->Name()
								);
						}
						else
						{
							accumulatedEdges.Add(edge);
							FixCrossReferencedRuleEdge(startState, orderedEdges, accumulatedEdges);
							accumulatedEdges.RemoveAt(accumulatedEdges.Count() - 1);
						}
						break;
					case EdgeInputType::Epsilon:
					case EdgeInputType::Ending:
					case EdgeInputType::LeftRec:
						// Epsilon edges do not exist in compact-NFA
						// Ending and LeftRec edges are not involved
						break;
					case EdgeInputType::LrInject:
						CHECK_FAIL(L"<BuildCrossReferencedNFAInternal>LrInject is impossible from a start state");
						break;
					default:
						CHECK_FAIL(L"<BuildCrossReferencedNFAInternal>Unhandled!");
					}
				}
			}

/***********************************************************************
SyntaxSymbolManager::FixLeftRecursionInjectEdge
***********************************************************************/

			void SyntaxSymbolManager::FixLeftRecursionInjectEdge(StateSymbol* startState, EdgeSymbol* injectEdge)
			{
				EdgeSymbol* lrpEdge = nullptr;
				for (auto outEdge : startState->OutEdges())
				{
					if (outEdge->input.type == EdgeInputType::LrPlaceholder && outEdge->input.token == injectEdge->input.token)
					{
						if (lrpEdge)
						{
							AddError(
								ParserErrorType::LeftRecursionPlaceholderNotUnique,
								{},
								injectEdge->fromState->Rule()->Name(),
								lrpFlags[injectEdge->input.token],
								startState->Rule()->Name()
								);
							return;
						}
						else
						{
							lrpEdge = outEdge;
						}
					}
				}

				if (!lrpEdge)
				{
					AddError(
						ParserErrorType::LeftRecursionPlaceholderNotFoundInRule,
						{},
						injectEdge->fromState->Rule()->Name(),
						lrpFlags[injectEdge->input.token],
						startState->Rule()->Name()
						);
					return;
				}

				{
					// check if injectEdge does nothing more than using rules

					if (injectEdge->insSwitch.Count() > 0)
					{
						goto FAILED_INSTRUCTION_CHECKING;
					}
					if (injectEdge->insAfterInput.Count() > 0)
					{
						goto FAILED_INSTRUCTION_CHECKING;
					}

					for (auto ins : injectEdge->insBeforeInput)
					{
						if (ins.type != AstInsType::DelayFieldAssignment)
						{
							goto FAILED_INSTRUCTION_CHECKING;
						}
					}

					for (auto returnEdge : injectEdge->returnEdges)
					{
						switch (returnEdge->insAfterInput.Count())
						{
						case 0:
							break;
						case 1:
							if (returnEdge->insAfterInput[0].type != AstInsType::ReopenObject)
							{
								goto FAILED_INSTRUCTION_CHECKING;
							}
							break;
						default:
							goto FAILED_INSTRUCTION_CHECKING;
						}
					}
					goto PASSED_INSTRUCTION_CHECKING;
				FAILED_INSTRUCTION_CHECKING:
					AddError(
						ParserErrorType::LeftRecursionInjectIntoNonLeftRecursiveRule,
						{},
						injectEdge->fromState->Rule()->Name(),
						lrpFlags[injectEdge->input.token],
						startState->Rule()->Name()
						);
					return;
				}
			PASSED_INSTRUCTION_CHECKING:;

				// search for all possible "LrPlaceholder {Ending} LeftRec Token" transitions
				// for each transition, compact edges and put injectEdge properly in returnEdges
				CHECK_FAIL(L"<BuildCrossReferencedNFAInternal>Unhandled!");
			}

/***********************************************************************
SyntaxSymbolManager::BuildCrossReferencedNFAInternal
***********************************************************************/

			void SyntaxSymbolManager::BuildCrossReferencedNFAInternal()
			{
				List<StateSymbol*> states;
				GetStatesInStableOrder(states);

				Group<StateSymbol*, EdgeSymbol*> orderedEdges;
				for (auto state : states)
				{
					List<EdgeSymbol*> edges;
					state->GetOutEdgesInStableOrder(states, edges);
					for (auto edge : edges)
					{
						orderedEdges.Add(state, edge);
					}
				}

				// compact multiple shift (Rule) edges and an input (Token or LrPlaceholder) edge to one edge
				// when a cross-referenced edge is executed, for each Rule edge:
				//   insSwitch instructions are executed
				//   insBeforeInput instructions are executed
				//   insAfterInput instructions are executed in its returnEdges
				for (auto state : states)
				{
					vint index = orderedEdges.Keys().IndexOf(state);
					if (index != -1)
					{
						for (auto edge : orderedEdges.GetByIndex(index))
						{
							if (edge->input.type == EdgeInputType::Rule)
							{
								List<EdgeSymbol*> accumulatedEdges;
								accumulatedEdges.Add(edge);
								FixCrossReferencedRuleEdge(edge->From(), orderedEdges, accumulatedEdges);
							}
						}
					}
				}

				// convert LrInject to Token
				for (auto state : states)
				{
					vint index = orderedEdges.Keys().IndexOf(state);
					if (index != -1)
					{
						for (auto edge : orderedEdges.GetByIndex(index))
						{
							if (edge->input.type == EdgeInputType::LrInject)
							{
								auto startState = edge->input.rule->startStates[0];
								FixLeftRecursionInjectEdge(startState, edge);
							}
						}
					}
				}
			}
		}
	}
}