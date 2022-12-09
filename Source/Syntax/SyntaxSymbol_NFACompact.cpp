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
						states = Ptr(new SortedList<StateSymbol*>);
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
						case EdgeInputType::LrPlaceholder:
						case EdgeInputType::LrInject:
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
						auto newState = Ptr(new StateSymbol(rule, state->ClauseId()));
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
				newEdge->important |= endingEdge->important;
				newEdge->important |= lrecPrefixEdge->important;

				newEdge->input.type = EdgeInputType::LeftRec;
				CopyFrom(newEdge->insBeforeInput, endingEdge->insBeforeInput, true);
				CopyFrom(newEdge->insAfterInput, endingEdge->insAfterInput, true);
				CopyFrom(newEdge->insBeforeInput, lrecPrefixEdge->insBeforeInput, true);
				CopyFrom(newEdge->insAfterInput, lrecPrefixEdge->insAfterInput, true);

				for (vint i = newEdge->insBeforeInput.Count() - 1; i >= 0; i--)
				{
					if (newEdge->insBeforeInput[i].type == AstInsType::BeginObject)
					{
						newEdge->insBeforeInput.Insert(i, { AstInsType::LriStore });
						newEdge->insBeforeInput.Insert(i + 2, { AstInsType::LriFetch });
					}
				}
				for (vint i = newEdge->insAfterInput.Count() - 1; i >= 0; i--)
				{
					if (newEdge->insAfterInput[i].type == AstInsType::BeginObject)
					{
						newEdge->insBeforeInput.Insert(i, { AstInsType::LriStore });
						newEdge->insBeforeInput.Insert(i + 2, { AstInsType::LriFetch });
					}
				}
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
SyntaxSymbolManager::EliminateSingleRulePrefix
***********************************************************************/

			void SyntaxSymbolManager::EliminateSingleRulePrefix(RuleSymbol* rule, StateSymbol* startState, StateSymbol* endState, StateList& newStates, EdgeList& newEdges)
			{
				/*
				* Move the single rule prefix from the rule begin state
				* if there is any single rule clause consist of the same rule
				*
				* [BEFORE]
				*    +-(x)-> A --------(ending)-+
				*    |                          |
				* S -+-(x)-> ... -> B -(ending)-+-> E
				*    |                          |
				*    +-(x)-> ... -> C -(ending)-+
				*
				* [AFTER]
				*            +-(leftrec)-> ... -> B -(ending)---+
				*            |                                  v
				* S-(x)-> A -+-----------------------(ending)-> E
				*            |                                  ^
				*            +-(leftrec)-> ... -> C -(ending)---+
				*/

				Group<RuleSymbol*, EdgeSymbol*> prefixEdges;
				List<EdgeSymbol*> continuationEdges, eliminatedEdges;

				// identify prefix edge and continuation edge
				// prefix edges are clauses (x)
				// continuation edges are all qualified clauses with prefix (x) except prefix edges
				for (auto edge : startState->OutEdges())
				{
					if (edge->input.type != EdgeInputType::Rule) continue;
					if (edge->input.rule == rule) continue;
					auto state = edge->To();
					if (state->InEdges().Count() > 1) continue;

					if (state->OutEdges().Count() == 1 && state->OutEdges()[0]->input.type == EdgeInputType::Ending)
					{
						prefixEdges.Add(edge->input.rule, edge);
					}
					else
					{
						continuationEdges.Add(edge);
					}
				}

				for (auto [ruleSymbol, prefixIndex] : indexed(prefixEdges.Keys()))
				{
					auto&& prefixEdgesOfRule = prefixEdges.GetByIndex(prefixIndex);
					CHECK_ERROR(prefixEdgesOfRule.Count() == 1, L"<EliminateSingleRulePrefix>Multiple prefix edges under the same rule is not supported yet.");

					// TODO:
					// prefixEdge means the clause could consume only one rule
					// multiple prefixEdge could be
					//   the rule has multiple such clauses
					//   there is one clause but it looks like "([a] | [b]) c"
					//     where both [a] and [b] create an epsilon edge to c
					//     and after removing epsilon edges they become both edge consuming c
					// in this case we need to create a prefix edges to replace all others
					// it also means unresolvable ambiguity
					// maybe a better solution is to define it as a kind of invalid syntax
				}

				// for all prefixEdge and continuationEdge under the same rule
				// if their insBeforeInput are different
				// move prefixEdge's insBeforeInput to insAfterInput with help from LriStore and LriFetch
				SortedList<RuleSymbol*> compatibleInsBeforeInputPrefixRules;
				for (auto [ruleSymbol, prefixIndex] : indexed(prefixEdges.Keys()))
				{
					// see if all prefixEdges are compatible
					auto&& prefixEdgesOfRule = prefixEdges.GetByIndex(prefixIndex);
					auto prefixEdge = prefixEdgesOfRule[0];
					for (auto otherPrefixEdge : From(prefixEdgesOfRule).Skip(1))
					{
						if (CompareEnumerable(prefixEdge->insBeforeInput, otherPrefixEdge->insBeforeInput) != 0)
						{
							goto INCOMPATIBLE;
						}
					}

					// see if all continuationEdges are compatible
					for (auto continuationEdge : continuationEdges)
					{
						if (continuationEdge->input.rule == prefixEdge->input.rule)
						{
							if (CompareEnumerable(prefixEdge->insBeforeInput, continuationEdge->insBeforeInput) != 0)
							{
								goto INCOMPATIBLE;
							}
						}
					}

					compatibleInsBeforeInputPrefixRules.Add(ruleSymbol);
				INCOMPATIBLE:;
				}

				// for all prefixEdge that fails the above test
				// combine insBeforeInput with insAfterInput with the help from LriStore and LriFetch
				// properly move instructions from prefixEdge to endingEdge
				for (auto [ruleSymbol, prefixIndex] : indexed(prefixEdges.Keys()))
				{
					bool compatible = compatibleInsBeforeInputPrefixRules.Contains(ruleSymbol);
					for (auto prefixEdge : prefixEdges.GetByIndex(prefixIndex))
					{
						List<AstIns> ins;
						if (!compatible && prefixEdge->insBeforeInput.Count() > 0)
						{
							ins.Add({ AstInsType::LriStore });
							CopyFrom(ins, prefixEdge->insBeforeInput, true);
							ins.Add({ AstInsType::LriFetch });
							prefixEdge->insBeforeInput.Clear();
						}
						CopyFrom(ins, prefixEdge->insAfterInput, true);
						prefixEdge->insAfterInput.Clear();

						auto endingEdge = prefixEdge->To()->OutEdges()[0];
						CopyFrom(ins, endingEdge->insBeforeInput, true);
						CopyFrom(endingEdge->insBeforeInput, ins);
					}
				}

				// for all qualified continuationEdge
				// create a new edge to run continuationEdge's instruction properly after prefixEdge
				// remove continuationEdge
				for (auto continuationEdge : continuationEdges)
				{
					vint prefixIndex = prefixEdges.Keys().IndexOf(continuationEdge->input.rule);
					if (prefixIndex == -1) continue;

					bool compatible = compatibleInsBeforeInputPrefixRules.Contains(continuationEdge->input.rule);
					bool eliminated = false;
					for (auto prefixEdge : prefixEdges.GetByIndex(prefixIndex))
					{
						// important and insSwitch happen before shifting into the rule
						if (continuationEdge->important != prefixEdge->important) continue;

						eliminated = true;
						auto state = prefixEdge->To();
						auto newEdge = Ptr(new EdgeSymbol(state, continuationEdge->To()));
						newEdges.Add(newEdge);

						newEdge->input.type = EdgeInputType::LeftRec;
						newEdge->important = continuationEdge->important;
						if (compatible)
						{
							CopyFrom(newEdge->insAfterInput, continuationEdge->insAfterInput);
						}
						else if (continuationEdge->insBeforeInput.Count() > 0)
						{
							// for incompatible continuationEdge
							// combine insBeforeInput with insAfterInput with the help from LriStore and LriFetch
							newEdge->insAfterInput.Add({ AstInsType::LriStore });
							CopyFrom(newEdge->insAfterInput, continuationEdge->insBeforeInput, true);
							newEdge->insAfterInput.Add({ AstInsType::LriFetch });
							CopyFrom(newEdge->insAfterInput, continuationEdge->insAfterInput, true);
						}
					}

					if (eliminated)
					{
						eliminatedEdges.Add(continuationEdge);
					}
				}

				for (auto eliminatedEdge : eliminatedEdges)
				{
					vint prefixIndex = prefixEdges.Keys().IndexOf(eliminatedEdge->input.rule);
					if (prefixIndex == -1) continue;
					eliminatedEdge->From()->outEdges.Remove(eliminatedEdge);
					eliminatedEdge->To()->inEdges.Remove(eliminatedEdge);
					newEdges.Remove(eliminatedEdge);
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
				auto psuedoState = CreateState(rule, -1);
				for (auto startState : rule->startStates)
				{
					CreateEdge(psuedoState, startState);
				}

				CompactSyntaxBuilder builder(rule, newStates, newEdges);
				auto compactStartState = builder.CreateCompactState(psuedoState);
				compactStartState->label = L" BEGIN ";

				auto compactEndState = Ptr(new StateSymbol(rule, -1));
				compactEndState->label = L" END ";
				compactEndState->endingState = true;
				newStates.Add(compactEndState);

				List<StateSymbol*> visited;
				visited.Add(compactStartState);

				// all epsilon-NFAs of its clauses become one connected epsilon-NFA of this rule
				// we can build the compact-NFA out of this epsilon-NFA starting from the start state
				for (vint i = 0; i < visited.Count(); i++)
				{
					auto current = visited[i];
					builder.BuildEpsilonEliminatedEdges(current, compactEndState.Obj(), visited);
				}

				// optimize
				EliminateLeftRecursion(rule, compactStartState, compactEndState.Obj(), newStates, newEdges);
				EliminateSingleRulePrefix(rule, compactStartState, compactEndState.Obj(), newStates, newEdges);

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

				// only when a state has any important out edge
				// its out edges are marked accordingly
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