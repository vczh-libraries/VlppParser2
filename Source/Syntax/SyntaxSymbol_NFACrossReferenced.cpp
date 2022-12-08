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
							auto newEdge = Ptr(new EdgeSymbol(startState, edge->To()));
							edges.Add(newEdge);

							newEdge->input = edge->input;
							newEdge->importancy = edge->importancy;
							for (auto acc : accumulatedEdges)
							{
								CopyFrom(newEdge->insBeforeInput, acc->insBeforeInput, true);
								newEdge->returnEdges.Add(acc);
							}
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
				List<EdgeSymbol*> placeholderEdges;
				for (auto outEdge : startState->OutEdges())
				{
					if (outEdge->input.type == EdgeInputType::LrPlaceholder)
					{
						if (!From(outEdge->input.flags).Intersect(injectEdge->input.flags).IsEmpty())
						{
							placeholderEdges.Add(outEdge);
						}
					}
				}

				vint created = 0;
				for(auto placeholderEdge : From(placeholderEdges))
				{
					List<StateSymbol*> endingStates;
					List<EdgeSymbol*> returnEdges;
					{
						// check if placeholderEdge does nothing more than using rules

						if (placeholderEdge->insAfterInput.Count() > 0)
						{
							goto FAILED_INSTRUCTION_CHECKING;
						}

						for (vint i = 0; i <= placeholderEdge->returnEdges.Count(); i++)
						{
							auto returnEdge =
								i == 0
								? injectEdge
								: placeholderEdge->returnEdges[i - 1]
								;
							auto endingState =
								i == placeholderEdge->returnEdges.Count()
								? placeholderEdge->To()
								: placeholderEdge->returnEdges[i]->To()
								;

							for (auto outEdge : endingState->OutEdges())
							{
								if (outEdge->input.type == EdgeInputType::Ending && outEdge->insAfterInput.Count() > 0)
								{
									goto FAILED_INSTRUCTION_CHECKING;
								}
							}

							returnEdges.Add(returnEdge);
							endingStates.Add(endingState);
						}
						goto PASSED_INSTRUCTION_CHECKING;
					FAILED_INSTRUCTION_CHECKING:
						AddError(
							ParserErrorType::LeftRecursionPlaceholderMixedWithSwitches,
							{},
							injectEdge->fromState->Rule()->Name(),
							lrpFlags[injectEdge->input.flags[0]],
							startState->Rule()->Name()
							);
						return;
					}
				PASSED_INSTRUCTION_CHECKING:;

					// search for all possible "LrPlaceholder {Ending} LeftRec Token" transitions
					// for each transition, compact edges and put injectEdge properly in returnEdges
					// here insBeforeInput has been ensured to be:
					//   EndObject
					//   LriStore
					//   placeholderEdge->insBeforeInput
					//   DelayFieldAssignment
					//   LriFetch
					//   loop {endingEdge->insBeforeInput returnEdge->insAfterInput}
					//   --LeftRec--> ...

					// EndObject is for the ReopenObject in the use rule transition before
					// DelayFieldAssignment is for the ReopenObject in injectEdge->insAfterInput
					// injectEdge is the last returnEdge

					List<AstIns> instructionPrefix;
					// there is no instruction in injectEdge->insBeforeInput
					instructionPrefix.Add({ AstInsType::EndObject });
					instructionPrefix.Add({ AstInsType::LriStore });
					instructionPrefix.Add({ AstInsType::DelayFieldAssignment });
					CopyFrom(instructionPrefix, placeholderEdge->insBeforeInput, true);
					instructionPrefix.Add({ AstInsType::LriFetch });

					for (vint i = returnEdges.Count() - 1; i >= 0; i--)
					{
						auto endingState = endingStates[i];
						auto returnEdge = returnEdges[i];
						EdgeSymbol* endingEdge = nullptr;

						for (auto outEdge : endingState->OutEdges())
						{
							switch (outEdge->input.type)
							{
							case EdgeInputType::Ending:
								endingEdge = outEdge;
								break;
							// find if there is any LeftRec from this state
							case EdgeInputType::LeftRec:
								{
									auto lrEdge = outEdge;
									// compact everything on top of this LeftRec and create an Input
									for (auto tokenEdge : lrEdge->To()->OutEdges())
									{
										if (tokenEdge->input.type == EdgeInputType::Token)
										{
											created++;
											auto newEdge = Ptr(new EdgeSymbol(injectEdge->From(), tokenEdge->To()));
											edges.Add(newEdge);

											newEdge->input = tokenEdge->input;
											newEdge->importancy = tokenEdge->importancy;
											CopyFrom(newEdge->returnEdges, From(returnEdges).Take(i + 1), true);
											CopyFrom(newEdge->returnEdges, tokenEdge->returnEdges, true);

											// newEdge consumes a token
											// lrEdge->insAfterInput happens before consuming this token
											// so it should be copied to newEdge->insBeforeInput
											CopyFrom(newEdge->insBeforeInput, instructionPrefix, true);
											CopyFrom(newEdge->insBeforeInput, lrEdge->insBeforeInput, true);
											CopyFrom(newEdge->insBeforeInput, lrEdge->insAfterInput, true);
											CopyFrom(newEdge->insBeforeInput, tokenEdge->insBeforeInput, true);

											CopyFrom(newEdge->insAfterInput, tokenEdge->insAfterInput, true);
										}
									}
								}
								break;
							// find if there is any Token from this state
							case EdgeInputType::Token:
								{
									created++;
									auto tokenEdge = outEdge;
									auto newEdge = Ptr(new EdgeSymbol(injectEdge->From(), tokenEdge->To()));
									edges.Add(newEdge);

									newEdge->input = tokenEdge->input;
									newEdge->importancy = tokenEdge->importancy;
									CopyFrom(newEdge->returnEdges, From(returnEdges).Take(i + 1), true);
									CopyFrom(newEdge->returnEdges, tokenEdge->returnEdges, true);

									// newEdge consumes a token
									// lrEdge->insAfterInput happens before consuming this token
									// so it should be copied to newEdge->insBeforeInput
									CopyFrom(newEdge->insBeforeInput, instructionPrefix, true);
									CopyFrom(newEdge->insBeforeInput, tokenEdge->insBeforeInput, true);
									CopyFrom(newEdge->insAfterInput, tokenEdge->insAfterInput, true);
								}
								break;
							default:;
							}
						}

						if (endingEdge)
						{
							CopyFrom(instructionPrefix, endingEdge->insBeforeInput, true);
							CopyFrom(instructionPrefix, returnEdge->insAfterInput, true);
						}
						else
						{
							break;
						}
					}
				}

				// report an error if nothing is created
				if (created == 0)
				{
					AddError(
						ParserErrorType::LeftRecursionInjectHasNoContinuation,
						{},
						injectEdge->fromState->Rule()->Name(),
						lrpFlags[injectEdge->input.flags[0]],
						startState->Rule()->Name()
						);
				}
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