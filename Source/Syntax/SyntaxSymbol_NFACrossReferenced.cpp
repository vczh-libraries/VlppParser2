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
				// search for all qualified placeholder edge starts from inject targets
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

				// calculate all return edges and ending states for each placeholder edge
				// return edges:
				//   injectEdge
				//   return edge begins from inject target
				//   ...
				//   return edge begins from most inner rule that contains placeholder
				// ending states:
				//   to state of return edge begins from inject target
				//   ...
				//   to state of return edge begins from most inner rule that contains placeholder
				//   to state from placeholder edge
				Array<List<StateSymbol*>> endingStatesArray(placeholderEdges.Count());
				Array<List<EdgeSymbol*>> returnEdgesArray(placeholderEdges.Count());
				for(auto [placeholderEdge, index] : indexed(placeholderEdges))
				{
					auto& endingStates = endingStatesArray[index];
					auto& returnEdges = returnEdgesArray[index];

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
					continue;
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

				// calculate all acceptable Token input from inject edge
				// key:
				//   token
				//   the number of return edges carried into this edge, at least 1
				// value:
				//   index of placeholder edge
				//   the LeftRec edge before the Token edge (optional)
				//   the Token edge that consume this input
				using InputKey = Pair<vint32_t, vint>;
				using InputValue = Tuple<vint, EdgeSymbol*, EdgeSymbol*>;
				Group<InputKey, InputValue> acceptableInputs;

				// calculate all acceptable Ending input from inject edge
				// value:
				//   index of placeholder edge
				//   the additional Ending edge
				using EndingInputValue = Pair<vint, EdgeSymbol*>;
				List<EndingInputValue> acceptableEndingInputs;

				for(auto [placeholderEdge, index] : indexed(placeholderEdges))
				{
					auto& endingStates = endingStatesArray[index];
					auto& returnEdges = returnEdgesArray[index];

					// TODO: (enumerable) foreach:indexed(reversed)
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
								if (returnEdge == injectEdge)
								{
									// all possible Ending input are comsumed
									// an ending edge should be created if
									//   the injection edge cannot be skipped
									//   the injection edge to state has Ending input
									// an optional injection can be skipped
									// a non-optional injection can be skipped by surrounding syntax

									auto endingEdgeAfterInject = From(injectEdge->To()->OutEdges())
										.Where([](auto edge) { return edge->input.type == EdgeInputType::Ending; })
										.First(nullptr);
									if (endingEdgeAfterInject)
									{
										// find if there is a state in the same rule that looks like:
										//      +-------------------(ending)---------------------+
										//      |                                                |
										//   S -+                                                +->E
										//      |                                                |
										//      +-{-(leftrec)-> X} -(lri:ThisEdge)-> T -(ending)-+
										// or
										//      +-(same rule)-> Y -----------------(ending)------------------+
										//      |                                                            |
										//   S -+                                                            +->E
										//      |                                                            |
										//      +-(same rule)-{-(leftrec)-> X} -(lri:ThisEdge)-> T -(ending)-+

										List<StateSymbol*> visiting;
										SortedList<StateSymbol*> visited;
										visiting.Add(injectEdge->From());

										// TODO: (enumerable) visiting/visited
										for (vint i = 0; i < visiting.Count(); i++)
										{
											auto visitingState = visiting[i];
											if (visited.Contains(visitingState)) continue;
											visited.Add(visitingState);

											for (auto siblingEdge : visitingState->OutEdges())
											{
												if (siblingEdge->input.type == EdgeInputType::Ending)
												{
													goto SKIP_SEARCHING;
												}
											}

											for (auto commingEdge : visitingState->InEdges())
											{
												if (commingEdge->From()->Rule() != injectEdge->From()->Rule()) continue;

												switch (commingEdge->input.type)
												{
												case EdgeInputType::LeftRec:
													visiting.Add(commingEdge->From());
													break;
												case EdgeInputType::Rule:
													{
														auto expectedReturnRule = commingEdge->input.rule;
														for (auto siblingCommingEdge : commingEdge->From()->OutEdges())
														{
															if (siblingCommingEdge == commingEdge) continue;
															if (siblingCommingEdge->input.type != EdgeInputType::Rule) continue;
															if (siblingCommingEdge->input.rule != expectedReturnRule) continue;

															if (!From(siblingCommingEdge->To()->OutEdges())
																.Where([](auto edge) { return edge->input.type == EdgeInputType::Ending; })
																.IsEmpty())
															{
																goto SKIP_SEARCHING;
															}
														}
													}
													break;
												default:;
												}
											}
										}

										{
											// if there is no such state
											// an Ending edge is needed
											acceptableEndingInputs.Add({ index,endingEdgeAfterInject });
										}
									SKIP_SEARCHING:;
									}
								}
								break;
							// find if there is any LeftRec from this state
							case EdgeInputType::LeftRec:
								{
									// compact everything on top of this LeftRec and create an Input
									for (auto tokenEdge : outEdge->To()->OutEdges())
									{
										if (tokenEdge->input.type == EdgeInputType::Token)
										{
											acceptableInputs.Add({ tokenEdge->input.token,i + 1 }, { index,outEdge,tokenEdge });
										}
									}
								}
								break;
							// find if there is any Token from this state
							case EdgeInputType::Token:
								acceptableInputs.Add({ outEdge->input.token,i + 1 }, { index,nullptr,outEdge });
								break;
							default:;
							}
						}

						if (!endingEdge)
						{
							// stop searching if there is no Ending input
							// since there will be no more {Ending} Token edge to compact
							break;
						}
					}
				}

				auto prepareLriEdgeInstructions = [&](vint placeholderIndex, vint returnEdgeCount, List<AstIns>& instructionPrefix)
				{
					auto placeholderEdge = placeholderEdges[placeholderIndex];
					auto& endingStates = endingStatesArray[placeholderIndex];
					auto& returnEdges = returnEdgesArray[placeholderIndex];

					// search for all possible "LrPlaceholder {Ending} LeftRec Token" transitions
					// for each transition, compact edges and put injectEdge properly in returnEdges
					// here insBeforeInput has been ensured to be:
					//   EndObject
					//   LriStore
					//   DelayFieldAssignment
					//   placeholderEdge->insBeforeInput
					//   LriFetch
					//   loop {endingEdge->insBeforeInput returnEdge->insAfterInput}
					//   --LeftRec--> ...

					// EndObject is for the ReopenObject in the use rule transition before
					// DelayFieldAssignment is for the ReopenObject in injectEdge->insAfterInput
					// injectEdge is the last returnEdge

					// there is no instruction in injectEdge->insBeforeInput
					instructionPrefix.Add({ AstInsType::EndObject });
					instructionPrefix.Add({ AstInsType::LriStore });
					instructionPrefix.Add({ AstInsType::DelayFieldAssignment });
					CopyFrom(instructionPrefix, placeholderEdge->insBeforeInput, true);
					instructionPrefix.Add({ AstInsType::LriFetch });

					// TODO: (enumerable) foreach:reversed
					for (vint i = returnEdges.Count() - 1; i >= returnEdgeCount; i--)
					{
						auto endingState = endingStates[i];
						auto returnEdge = returnEdges[i];
						auto endingEdge = From(endingState->OutEdges())
							.Where([](auto edge) { return edge->input.type == EdgeInputType::Ending; })
							.First();

						CopyFrom(instructionPrefix, endingEdge->insBeforeInput, true);
						CopyFrom(instructionPrefix, returnEdge->insAfterInput, true);
					}
				};

				// TODO: (enumerable) foreach on group
				for (auto [input, inputIndex] : indexed(acceptableInputs.Keys()))
				{
					auto [inputToken, returnEdgeCount] = input;
					auto&& placeholderRecords = acceptableInputs.GetByIndex(inputIndex);

					// group inputs by lrEdge, tokenEdge, carried return edges
					// if there are multiple inputs from the same key
					// it means such input creates an ambiguity
					// 
					// it usually happens in clauses like Prefix lri(flags) Target
					// where multiple valid flags found in Target at the same time
					//
					// if we could indentify some inputs here where excluded return edges are all reuse edges
					// then we can only create edges for one of them

					struct Entry
					{
						EdgeSymbol*				lrEdge;
						EdgeSymbol*				tokenEdge;
						List<EdgeSymbol*>*		returnEdges;
						vint					returnEdgeCount;

						std::strong_ordering operator<=>(const Entry& entry)const
						{
							std::strong_ordering
							result = lrEdge <=> entry.lrEdge; if (result != 0) return result;
							result = tokenEdge <=> entry.tokenEdge; if (result != 0) return result;
							return CompareEnumerable(
								From(*returnEdges).Take(returnEdgeCount),
								From(*entry.returnEdges).Take(returnEdgeCount)
								);
						}

						bool operator==(const Entry& entry) const { return (*this <=> entry) == 0; }
					};
					Group<Entry, vint> simpleUseRecords;

					// search for placeholder edges where their excluded return edges are all reuse edges
					// TODO: (enumerable) foreach on group
					for (vint recordIndex = 0; recordIndex < placeholderRecords.Count(); recordIndex++)
					{
						auto [placeholderIndex, lrEdge, tokenEdge] = placeholderRecords[recordIndex];
						Entry entry{ lrEdge,tokenEdge,&returnEdgesArray[placeholderIndex],returnEdgeCount };
						if(From(*entry.returnEdges)
							.Skip(returnEdgeCount)
							.All([](EdgeSymbol* edge) {return edge->input.ruleType == automaton::ReturnRuleType::Reuse; })
							)
						{
							simpleUseRecords.Add(entry, recordIndex);
						}
					}

					// for each group, if there are more than one placeholder edges
					// mark them as deleted except the first one
					SortedList<vint> recordsToRemove;
					// TODO: (enumerable) foreach on group
					for (vint recordGroup = 0; recordGroup < simpleUseRecords.Count(); recordGroup++)
					{
						auto&& records = simpleUseRecords.GetByIndex(recordGroup);
						if (records.Count() > 1)
						{
							CopyFrom(recordsToRemove, From(records).Skip(1));
						}
					}

					// delete them in Group
					// this way is not recommended but the group is going to be discarded very soon
					// TODO: (enumerable) foreach:reversed
					for (vint i = recordsToRemove.Count() - 1; i >= 0; i--)
					{
						const_cast<List<InputValue>&>(placeholderRecords).RemoveAt(recordsToRemove[i]);
					}

					// convert reminaings
					for (auto [placeholderIndex, lrEdge, tokenEdge] : placeholderRecords)
					{
						auto& returnEdges = returnEdgesArray[placeholderIndex];

						auto newEdge = Ptr(new EdgeSymbol(injectEdge->From(), tokenEdge->To()));
						edges.Add(newEdge);
						newEdge->input = tokenEdge->input;
						newEdge->importancy = tokenEdge->importancy;

						CopyFrom(newEdge->returnEdges, From(returnEdges).Take(returnEdgeCount), true);
						CopyFrom(newEdge->returnEdges, tokenEdge->returnEdges, true);
						prepareLriEdgeInstructions(placeholderIndex, returnEdgeCount, newEdge->insBeforeInput);
						if (lrEdge)
						{
							// newEdge consumes a token
							// lrEdge->insAfterInput happens before consuming this token
							// so it should be copied to newEdge->insBeforeInput
							CopyFrom(newEdge->insBeforeInput, lrEdge->insBeforeInput, true);
							CopyFrom(newEdge->insBeforeInput, lrEdge->insAfterInput, true);
						}
						CopyFrom(newEdge->insBeforeInput, tokenEdge->insBeforeInput, true);
						CopyFrom(newEdge->insAfterInput, tokenEdge->insAfterInput, true);
					}
				}

				for (auto [placeholderIndex, endingEdgeAfterInject] : acceptableEndingInputs)
				{
				}

				// report an error if nothing is created
				if (acceptableInputs.Count() == 0 && acceptableEndingInputs.Count() == 0)
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