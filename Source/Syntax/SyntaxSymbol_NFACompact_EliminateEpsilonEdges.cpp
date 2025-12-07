#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
CompactSyntaxBuilder
***********************************************************************/

			class CompactSyntaxBuilder
			{
				using StateList = collections::List<Ptr<StateSymbol>>;
				using EdgeList = collections::List<Ptr<EdgeSymbol>>;
			protected:
				RuleSymbol*									rule;
				StateList&									newStates;
				EdgeList&									newEdges;
				Dictionary<StateSymbol*, StateSymbol*>		oldToNew;
				Dictionary<StateSymbol*, StateSymbol*>		newToOld;

				void BuildEpsilonEliminatedEdgesInternal(
					StateSymbol* walkingOldState,
					StateSymbol* newState,
					StateSymbol* endState,
					List<StateSymbol*>& visited,
					List<EdgeSymbol*>& accumulatedEdges)
				{
					/*
					* walkingOldState  : a state in the epsilon-NFA
					* newState         : a state in the compact-NFA
					*                    it represents the mirrored walkingOldState in the first call when accumulatedEdges is empty
					*                    in future recursive calls, walkingOldState keeps changing, but newState stays the same
					* endState         : the ending state of the rule
					* visited          : stores any new discovered epsilon-NFA states
					*                    duplicated states will not be added to this list
					* accumulatedEdges : epsilon edges from the first walkingOldState to the current walkingOldState
					*/

					for (auto edge : walkingOldState->OutEdges())
					{
						accumulatedEdges.Add(edge);
						switch (edge->input.type)
						{
						case EdgeInputType::Token:
						case EdgeInputType::Rule:
							{
								// a new edge is created, accumulating multiple epsilon edges, ending with such edge
								auto targetNewState = CreateCompactState(edge->To());
								if (!visited.Contains(targetNewState))
								{
									visited.Add(targetNewState);
								}
								auto newEdge = Ptr(new EdgeSymbol(newState, targetNewState));
								newEdges.Add(newEdge);
								newEdge->input = edge->input;
								for (auto accumulatedEdge : accumulatedEdges)
								{
									CopyFrom(newEdge->insAfterInput, accumulatedEdge->insAfterInput, true);
									CopyFrom(newEdge->competitions, accumulatedEdge->competitions, true);
								}
							}
							break;
						case EdgeInputType::Epsilon:
							BuildEpsilonEliminatedEdgesInternal(edge->To(), newState, endState, visited, accumulatedEdges);
							break;
						case EdgeInputType::Ending:
							// Ending is ignored because it doesn't exist in epsilon-NFA
							break;
						default:
							CHECK_FAIL(L"<BuildCompactNFAInternal>Unhandled!");
						}
						accumulatedEdges.RemoveAt(accumulatedEdges.Count() - 1);
					}

					if (walkingOldState->endingState)
					{
						// if accumulated epsilon edges lead to the epsilon-NFA ending state
						// create an Ending edge to the compact-NFA ending state
						// when a non-epsilon edge connects to the ending state directly
						// this is triggered by examing the epsilon-NFA ending state directly
						// at this moment accumulatedEdges is an empty collection
						auto newEdge = Ptr(new EdgeSymbol(newState, endState));
						newEdge->input.type = EdgeInputType::Ending;
						for (auto accumulatedEdge : accumulatedEdges)
						{
							CopyFrom(newEdge->insAfterInput, accumulatedEdge->insAfterInput, true);
							CopyFrom(newEdge->competitions, accumulatedEdge->competitions, true);
						}

						for (auto endingEdge : newState->OutEdges())
						{
							if (endingEdge != newEdge && endingEdge->input.type == EdgeInputType::Ending)
							{
								if (CompareEnumerable(endingEdge->insAfterInput, newEdge->insAfterInput) == 0)
								{
									newState->outEdges.Remove(newEdge.Obj());
									endState->inEdges.Remove(newEdge.Obj());
									goto DISCARD_ENDING_EDGE;
								}
							}
						}
						newEdges.Add(newEdge);
					DISCARD_ENDING_EDGE:;
					}
				}

			public:
				CompactSyntaxBuilder(RuleSymbol* _rule, StateList& _newStates, EdgeList& _newEdges)
					: rule(_rule)
					, newStates(_newStates)
					, newEdges(_newEdges)
				{
				}

				StateSymbol* CreateCompactState(StateSymbol* state)
				{
					vint index = oldToNew.Keys().IndexOf(state);
					if (index != -1)
					{
						return oldToNew.Values()[index];
					}
					else
					{
						auto newState = Ptr(new StateSymbol(rule));
						newState->label = state->label;
						newStates.Add(newState);
						oldToNew.Add(state, newState.Obj());
						newToOld.Add(newState.Obj(), state);
						return newState.Obj();
					}
				}

				void BuildEpsilonEliminatedEdges(
					StateSymbol* newState,
					StateSymbol* endState,
					List<StateSymbol*>& visited)
				{
					List<EdgeSymbol*> accumulatedEdges;
					BuildEpsilonEliminatedEdgesInternal(newToOld[newState], newState, endState, visited, accumulatedEdges);
				}
			};

/***********************************************************************
SyntaxSymbolManager::EliminateEpsilonEdges
***********************************************************************/

			SyntaxSymbolManager::StartEndStatePair SyntaxSymbolManager::EliminateEpsilonEdges(RuleSymbol* rule, StateList& newStates, EdgeList& newEdges)
			{
				/*
				* For any transition that goes through some epsilon edge and ends with a non-epsilon edge
				* we copy all instructions from epsilon edges and the non-epsilon edge in order
				* and create a new edge directly pointing to the toState of the non-epsilon edge
				* 
				* [BEFORE]
				*         +-(x)-> B
				*         |
				* A -(e1)-+-(e2)-> C -+-(y)-> E
				*         |           |
				*         +-(e3)-> D -+
				* 
				* [AFTER]
				*    +-(e1,x)-> B
				*    |
				* A -+-(e1,e2,y)-> E
				*    |             ^
				*    +-(e1,e3,y)---+
				*/

				// epsilon-NFAs are per clause
				// now we need to create a start state and an ending state
				// to connect all epsilon-NFAs of its clauses together
				auto psuedoState = Ptr(new StateSymbol(rule));
				newStates.Add(psuedoState);
				for (auto startState : rule->startStates)
				{
					newEdges.Add(Ptr(new EdgeSymbol(psuedoState.Obj(), startState)));
				}

				CompactSyntaxBuilder builder(rule, newStates, newEdges);
				auto compactStartState = builder.CreateCompactState(psuedoState.Obj());
				compactStartState->label = L" BEGIN ";

				auto compactEndState = Ptr(new StateSymbol(rule));
				compactEndState->label = L" END ";
				compactEndState->endingState = true;
				newStates.Add(compactEndState);

				List<StateSymbol*> visited;
				visited.Add(compactStartState);

				// all epsilon-NFAs of its clauses become one connected epsilon-NFA of this rule
				// we can build the compact-NFA out of this epsilon-NFA starting from the start state
				// TODO: (enumerable) foreach:alterable
				for (vint i = 0; i < visited.Count(); i++)
				{
					auto current = visited[i];
					builder.BuildEpsilonEliminatedEdges(current, compactEndState.Obj(), visited);
				}

				return { compactStartState,compactEndState.Obj()};
			}
		}
	}
}