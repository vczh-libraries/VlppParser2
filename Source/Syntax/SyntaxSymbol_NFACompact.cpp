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
							if (edge->fromState) edge->fromState->outEdges.Remove(edge.Obj());
							if (edge->toState) edge->toState->inEdges.Remove(edge.Obj());
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
					// remove all epsilon edges, potentially duplicating input edges
					auto [startState, endState] = EliminateEpsilonEdges(ruleSymbol, newStates, newEdges);
					ruleSymbol->startStates.Clear();
					ruleSymbol->startStates.Add(startState);

					// there will be only one start state per rule after EliminateEpsilonEdges
					// detect and resolve direct left recursion
					EliminateLeftRecursion(ruleSymbol, startState, endState, newStates, newEdges);

					// merge as many input edges as possible to reduce wasted traces during parsing
					// they consume same token or rule from the same state
					// performance will be bad if duplicated parsing actually happen
					// could save 20x wasted traces for Workflow parser
					MergeEdgesWithSameInput(ruleSymbol, startState, newStates, newEdges);
					MergeEdgesWithSameRuleUsingLeftrec(ruleSymbol, ruleSymbol->startStates[0], newStates, newEdges);
				}

				auto pmCache = CreatePrefixMergeCache();
				if (!pmCache) return;
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