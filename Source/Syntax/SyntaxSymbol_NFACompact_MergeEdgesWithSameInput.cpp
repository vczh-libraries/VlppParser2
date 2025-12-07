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
				/*
				* Two edges can be merged if:
				*   They are both Token or Rule
				*   They have the same input, insAfterInput and competitions
				*   returnEdges of an edge is empty at this moment so it is ignored
				* 
				* If a state has multiple outgoing edges that can be merged
				*   the target states will be merged into one
				*   all outgoing edges from target states will be recreated from the merged state
				*   mergable edges will be replaced by a new edge
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
				*                 +-(b)-> XY
				*                 |
				* A -+-(r)-> UVW -+-(c)-> Z
				*/

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
									// if the merged state already exists, reuse it
									mergedState = statesToMerged.Values()[index];
								}
								else
								{
									// otherwise, create a new merged state
									// its label is the combination of all originals
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

/***********************************************************************
SyntaxSymbolManager::MergeEdgesWithSameInputCrossReference
***********************************************************************/

			void SyntaxSymbolManager::MergeEdgesWithSameInputCrossReference(RuleSymbol* rule, StateSymbol* startState, StateList& newStates, EdgeList& newEdges)
			{
				/*
				* For any state A whose prefix calls look like:
				*   A -> r1 -> ... rn -> (u1, v1, ...)
				*   A -> s1 -> ... sn -> (u1, v1, ...)
				* The same (u1, v1, ...) will be merged.
				* Especially when A indirectly reuses (u1, v1, ...), this approach removes duplicated ambiguous results.
				*   by not doing this, it produces two duplicated A -> (different paths) -> (u1, v1, ...)
				*   and parsing efforts are wasted if (u1, v1, ...) costs a large number of transitions
				* 
				* Because a prefix will be produced and then passed into its parent rule like this:
				*   {CreateObject(u1)} ... {StackSlot(0), CreateObject(r1)} {StackSlot(0), CreateObject(A)}
				*   we can parse (u1, v1, ...) first, and then patch multiple EndingInput transitions directly into (rn, sn)
				*   such EndingInput transition will have non-empty returnEdges
				* 
				* (u1, v1, ...) are not required to be reuse clauses which only contain one rule
				*   as even if they are, they may be followed by LeftRec transitions, it doesn't make the work simpler, but even more checking
				*   recursive cross-reference merges will not be implemented at the moment
				*   treat it as a special case which only merge prefixes in the size of one rule
				* 
				* Pay attention to complex situation when prefix calls trees and only sub trees merge
				* 
				* This function checks all states with multiple outgoing Rule transitions
				*   if any mergable transitions are found, all outgoing Rule transitions will have to be reworked
				*   because if a sub tree merges with another
				*   all other roots of sub trees in all ancestors will have to be connected from that state
				*   e.g.
				*              +- r2 ..
				*              |      +- r4 ..
				*     A -> r1 -+- r3 -+  r5 ..
				*              |
				*              +- r6..
				*   when r4 merges, (r2, r5, r6) will need similar edges to start from A:
				*     A -+- r4 (injects into r3)
				*        +- r5 (injects into r3)
				*        +- r2 (injects into r1)
				*        +- r6 (injects into r1)
				*   only first level transitions are unaffected, all extra injections include Token from (r1, r3)
				*/
			}
		}
	}
}