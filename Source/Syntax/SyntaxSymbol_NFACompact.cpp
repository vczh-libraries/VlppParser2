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
									CopyFrom(newEdge->insSwitch, accumulatedEdge->insSwitch, true);
									CopyFrom(newEdge->insBeforeInput, accumulatedEdge->insBeforeInput, true);
									CopyFrom(newEdge->insAfterInput, accumulatedEdge->insAfterInput, true);
									newEdge->important |= accumulatedEdge->important;
								}
							}
							break;
						case EdgeInputType::Epsilon:
							BuildEpsilonEliminatedEdgesInternal(edge->To(), newState, endState, visited, accumulatedEdges);
							break;
						default:;
						}
						accumulatedEdges.RemoveAt(accumulatedEdges.Count() - 1);
					}

					if (walkingOldState->endingState)
					{
						auto newEdge = new EdgeSymbol(newState, endState);
						newEdge->input.type = EdgeInputType::Ending;
						for (auto accumulatedEdge : accumulatedEdges)
						{
							CopyFrom(newEdge->insSwitch, accumulatedEdge->insSwitch, true);
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
						auto newState = new StateSymbol(rule, state->ClauseId());
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
SyntaxSymbolManager::CreateLeftRecEdge
***********************************************************************/

			void SyntaxSymbolManager::BuildLeftRecEdge(EdgeSymbol* newEdge, EdgeSymbol* endingEdge, EdgeSymbol* lrecPrefixEdge)
			{
				newEdge->important |= endingEdge->important;
				newEdge->important |= lrecPrefixEdge->important;

				newEdge->input.type = EdgeInputType::LeftRec;
				CopyFrom(newEdge->insSwitch, endingEdge->insSwitch, true);
				CopyFrom(newEdge->insBeforeInput, endingEdge->insBeforeInput, true);
				CopyFrom(newEdge->insAfterInput, endingEdge->insAfterInput, true);
				CopyFrom(newEdge->insSwitch, lrecPrefixEdge->insSwitch, true);
				CopyFrom(newEdge->insBeforeInput, lrecPrefixEdge->insBeforeInput, true);
				CopyFrom(newEdge->insAfterInput, lrecPrefixEdge->insAfterInput, true);

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

					bool hasPushIns = false;
					bool hasTestIns = false;
					for (auto ins : edge->insSwitch)
					{
						if (ins.type < automaton::SwitchInsType::ConditionRead)
						{
							hasPushIns = true;
						}
						else
						{
							hasTestIns = true;
						}
					}

					if (hasPushIns)
					{
						AddError(
							ParserErrorType::LeftRecursiveClauseInsidePushCondition,
							{},
							rule->Name()
							);
					}
					if (hasTestIns)
					{
						AddError(
							ParserErrorType::LeftRecursiveClauseInsideTestCondition,
							{},
							rule->Name()
							);
					}
				}

				for (auto lrecEdge : lrecEdges)
				{
					for (auto endingEdge : endState->InEdges())
					{
						auto state = endingEdge->From();
						auto newEdge = new EdgeSymbol(state, lrecEdge->To());
						newEdges.Add(newEdge);
						BuildLeftRecEdge(newEdge, endingEdge, lrecEdge);
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
				List<EdgeSymbol*> continuationEdges;

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

				for (auto continuationEdge : continuationEdges)
				{
					vint prefixIndex = prefixEdges.Keys().IndexOf(continuationEdge->input.rule);
					if (prefixIndex == -1) continue;
					for (auto prefixEdge : prefixEdges.GetByIndex(prefixIndex))
					{
						auto state = prefixEdge->To();
						auto endingEdge = state->OutEdges()[0];
						auto newEdge = new EdgeSymbol(state, continuationEdge->To());
						newEdges.Add(newEdge);
						BuildLeftRecEdge(newEdge, endingEdge, continuationEdge);
					}
				}

				for (auto continuationEdge : continuationEdges)
				{
					vint prefixIndex = prefixEdges.Keys().IndexOf(continuationEdge->input.rule);
					if (prefixIndex == -1) continue;
					continuationEdge->From()->outEdges.Remove(continuationEdge);
					continuationEdge->To()->inEdges.Remove(continuationEdge);
					newEdges.Remove(continuationEdge);
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

				auto compactEndState = new StateSymbol(rule, -1);
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
					builder.BuildEpsilonEliminatedEdges(current, compactEndState, visited);
				}

				// optimize
				EliminateLeftRecursion(rule, compactStartState, compactEndState, newStates, newEdges);
				EliminateSingleRulePrefix(rule, compactStartState, compactEndState, newStates, newEdges);

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