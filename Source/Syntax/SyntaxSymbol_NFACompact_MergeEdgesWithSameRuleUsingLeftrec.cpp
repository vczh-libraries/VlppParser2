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
				*    +-(r)-> U -(b)-> X
				*    |
				* A -+-(r)-> V -(b)-> Y
				*    |
				*    +-(r)-> W -(c)-> Z
				*
				* [AFTER]
				*
				*             +-(leftrec)-> U -(b)-> X
				*             |
				* A -(r)-> B -+-(leftrec)-> V -(b)-> Y
				*             |
				*             +-(leftrec)-> W -(c)-> Z
				*/

				IncrementalChange ic;
				CopyFrom(ic.reusedStates, From(newStates).Select([](auto p) {return p.Obj(); }));
				CopyFrom(ic.reusedEdges, From(newEdges).Select([](auto p) {return p.Obj(); }));

				SortedList<StateSymbol*> visitedStates;
				List<StateSymbol*> workingStates;
				workingStates.Add(startState);

				for (vint i = 0; i < workingStates.Count(); i++)
				{
					auto currentState = workingStates[i];
					Group<RuleSymbol*, EdgeSymbol*> groupedEdges;
					for (auto edge : currentState->OutEdges())
					{
						if (edge->input.type != EdgeInputType::Rule) continue;
						groupedEdges.Add(edge->input.rule, edge);
					}

					for (vint j = 0; j < groupedEdges.Count(); j++)
					{
						auto&& edges = groupedEdges.GetByIndex(j);
						if (edges.Count() > 1)
						{
							auto newState = Ptr(new StateSymbol(rule));
							ic.createdStates.Add(newState);
							auto newEdge = Ptr(new EdgeSymbol(currentState, newState.Obj()));
							ic.createdEdges.Add(newEdge);

							newState->label = currentState->label + L"[pm-lr]";

							newEdge->input = edges[0]->input;
							CopyFrom(newState->outEdges, edges);
							for (auto edge : edges)
							{
								edge->fromState->outEdges.Remove(edge);
								edge->fromState = newState.Obj();
								edge->input = { EdgeInputType::LeftRec };
							}
						}
					}

					for (auto edge : currentState->OutEdges())
					{
						if (edge->input.type != EdgeInputType::Rule) continue;
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