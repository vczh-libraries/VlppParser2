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
				vint				index;
				StateSymbol*		state = nullptr;

				auto operator<=>(const LabeledState& ls) const = default;
				bool operator==(const LabeledState&) const = default;
			};

			using StateSymbolSet = SymbolSet<LabeledState, true>;
			using InsSymbolSet = SymbolSet<AstIns, false>;
			using CompetitionSymbolSet = SymbolSet<EdgeCompetition, true>;

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
							edge->From()->outEdges.Remove(edge.Obj());
							edge->To()->inEdges.Remove(edge.Obj());
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
				// We should take into consideration that input includes insAfterInput and competitions
				//   returnEdges are always empty at the moment

				Dictionary<StateSymbolSet, Ptr<StateSymbol>> statesToMerged;
				Dictionary<StateSymbol*, StateSymbolSet::ListPtr> mergedToStates;

				List<StateSymbol*> workingStates;
				IncrementalChange ic;

				// Start from the start state
				{
					ic.reusedStates.Add(startState);
					workingStates.Add(startState);
				}

				auto ReuseState = [&](StateSymbol* state)
				{
					if (!ic.reusedStates.Contains(state))
					{
						ic.reusedStates.Add(state);
						workingStates.Add(state);
					}
				};

				auto ReuseEdge = [&](EdgeSymbol* edge)
				{
					ReuseState(edge->To());
					if (!ic.reusedEdges.Contains(edge))
					{
						ic.reusedEdges.Add(edge);
					}
				};

				auto ApplyEdgeToMergedState = [&](EdgeSymbol* edge, StateSymbol* mergedState)
				{
					ReuseState(edge->To());
					auto newEdge = Ptr(new EdgeSymbol(mergedState, edge->To()));
					ic.createdEdges.Add(newEdge);
					newEdge->input = edge->input;
					CopyFrom(newEdge->competitions, edge->competitions);
					CopyFrom(newEdge->insAfterInput, edge->insAfterInput);
				};

				for (vint i = 0; i < workingStates.Count(); i++)
				{
					auto currentState = workingStates[i];
					vint currentMergedToStateIndex = mergedToStates.Keys().IndexOf(currentState);

					Group<Tuple<EdgeInput, InsSymbolSet, CompetitionSymbolSet>, EdgeSymbol*> groupedEdges;
					if (currentMergedToStateIndex == -1)
					{
						// if the current state is an original state
						for (auto edge : currentState->OutEdges())
						{
							if (edge->input.type == EdgeInputType::Token || edge->input.type == EdgeInputType::Rule)
							{
								// only group Token or Rule edges
								groupedEdges.Add(
									{
										edge->input,
										InsSymbolSet{edge->insAfterInput},
										CompetitionSymbolSet(edge->competitions)
									}, edge);
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
									groupedEdges.Add(
										{
											edge->input,
											InsSymbolSet{edge->insAfterInput},
											CompetitionSymbolSet(edge->competitions)
										}, edge);
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
									targetSet.Add({ edge->To()->label,newStates.IndexOf(edge->To()),edge->To() });
								}

								vint index = statesToMerged.Keys().IndexOf(targetSet);
								if (index != -1)
								{
									mergedState = statesToMerged.Values()[index];
								}
								else
								{
									mergedState = Ptr(new StateSymbol(startState->Rule()));
									ic.createdStates.Add(mergedState);
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
							ic.createdEdges.Add(newEdge);

							newEdge->input = groupedKey.get<0>();
							CopyFrom(newEdge->insAfterInput, groupedKey.get<1>().Symbols());
							CopyFrom(newEdge->competitions, groupedKey.get<2>().Symbols());
						}
					}
				}

				ApplyIncrementalChange(ic, newStates, newEdges);
			}
		}
	}
}