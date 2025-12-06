#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
SymbolSet
***********************************************************************/

			template<typename TSymbol, bool Ordered>
			struct SymbolSetListType;

			template<typename TSymbol>
			struct SymbolSetListType<TSymbol, false>
			{
				using Type = List<TSymbol>;
			};

			template<typename TSymbol>
			struct SymbolSetListType<TSymbol, true>
			{
				using Type = SortedList<TSymbol>;
			};

			template<typename TSymbol, bool Ordered>
			using SymbolSetListType_t = typename SymbolSetListType<TSymbol, Ordered>::Type;;

			template<typename TSymbol, bool Ordered>
			struct SymbolSet
			{
			public:
				using ListType = SymbolSetListType_t<TSymbol, Ordered>;
				using ListPtr = Ptr<ListType>;

			private:
				static const ListType				EmptySymbols;
				ListPtr								symbols;

			public:
				SymbolSet() = default;
				SymbolSet(const SymbolSet&) = delete;
				SymbolSet<TSymbol, Ordered>& operator=(const SymbolSet<TSymbol, Ordered>&) = delete;

				SymbolSet<TSymbol, Ordered>(SymbolSet<TSymbol, Ordered>&& set)
				{
					symbols = set.symbols;
					set.symbols = nullptr;
				}

				SymbolSet<TSymbol, Ordered>& operator=(SymbolSet<TSymbol, Ordered>&& set)
				{
					symbols = set.symbols;
					set.symbols = nullptr;
					return *this;
				}

				SymbolSet(TSymbol _symbol)
				{
					Add(_symbol);
				}

				SymbolSet(const IEnumerable<TSymbol>& _symbols)
				{
					symbols = Ptr(new ListType);
					CopyFrom(*symbols.Obj(), _symbols);
				}

				bool Add(TSymbol _symbol)
				{
					if (!symbols)
					{
						symbols = Ptr(new ListType);
					}
					if (symbols->Contains(_symbol)) return false;
					symbols->Add(_symbol);
					return true;
				}

				const ListType& Symbols() const
				{
					return symbols ? *symbols.Obj() : EmptySymbols;
				}

				ListPtr SymbolsPtr() const
				{
					return symbols;
				}

				std::strong_ordering operator<=>(const SymbolSet<TSymbol, Ordered>& set) const
				{
					if (!symbols && !set.symbols) return std::strong_ordering::equal;
					if (!symbols) return std::strong_ordering::less;
					if (!set.symbols) return std::strong_ordering::greater;
					return CompareEnumerable(*symbols.Obj(), *set.symbols.Obj());
				}

				bool operator==(const SymbolSet<TSymbol, Ordered>& set) const = default;
			};

			template<typename TSymbol, bool Ordered>
			const SymbolSetListType_t<TSymbol, Ordered> SymbolSet<TSymbol, Ordered>::EmptySymbols;

			struct LabeledState
			{
				WString				label;
				StateSymbol*		state = nullptr;

				auto operator<=>(const LabeledState& ls) const { return label <=> ls.label; };
				bool operator==(const LabeledState&) const = default;
			};

			using StateSymbolSet = SymbolSet<LabeledState, true>;
			using InsSymbolSet = SymbolSet<AstIns, false>;

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

/***********************************************************************
SyntaxSymbolManager::MergeEdgesWithSameInput
***********************************************************************/

			void SyntaxSymbolManager::MergeEdgesWithSameInput(RuleSymbol* rule, StateSymbol* startState, StateList& newStates, EdgeList& newEdges)
			{
				// Just like building DFA
				//   start from startState, put into pending list
				//   group outgoing edges by input
				//   make new state for each group with multiple input
				//   maintain a map from merged states to use state using StateSymbolSet
				//   for each grouped edge, whether new states are created or not, put target state into pending list
				//   work until pending list is empty
				// After merging, newStates and newEdges should not contain removed objects
				// We should take into consideration that input includes insAfterInput
				//   returnEdges are always empty at the moment
				//   competitions should be merged

				Dictionary<StateSymbolSet, Ptr<StateSymbol>> statesToMerged;
				Dictionary<StateSymbol*, StateSymbolSet::ListPtr> mergedToStates;

				StateList createdStates;
				EdgeList createdEdges;
				List<StateSymbol*> workingStates;
				SortedList<StateSymbol*> reusedStates;
				SortedList<EdgeSymbol*> reusedEdges;

				// Start from the start state
				{
					reusedStates.Add(startState);
					workingStates.Add(startState);
				}

				auto ReuseState = [&](StateSymbol* state)
				{
					if (!reusedStates.Contains(state))
					{
						reusedStates.Add(state);
						workingStates.Add(state);
					}
				};

				auto ReuseEdge = [&](EdgeSymbol* edge)
				{
					ReuseState(edge->To());
					if (!reusedEdges.Contains(edge))
					{
						reusedEdges.Add(edge);
					}
				};

				auto ApplyEdgeToMergedState = [&](EdgeSymbol* edge, StateSymbol* mergedState)
				{
					ReuseState(edge->To());
					auto newEdge = Ptr(new EdgeSymbol(mergedState, edge->To()));
					createdEdges.Add(newEdge);
					newEdge->input = edge->input;
					CopyFrom(newEdge->competitions, edge->competitions);
					CopyFrom(newEdge->insAfterInput, edge->insAfterInput);
				};

				for (vint i = 0; i < workingStates.Count(); i++)
				{
					auto currentState = workingStates[i];
					vint currentMergedToStateIndex = mergedToStates.Keys().IndexOf(currentState);

					Group<Pair<EdgeInput, InsSymbolSet>, EdgeSymbol*> groupedEdges;
					if (currentMergedToStateIndex == -1)
					{
						// if the current state is an original state
						for (auto edge : currentState->OutEdges())
						{
							if (edge->input.type == EdgeInputType::Token || edge->input.type == EdgeInputType::Rule)
							{
								// only group Token or Rule edges
								groupedEdges.Add({ edge->input,InsSymbolSet{edge->insAfterInput} }, edge);
							}
							else
							{
								// reuse others
								ReuseEdge(edge);
							}
						}
					}
					else
					{
						// if the current state is a merged state, search all of its original states
						for (auto targetState : *mergedToStates.Values()[currentMergedToStateIndex].Obj())
						{
							for (auto edge : targetState.state->OutEdges())
							{
								if (edge->input.type == EdgeInputType::Token || edge->input.type == EdgeInputType::Rule)
								{
									// only group Token or Rule edges
									groupedEdges.Add({ edge->input,InsSymbolSet{edge->insAfterInput} }, edge);
								}
								else
								{
									// duplicate others to start from the merged state
									ApplyEdgeToMergedState(edge, currentState);
								}
							}
						}
					}

					// see if multiple edges could be grouped together
					for (vint groupedIndex = 0; groupedIndex < groupedEdges.Count(); groupedIndex++)
					{
						auto&& groupedKey = groupedEdges.Keys()[groupedIndex];
						auto&& groupedValues = groupedEdges.GetByIndex(groupedIndex);

						if (groupedValues.Count() == 1)
						{
							// if a group only has one edge, reuse the target state
							if (currentMergedToStateIndex == -1)
							{
								ReuseEdge(groupedValues[0]);
							}
							else
							{
								ApplyEdgeToMergedState(groupedValues[0], currentState);
							}
						}
						else
						{
							// if a group has multiple edges, merge all target states into one
							Ptr<StateSymbol> mergedState;
							{
								StateSymbolSet targetSet;
								for (auto edge : groupedValues)
								{
									targetSet.Add({ edge->To()->label,edge->To() });
								}

								vint index = statesToMerged.Keys().IndexOf(targetSet);
								if (index != -1)
								{
									mergedState = statesToMerged.Values()[index];
								}
								else
								{
									mergedState = Ptr(new StateSymbol(startState->Rule()));
									createdStates.Add(mergedState);
									workingStates.Add(mergedState.Obj());

									mergedState->label = stream::GenerateToStream([&](stream::TextWriter& writer)
									{
										writer.WriteString(L"{{");
										for (auto [state, index] : indexed(targetSet.Symbols()))
										{
											if (index > 0) writer.WriteString(L" ; ");
											writer.WriteString(state.label);
										}
										writer.WriteString(L"}}");
									});

									mergedToStates.Add(mergedState.Obj(), targetSet.SymbolsPtr());
									statesToMerged.Add(std::move(targetSet), mergedState);
								}
							}

							auto newEdge = Ptr(new EdgeSymbol(currentState, mergedState.Obj()));
							createdEdges.Add(newEdge);

							newEdge->input = groupedKey.key;
							CopyFrom(newEdge->insAfterInput, groupedKey.value.Symbols());
							for (auto edge : groupedValues)
							{
								CopyFrom(newEdge->competitions, edge->competitions, true);
							}
						}
					}
				}

				if (createdStates.Count() + createdEdges.Count() > 0)
				{
					for (vint i = newEdges.Count() - 1; i >= 0; i--)
					{
						auto edge = newEdges[i];
						if (edge->From()->Rule() != rule) break;
						if (!reusedEdges.Contains(edge.Obj()))
						{
							edge->From()->outEdges.Remove(edge.Obj());
							edge->To()->inEdges.Remove(edge.Obj());
							newEdges.RemoveAt(i);
						}
					}

					for (vint i = newStates.Count() - 1; i >= 0; i--)
					{
						auto state = newStates[i];
						if (state->Rule() != rule) break;
						if (!reusedStates.Contains(state.Obj()))
						{
							newStates.RemoveAt(i);
						}
					}

					CopyFrom(newStates, createdStates, true);
					CopyFrom(newEdges, createdEdges, true);
				}
			}

/***********************************************************************
SyntaxSymbolManager::EliminateEpsilonEdges
***********************************************************************/

			StateSymbol* SyntaxSymbolManager::EliminateEpsilonEdges(RuleSymbol* rule, StateList& newStates, EdgeList& newEdges)
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
				auto psuedoState = CreateState(rule);
				for (auto startState : rule->startStates)
				{
					CreateEdge(psuedoState, startState);
				}

				CompactSyntaxBuilder builder(rule, newStates, newEdges);
				auto compactStartState = builder.CreateCompactState(psuedoState);
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

				// optimize
				EliminateLeftRecursion(rule, compactStartState, compactEndState.Obj(), newStates, newEdges);
				MergeEdgesWithSameInput(rule, compactStartState, newStates, newEdges);

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
				states = std::move(newStates);
				edges = std::move(newEdges);
			}
		}
	}
}