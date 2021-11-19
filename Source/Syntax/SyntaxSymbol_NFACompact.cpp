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
				Dictionary<StateSymbolSet, StateSymbol*>	closureToNew;
				Dictionary<StateSymbol*, StateSymbolSet>	newToClosure;
				Dictionary<StateSymbol*, StateSymbolSet>	oldToClosure;

				static bool IsPureEpsilonEdge(EdgeSymbol* edge)
				{
					if (edge->input.type != EdgeInputType::Epsilon) return false;
					CHECK_ERROR(edge->insAfterInput.Count() == 0, L"vl::gre::parsergen::CompactSyntaxBuilder::IsPureEpsilonEdge(EdgeSymbol*)#Epsilon edge is not allowed to have non-empty insAfterInput.");
					return edge->insBeforeInput.Count() == 0;
				}

				static bool IsPureEpsilonState(StateSymbol* state)
				{
					return From(state->OutEdges()).All(IsPureEpsilonEdge);
				}

				StateSymbol* CreateCompactState(StateSymbolSet&& key)
				{
					vint index = closureToNew.Keys().IndexOf(key);
					if (index != -1)
					{
						return closureToNew.Values()[index];
					}

					auto newState = new StateSymbol(rule);
					newStates.Add(newState);
					newToClosure.Add(newState, key.Copy());
					closureToNew.Add(std::move(key), newState);

					return newState;
				}

				StateSymbolSet CalculateEpsilonClosure(StateSymbol* state)
				{
					vint index = oldToClosure.Keys().IndexOf(state);
					if (index != -1)
					{
						return oldToClosure.Values()[index].Copy();
					}
					else
					{
						StateSymbolSet key;
						List<StateSymbol*> visited;
						key.Add(state);
						visited.Add(state);

						for (vint i = 0; i < visited.Count(); i++)
						{
							auto current = visited[i];
							for (auto edge : current->OutEdges())
							{
								if (IsPureEpsilonEdge(edge))
								{
									auto toState = edge->To();
									if (!visited.Contains(toState))
									{
										visited.Add(toState);
										if (!IsPureEpsilonState(toState))
										{
											key.Add(toState);
										}
									}
								}
							}
						}

						oldToClosure.Add(state, key.Copy());
						return std::move(key);
					}
				}

				void BuildEpsilonEliminatedEdgesInternal(
					StateSymbolSet walkingClosure,
					StateSymbol* newState,
					StateSymbol* endState,
					List<StateSymbol*>& visited,
					List<EdgeSymbol*>& accumulatedEdges)
				{
					for (auto oldState : walkingClosure.States())
					{
						for (auto edge : oldState->OutEdges())
						{
							if (!IsPureEpsilonEdge(edge))
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
										for (auto accumulatedEdge : accumulatedEdges)
										{
											CopyFrom(newEdge->insBeforeInput, accumulatedEdge->insBeforeInput, true);
											CopyFrom(newEdge->insAfterInput, accumulatedEdge->insAfterInput, true);
										}
									}
									break;
								case EdgeInputType::Epsilon:
									BuildEpsilonEliminatedEdgesInternal(CalculateEpsilonClosure(edge->To()), newState, endState, visited, accumulatedEdges);
									break;
								}
								accumulatedEdges.RemoveAt(accumulatedEdges.Count() - 1);
							}
						}

						if (oldState->endingState)
						{
							auto newEdge = new EdgeSymbol(newState, endState);
							newEdge->input.type = EdgeInputType::Ending;
							for (auto accumulatedEdge : accumulatedEdges)
							{
								CopyFrom(newEdge->insBeforeInput, accumulatedEdge->insBeforeInput, true);
								CopyFrom(newEdge->insAfterInput, accumulatedEdge->insAfterInput, true);
							}

							for (auto endingEdge : newState->OutEdges())
							{
								if (endingEdge != newEdge && endingEdge->input.type == EdgeInputType::Ending)
								{
									if(
										CompareEnumerable(endingEdge->insBeforeInput,newEdge->insBeforeInput) == 0 &&
										CompareEnumerable(endingEdge->insAfterInput, newEdge->insAfterInput) == 0)
									{
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
						auto newState = CreateCompactState(CalculateEpsilonClosure(state));
						newState->label = state->label;
						oldToNew.Add(state, newState);
						return newState;
					}
				}

				void BuildEpsilonEliminatedEdges(
					StateSymbol* newState,
					StateSymbol* endState,
					List<StateSymbol*>& visited)
				{
					List<EdgeSymbol*> accumulatedEdges;
					BuildEpsilonEliminatedEdgesInternal(newToClosure[newState].Copy(), newState, endState, visited, accumulatedEdges);
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
			}
		}
	}
}