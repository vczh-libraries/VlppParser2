#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
SyntaxSymbolManager::MergeEdgesWithSameInput
***********************************************************************/

			void SyntaxSymbolManager::ApplyIncrementalChange(const IncrementalChange& ic, StateList& newStates, EdgeList& newEdges)
			{
				if (ic.createdStates.Count() + ic.createdEdges.Count() > 0)
				{
					for (vint i = newEdges.Count() - 1; i >= 0; i--)
					{
						auto edge = newEdges[i];
						if (!ic.reusedEdges.Contains(edge.Obj()))
						{
							edge->From()->outEdges.Remove(edge.Obj());
							edge->To()->inEdges.Remove(edge.Obj());
							newEdges.RemoveAt(i);
						}
					}

					for (vint i = newStates.Count() - 1; i >= 0; i--)
					{
						auto state = newStates[i];
						if (!ic.reusedStates.Contains(state.Obj()))
						{
							newStates.RemoveAt(i);
						}
					}

					CopyFrom(newStates, ic.createdStates, true);
					CopyFrom(newEdges, ic.createdEdges, true);
				}
			}

/***********************************************************************
SyntaxSymbolManager::CheckIndirectLeftRecursion
***********************************************************************/

			void SyntaxSymbolManager::CheckIndirectLeftRecursion(StateSymbol* startState, collections::List<EdgeSymbol*>& accumulatedEdges)
			{
				for (auto edge : startState->OutEdges())
				{
					if (edge->input.type == EdgeInputType::Rule)
					{
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
							CheckIndirectLeftRecursion(edge->input.rule->startStates[0], accumulatedEdges);
							accumulatedEdges.RemoveAt(accumulatedEdges.Count() - 1);
						}
					}
				}
			}

/***********************************************************************
SyntaxSymbolManager::BuildCompactNFAInternal
***********************************************************************/

			void SyntaxSymbolManager::BuildCompactNFAInternal()
			{
				Array<Pair<Ptr<StateList>, Ptr<EdgeList>>> newStatesAndEdges(rules.map.Count());
				for (vint i = 0; i < newStatesAndEdges.Count(); i++)
				{
					newStatesAndEdges[i] = { Ptr(new StateList),Ptr(new EdgeList) };
				}

				for (auto [ruleSymbol, i] : indexed(rules.map.Values()))
				{
					auto&& newStates = *newStatesAndEdges[i].key.Obj();
					auto&& newEdges = *newStatesAndEdges[i].value.Obj();
					auto [startState, endState] = EliminateEpsilonEdges(ruleSymbol, newStates, newEdges);
					ruleSymbol->startStates.Clear();
					ruleSymbol->startStates.Add(startState);

					EliminateLeftRecursion(ruleSymbol, startState, endState, newStates, newEdges);
					MergeEdgesWithSameInput(ruleSymbol, startState, newStates, newEdges);
				}

				// there will be only one start state per rule after EliminateEpsilonEdges

				collections::List<EdgeSymbol*> accumulatedEdges;
				for (auto ruleSymbol : rules.map.Values())
				{
					CheckIndirectLeftRecursion(ruleSymbol->startStates[0], accumulatedEdges);
				}
				if (global.Errors().Count() > 0) return;

				auto pmCache = CreatePrefixMerge();
				for (auto [ruleSymbol, i] : indexed(rules.map.Values()))
				{
					auto&& newStates = *newStatesAndEdges[i].key.Obj();
					auto&& newEdges = *newStatesAndEdges[i].value.Obj();
					PrefixMergeCrossReference(pmCache.Obj(), ruleSymbol, ruleSymbol->startStates[0], newStates, newEdges);
				}

				states.Clear();
				edges.Clear();
				for (vint i = 0; i < newStatesAndEdges.Count(); i++)
				{
					auto&& newStates = *newStatesAndEdges[i].key.Obj();
					auto&& newEdges = *newStatesAndEdges[i].value.Obj();
					CopyFrom(states, newStates, true);
					CopyFrom(edges, newEdges, true);
				}
			}
		}
	}
}