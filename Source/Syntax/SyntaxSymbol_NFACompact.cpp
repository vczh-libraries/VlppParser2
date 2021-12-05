#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
StateSymbolSet
***********************************************************************/

			struct StateSymbolSet
			{
			private:
				static const SortedList<StateSymbol*> EmptyStates;

				Ptr<SortedList<StateSymbol*>> states;

			public:
				StateSymbolSet() = default;
				StateSymbolSet(const StateSymbolSet&) = delete;
				StateSymbolSet& operator=(const StateSymbolSet&) = delete;

				StateSymbolSet(StateSymbolSet&& set)
				{
					states = set.states;
					set.states = nullptr;
				}

				StateSymbolSet& operator=(StateSymbolSet&& set)
				{
					states = set.states;
					set.states = nullptr;
					return *this;
				}

				StateSymbolSet Copy() const
				{
					StateSymbolSet set;
					set.states = states;
					return set;
				}

				bool Add(StateSymbol* state)
				{
					if (states)
					{
						if (states->Contains(state)) return false;
						states->Add(state);
						return true;
					}
					else
					{
						states = new SortedList<StateSymbol*>();
						states->Add(state);
						return true;
					}
				}

				const SortedList<StateSymbol*>& States() const
				{
					return states ? *states.Obj() : EmptyStates;
				}

				vint Compare(const StateSymbolSet& set) const
				{
					if (!states && !set.states) return 0;
					if (!states) return -1;
					if (!set.states) return 1;
					return CompareEnumerable(*states.Obj(), *set.states.Obj());
				}

				bool operator==(const StateSymbolSet& set) const { return Compare(set) == 0; }
				bool operator!=(const StateSymbolSet& set) const { return Compare(set) != 0; }
				bool operator< (const StateSymbolSet& set) const { return Compare(set) < 0; }
				bool operator<=(const StateSymbolSet& set) const { return Compare(set) <= 0; }
				bool operator> (const StateSymbolSet& set) const { return Compare(set) > 0; }
				bool operator>=(const StateSymbolSet& set) const { return Compare(set) >= 0; }
			};
			const SortedList<StateSymbol*> StateSymbolSet::EmptyStates;

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
					for (auto edge : walkingOldState->OutEdges())
					{
						accumulatedEdges.Add(edge);
						switch (edge->input.type)
						{
						case EdgeInputType::Token:
						case EdgeInputType::Rule:
							{
								auto targetNewState = CreateCompactState(edge->To());
								if (!visited.Contains(targetNewState))
								{
									visited.Add(targetNewState);
								}
								auto newEdge = new EdgeSymbol(newState, targetNewState);
								newEdges.Add(newEdge);
								newEdge->input = edge->input;
								newEdge->important |= edge->important;
								for (auto accumulatedEdge : accumulatedEdges)
								{
									CopyFrom(newEdge->insBeforeInput, accumulatedEdge->insBeforeInput, true);
									CopyFrom(newEdge->insAfterInput, accumulatedEdge->insAfterInput, true);
									newEdge->important |= accumulatedEdge->important;
								}
							}
							break;
						case EdgeInputType::Epsilon:
							BuildEpsilonEliminatedEdgesInternal(edge->To(), newState, endState, visited, accumulatedEdges);
							break;
						}
						accumulatedEdges.RemoveAt(accumulatedEdges.Count() - 1);
					}

					if (walkingOldState->endingState)
					{
						auto newEdge = new EdgeSymbol(newState, endState);
						newEdge->input.type = EdgeInputType::Ending;
						for (auto accumulatedEdge : accumulatedEdges)
						{
							CopyFrom(newEdge->insBeforeInput, accumulatedEdge->insBeforeInput, true);
							CopyFrom(newEdge->insAfterInput, accumulatedEdge->insAfterInput, true);
							newEdge->important |= accumulatedEdge->important;
						}

						for (auto endingEdge : newState->OutEdges())
						{
							if (endingEdge != newEdge && endingEdge->input.type == EdgeInputType::Ending)
							{
								if (
									CompareEnumerable(endingEdge->insBeforeInput, newEdge->insBeforeInput) == 0 &&
									CompareEnumerable(endingEdge->insAfterInput, newEdge->insAfterInput) == 0)
								{
									CHECK_ERROR(newEdge->important == endingEdge->important, L"It is not possible to have two equal ending edges with different priority.");
									newState->outEdges.Remove(newEdge);
									endState->inEdges.Remove(newEdge);
									delete newEdge;
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
						auto newState = new StateSymbol(rule);
						newState->label = state->label;
						newStates.Add(newState);
						oldToNew.Add(state, newState);
						newToOld.Add(newState, state);
						return newState;
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
SyntaxSymbolManager::EliminateLeftRecursion
***********************************************************************/

			void SyntaxSymbolManager::EliminateLeftRecursion(RuleSymbol* rule, StateSymbol* startState, StateSymbol* endState, StateList& newStates, EdgeList& newEdges)
			{
				List<EdgeSymbol*> lrecEdges;
				for (auto edge : startState->OutEdges())
				{
					if (edge->input.type == EdgeInputType::Rule && edge->input.rule == rule)
					{
						lrecEdges.Add(edge);
					}
				}

				for (auto lrecEdge : lrecEdges)
				{
					for (auto endingEdge : endState->InEdges())
					{
						auto state = endingEdge->From();
						auto newEdge = new EdgeSymbol(state, lrecEdge->To());
						newEdges.Add(newEdge);
						newEdge->important |= endingEdge->important;
						newEdge->important |= lrecEdge->important;

						newEdge->input.type = EdgeInputType::LeftRec;
						CopyFrom(newEdge->insBeforeInput, endingEdge->insBeforeInput, true);
						CopyFrom(newEdge->insAfterInput, endingEdge->insAfterInput, true);
						CopyFrom(newEdge->insBeforeInput, lrecEdge->insBeforeInput, true);
						CopyFrom(newEdge->insAfterInput, lrecEdge->insAfterInput, true);

						for (vint i = 0; i < newEdge->insBeforeInput.Count(); i++)
						{
							auto& ins = newEdge->insBeforeInput[i];
							if (ins.type == AstInsType::BeginObject)
							{
								ins.type = AstInsType::BeginObjectLeftRecursive;
							}
						}
						for (vint i = 0; i < newEdge->insAfterInput.Count(); i++)
						{
							auto& ins = newEdge->insAfterInput[i];
							if (ins.type == AstInsType::BeginObject)
							{
								ins.type = AstInsType::BeginObjectLeftRecursive;
							}
						}
					}
				}

				for (auto lrecEdge : lrecEdges)
				{
					lrecEdge->From()->outEdges.Remove(lrecEdge);
					lrecEdge->To()->inEdges.Remove(lrecEdge);
					newEdges.Remove(lrecEdge);
				}
			}

/***********************************************************************
SyntaxSymbolManager::EliminateEpsilonEdges
***********************************************************************/

			StateSymbol* SyntaxSymbolManager::EliminateEpsilonEdges(RuleSymbol* rule, StateList& newStates, EdgeList& newEdges)
			{
				auto psuedoState = CreateState(rule);
				for (auto startState : rule->startStates)
				{
					CreateEdge(psuedoState, startState);
				}

				CompactSyntaxBuilder builder(rule, newStates, newEdges);
				auto compactStartState = builder.CreateCompactState(psuedoState);
				compactStartState->label = L" BEGIN ";

				auto compactEndState = new StateSymbol(rule);
				compactEndState->label = L" END ";
				compactEndState->endingState = true;
				newStates.Add(compactEndState);

				List<StateSymbol*> visited;
				visited.Add(compactStartState);

				for (vint i = 0; i < visited.Count(); i++)
				{
					auto current = visited[i];
					builder.BuildEpsilonEliminatedEdges(current, compactEndState, visited);
				}

				EliminateLeftRecursion(rule, compactStartState, compactEndState, newStates, newEdges);
				return compactStartState;
			}

/***********************************************************************
SyntaxSymbolManager::BuildCompactNFAInternal
***********************************************************************/

			void SyntaxSymbolManager::BuildCompactNFAInternal()
			{
				StateList newStates;
				EdgeList newEdges;
				for (auto ruleSymbol : rules.map.Values())
				{
					auto startState = EliminateEpsilonEdges(ruleSymbol, newStates, newEdges);
					ruleSymbol->startStates.Clear();
					ruleSymbol->startStates.Add(startState);
				}
				CopyFrom(states, newStates);
				CopyFrom(edges, newEdges);

				for (auto state : states)
				{
					bool competition = false;
					for (auto edge : state->OutEdges())
					{
						if (edge->important)
						{
							competition = true;
							break;
						}
					}

					if (competition)
					{
						for (auto edge : state->OutEdges())
						{
							edge->importancy = edge->important ? EdgeImportancy::HighPriority : EdgeImportancy::LowPriority;
						}
					}
				}
			}
		}
	}
}