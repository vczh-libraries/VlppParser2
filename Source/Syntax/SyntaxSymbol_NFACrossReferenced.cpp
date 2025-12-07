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
						if (edge->returnEdges.Count() == 0)
						{
							// Cannot call CreateEdge here because it is checked as a public API
							// But during NFA building we should still change the automaton
							auto newEdge = Ptr(new EdgeSymbol(startState, edge->To()));
							edges.Add(newEdge);

							newEdge->input = edge->input;
							CopyFrom(newEdge->competitions, edge->competitions, true);
							for (auto acc : accumulatedEdges)
							{
								newEdge->returnEdges.Add(acc);
							}
							CopyFrom(newEdge->insAfterInput, edge->insAfterInput, true);
						}
						break;
					case EdgeInputType::Rule:
						// RuleIsIndirectlyLeftRecursive has been checked so there will be no deadloop 
						accumulatedEdges.Add(edge);
						FixCrossReferencedRuleEdge(startState, orderedEdges, accumulatedEdges);
						accumulatedEdges.RemoveAt(accumulatedEdges.Count() - 1);
						break;
					case EdgeInputType::Epsilon:
					case EdgeInputType::Ending:
					case EdgeInputType::LeftRec:
						// Epsilon edges do not exist in compact-NFA
						// Ending and LeftRec edges are not involved
						break;
					default:
						CHECK_FAIL(L"<BuildCrossReferencedNFAInternal>Unhandled!");
					}
				}
			}

/***********************************************************************
SyntaxSymbolManager::BuildCrossReferencedNFAInternal
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

			void SyntaxSymbolManager::BuildCrossReferencedNFAInternal()
			{
				{
					collections::List<EdgeSymbol*> accumulatedEdges;
					for (auto ruleSymbol : rules.map.Values())
					{
						// there will be only one start state per rule in CompactNFA
						CheckIndirectLeftRecursion(ruleSymbol->startStates[0], accumulatedEdges);
					}
					if (global.Errors().Count() > 0)
					{
						return;
					}
				}
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
			}
		}
	}
}