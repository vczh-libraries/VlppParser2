#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
SyntaxSymbolManager::MergeEdgesWithSameRuleUsingLeftrec
***********************************************************************/

			void SyntaxSymbolManager::MergeEdgesWithSameRuleUsingLeftrec(RuleSymbol* rule, StateSymbol* startState, StateList& newStates, EdgeList& newEdges)
			{
				/*
				* Two edges can be merged if:
				*   They consume the same Rule, but they have different insAfterInput and competitions
				*   We only check rules because MergeEdgesWithSameInput already take care of those
				*
				* If a state has multiple outgoing edges that can be merged
				*   An edge with no insAfterInput and competitions will be inserted before these edges
				*   These edges will be converted to LeftRec
				*
				* [BEFORE]
				*    +-(r)-> U -(ending)-> X
				*    |
				* A -+-(r)-> V -(b)-> Y
				*    |
				*    +-(r)-> W -+-(c)-> Z1
				*               |
				*               +-(ending)-> Z2
				*
				* [AFTER]
				*
				*             +-(ending)-> X
				*             |
				* A -(r)-> B -+-(leftrec)-> V -(b)-> Y
				*             |
				*             +-(leftrec)-> W -(c)-> Z1
				*             |
				*             +-(ending)-> Z2
				*/

				IncrementalChange ic;
				CopyFrom(ic.opStates, From(newStates).Select([](auto p) {return p.Obj(); }));
				CopyFrom(ic.opEdges, From(newEdges).Select([](auto p) {return p.Obj(); }));

				SortedList<StateSymbol*> visitedStates;
				List<StateSymbol*> workingStates;
				workingStates.Add(startState);

				// travel through all states
				for (vint i = 0; i < workingStates.Count(); i++)
				{
					// group outgoing Rule edges by rules, ignore others
					auto currentState = workingStates[i];
					Group<RuleSymbol*, EdgeSymbol*> groupedEdges;
					for (auto edge : currentState->OutEdges())
					{
						if (edge->input.type != EdgeInputType::Rule) continue;
						groupedEdges.Add(edge->input.rule, edge);
					}

					for (vint j = 0; j < groupedEdges.Count(); j++)
					{
						// only process cases when multiple edges consume the same rule
						// their insAfterInput or competitions are different
						// otherwise they should have been merged in MergeEdgesWithSameInput
						auto&& edges = groupedEdges.GetByIndex(j);
						if (edges.Count() > 1)
						{
							// newState labeling [pm-lr] is made
							auto newState = Ptr(new StateSymbol(rule));
							ic.createdStates.Add(newState);
							newState->label = currentState->label + L"[pm-lr]";

							// currentState goes to newState by an Rule edge with empty insAfterInput and competitions
							auto newEdge = Ptr(new EdgeSymbol(currentState, newState.Obj()));
							ic.createdEdges.Add(newEdge);
							newEdge->input = edges[0]->input;

							// edit all original Rule edges
							for (auto originalRuleEdge : edges)
							{
								// disconnect originalRuleEdge from currentState
								originalRuleEdge->fromState->outEdges.Remove(originalRuleEdge);
								originalRuleEdge->fromState = nullptr;

								vint epsilonCount = 0;
								vint inputCount = 0;

								auto targetState = originalRuleEdge->toState;
								for (auto targetEdge : targetState->outEdges)
								{
									if (targetEdge->input.type == EdgeInputType::Ending || targetEdge->input.type == EdgeInputType::LeftRec)
									{
										epsilonCount++;
									}
									else
									{
										inputCount++;
									}
								}

								if (epsilonCount == 0)
								{
									// if all edges following originalRuleEdge are input edges
									// reconnect it from newState and turn it into LeftRec
									originalRuleEdge->fromState = newState.Obj();
									originalRuleEdge->fromState->outEdges.Add(originalRuleEdge);
									originalRuleEdge->input = { EdgeInputType::LeftRec };
								}
								else
								{
									// otherwise complex editing involves
									Ptr<StateSymbol> newTargetState;
									for (auto targetEdge : targetState->OutEdges())
									{
										if (targetEdge->input.type == EdgeInputType::Ending || targetEdge->input.type == EdgeInputType::LeftRec)
										{
											// for any epsilon edge following originalRuleEdge
											// we don't want two epsilon edge as it would make the NFA non-compact
											// a new edge will be made to merge originalRuleEdge and this following epsilon edge
											auto newTargetEdge = Ptr(new EdgeSymbol(newState.Obj(), targetEdge->toState));
											ic.createdEdges.Add(newTargetEdge);

											newTargetEdge->input = targetEdge->input;
											CopyFrom(newTargetEdge->competitions, originalRuleEdge->competitions, true);
											CopyFrom(newTargetEdge->competitions, targetEdge->competitions, true);
											CopyFrom(newTargetEdge->insAfterInput, originalRuleEdge->insAfterInput, true);
											CopyFrom(newTargetEdge->insAfterInput, targetEdge->insAfterInput, true);
										}
										else
										{
											// for any input edge following originalRuleEdge
											// as we can't change targetState which originalRuleEdge points to
											// because there might be other edges connecting to targetState
											// we will duplicate targetState to a [pm-dup]
											// and make originalRuleEdge connect to it from newState as a LeftRec
											if (!newTargetState)
											{
												newTargetState = Ptr(new StateSymbol(rule));
												ic.createdStates.Add(newTargetState);
												newTargetState->label = targetState->label + L"[pm-dup]";

												originalRuleEdge->fromState = newState.Obj();
												originalRuleEdge->fromState->outEdges.Add(originalRuleEdge);
												originalRuleEdge->toState->inEdges.Remove(originalRuleEdge);
												originalRuleEdge->toState = newTargetState.Obj();
												originalRuleEdge->toState->inEdges.Add(originalRuleEdge);
												originalRuleEdge->input = { EdgeInputType::LeftRec };
											}

											// and copy the input rule
											auto newTargetEdge = Ptr(new EdgeSymbol(newTargetState.Obj(), targetEdge->toState));
											ic.createdEdges.Add(newTargetEdge);

											newTargetEdge->input = targetEdge->input;
											CopyFrom(newTargetEdge->competitions, targetEdge->competitions, true);
											CopyFrom(newTargetEdge->insAfterInput, targetEdge->insAfterInput, true);
										}
									}
								}

								if (originalRuleEdge->fromState == nullptr)
								{
									// if originalRuleEdge is not reused, remove it
									originalRuleEdge->toState->inEdges.Remove(originalRuleEdge);
									ic.opEdges.Remove(originalRuleEdge);
								}

								if (targetState->inEdges.Count() == 0)
								{
									// after removing originalRuleEdge
									// if no other state points to targetState
									// remove it
									// if a [pm-dup] is made, we won't reusing and just let that happen
									ic.opStates.Remove(targetState);
									for (auto targetEdge : targetState->OutEdges())
									{
										targetEdge->toState->inEdges.Remove(targetEdge);
										ic.opEdges.Remove(targetEdge);
									}
								}
							}
						}
					}

					for (auto edge : currentState->OutEdges())
					{
						if (!visitedStates.Contains(edge->To()))
						{
							visitedStates.Add(edge->To());
							workingStates.Add(edge->To());
						}
					}
				}

				ApplyIncrementalChange(ic, newStates, newEdges);
			}
		}
	}
}