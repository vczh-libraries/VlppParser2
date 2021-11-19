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
				auto lastEdge = accumulatedEdges[accumulatedEdges.Count() - 1];
				auto lastRule = lastEdge->input.rule;
				auto ruleBegin = lastRule->startStates[0];
				for (auto edge : ruleBegin->OutEdges())
				{
					switch (edge->input.type)
					{
					case EdgeInputType::Token:
						if (edge->returnEdges.Count() == 0)
						{
							auto newEdge = new EdgeSymbol(startState, edge->To());
							edges.Add(newEdge);

							newEdge->input = edge->input;
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
				vint count = edges.Count();
				for (vint i = 0; i < count; i++)
				{
					auto edge = edges[i].Obj();
					if (edge->input.type == EdgeInputType::Rule)
					{
						List<EdgeSymbol*> accumulatedEdges;
						accumulatedEdges.Add(edge);
						FixCrossReferencedRuleEdge(edge->From(), accumulatedEdges);
					}
				}
			}
		}
	}
}