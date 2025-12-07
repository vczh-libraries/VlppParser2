#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
SyntaxSymbolManager::BuildLeftRecEdge
***********************************************************************/

			void SyntaxSymbolManager::BuildLeftRecEdge(EdgeSymbol* newEdge, EdgeSymbol* endingEdge, EdgeSymbol* lrecPrefixEdge)
			{
				CopyFrom(newEdge->competitions, endingEdge->competitions, true);
				CopyFrom(newEdge->competitions, lrecPrefixEdge->competitions, true);

				newEdge->input.type = EdgeInputType::LeftRec;
				CopyFrom(newEdge->insAfterInput, endingEdge->insAfterInput, true);
				CopyFrom(newEdge->insAfterInput, lrecPrefixEdge->insAfterInput, true);
			}

/***********************************************************************
SyntaxSymbolManager::EliminateLeftRecursion
***********************************************************************/

			void SyntaxSymbolManager::EliminateLeftRecursion(RuleSymbol* rule, StateSymbol* startState, StateSymbol* endState, StateList& newStates, EdgeList& newEdges)
			{
				/*
				* Move the single rule prefix from the rule begin state
				* if it is left recursive
				* 
				* [BEFORE] (r is the current rule)
				*    +-> ... -> A --------(ending)-+
				*    |                             |
				* S -+-(r)----> ... -> B -(ending)-+-> E
				*    |    ---                      |
				*    +-(r)----> ... -> C -(ending)-+
				* 
				* [AFTER] (the epsilon edge doesn't exist, it is for demo only)
				*            +----(epsilon)----------+
				*            |                       |
				*            |  +-(leftrec)-> ... -> B -(ending)---+
				*            v  |                                  v
				* S-> ... -> A -+-----------------------(ending)-> E
				*            ^  |                                  ^
				*            |  +-(leftrec)-> ... -> C -(ending)---+
				*            |                       |
				*            +----(epsilon)----------+
				*/

				List<EdgeSymbol*> lrecEdges;
				for (auto edge : startState->OutEdges())
				{
					if (edge->input.type != EdgeInputType::Rule) continue;
					if (edge->input.rule != rule) continue;
					lrecEdges.Add(edge);
				}

				for (auto lrecEdge : lrecEdges)
				{
					for (auto endingEdge : endState->InEdges())
					{
						auto state = endingEdge->From();
						auto newEdge = Ptr(new EdgeSymbol(state, lrecEdge->To()));
						newEdges.Add(newEdge);
						BuildLeftRecEdge(newEdge.Obj(), endingEdge, lrecEdge);
					}
				}

				for (auto lrecEdge : lrecEdges)
				{
					lrecEdge->From()->outEdges.Remove(lrecEdge);
					lrecEdge->To()->inEdges.Remove(lrecEdge);
					newEdges.Remove(lrecEdge);
				}
			}
		}
	}
}