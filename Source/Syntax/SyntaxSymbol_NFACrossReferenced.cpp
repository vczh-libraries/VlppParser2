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

			void SyntaxSymbolManager::FixCrossReferencedRuleEdge(StateSymbol* startState, collections::List<EdgeSymbol*>& accumulatedEdges)
			{
				auto currentState = accumulatedEdges[accumulatedEdges.Count() - 1]->To();
				for (auto edge : currentState->OutEdges())
				{
					switch (edge->input.type)
					{
					case EdgeInputType::Token:
						{
							auto newEdge = new EdgeSymbol(startState, edge->To());
							edges.Add(newEdge);

							for (auto acc : accumulatedEdges)
							{
								CopyFrom(newEdge->insBeforeInput, acc->insBeforeInput, true);
								CopyFrom(newEdge->insAfterInput, acc->insAfterInput, true);
								newEdge->returnStates.Add(acc->To());
							}
							CopyFrom(newEdge->insBeforeInput, edge->insBeforeInput, true);
							CopyFrom(newEdge->insAfterInput, edge->insAfterInput, true);
						}
						break;
					case EdgeInputType::Rule:
						if (accumulatedEdges.Contains(edge))
						{
							global.AddError(
								ParserErrorType::RuleIsIndirectlyLeftRecursive,
								edge->input.rule->Name()
								);
						}
						else
						{
							accumulatedEdges.Add(edge);
							FixCrossReferencedRuleEdge(startState, accumulatedEdges);
							accumulatedEdges.RemoveAt(accumulatedEdges.Count() - 1);
						}
						break;
					}
				}
			}

/***********************************************************************
SyntaxSymbolManager::BuildCrossReferencedNFAInternal
***********************************************************************/

			void SyntaxSymbolManager::BuildCrossReferencedNFAInternal()
			{
				for (auto edge : edges)
				{
					if (edge->input.type == EdgeInputType::Rule)
					{
						List<EdgeSymbol*> accumulatedEdges;
						accumulatedEdges.Add(edge.Obj());
						FixCrossReferencedRuleEdge(edge->From(), accumulatedEdges);
					}
				}
			}
		}
	}
}