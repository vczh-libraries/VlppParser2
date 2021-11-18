#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
StateSymbol
***********************************************************************/

			StateSymbol::StateSymbol(RuleSymbol* _rule)
				: ownerManager(_rule->Owner())
				, rule(_rule)
			{
			}

/***********************************************************************
EdgeSymbol
***********************************************************************/

			EdgeSymbol::EdgeSymbol(StateSymbol* _from, StateSymbol* _to)
				: ownerManager(_from->Owner())
				, fromState(_from)
				, toState(_to)
			{
				fromState->outEdges.Add(this);
				toState->inEdges.Add(this);
			}

/***********************************************************************
RuleSymbol
***********************************************************************/

			RuleSymbol::RuleSymbol(SyntaxSymbolManager* _ownerManager, const WString& _name)
				: ownerManager(_ownerManager)
				, name(_name)
			{
			}

/***********************************************************************
SyntaxSymbolManager
***********************************************************************/

			SyntaxSymbolManager::SyntaxSymbolManager(ParserSymbolManager& _global)
				: global(_global)
			{
			}

			RuleSymbol* SyntaxSymbolManager::CreateRule(const WString& name)
			{
				CHECK_ERROR(states.Count() + edges.Count() == 0, L"vl::gre::parsergen::SyntaxSymbolManager::CreateRule(const WString&)#Cannot create new rules after building the automaton.");
				auto rule = new RuleSymbol(this, name);
				if (!rules.Add(name, rule))
				{
					global.AddError(
						ParserErrorType::DuplicatedRule,
						name
						);
				}
				return rule;
			}

			StateSymbol* SyntaxSymbolManager::CreateState(RuleSymbol* rule)
			{
				CHECK_ERROR(!isCompact, L"vl::gre::parsergen::SyntaxSymbolManager::CreateState(RuleSymbol*)#Cannot change the automaton after calling BuildCompactSyntax().");
				auto symbol = new StateSymbol(rule);
				states.Add(symbol);
				return symbol;
			}

			EdgeSymbol* SyntaxSymbolManager::CreateEdge(StateSymbol* from, StateSymbol* to)
			{
				CHECK_ERROR(!isCompact, L"vl::gre::parsergen::SyntaxSymbolManager::CreateEdge(StateSymbol*, StateSymbol*)#Cannot change the automaton after calling BuildCompactSyntax().");
				auto symbol = new EdgeSymbol(from, to);
				edges.Add(symbol);
				return symbol;
			}

/***********************************************************************
SyntaxSymbolManager::BuildCompactSyntax
***********************************************************************/

			struct StateSymbolSet
			{
				SortedList<StateSymbol*> states;

				StateSymbolSet() = default;
				StateSymbolSet(StateSymbolSet&& set) : states(std::move(set.states)) {}
				StateSymbolSet(const StateSymbolSet&) = delete;
				StateSymbolSet& operator=(StateSymbolSet&& set) { states = std::move(set.states); return *this; }
				StateSymbolSet& operator=(const StateSymbolSet&) = delete;

				bool Add(StateSymbol* state)
				{
					if (states.Contains(state)) return false;
					states.Add(state);
					return true;
				}

				vint Compare(const StateSymbolSet& set) const
				{
					return CompareEnumerable(states, set.states);
				}

				bool operator==(const StateSymbolSet& set) const { return Compare(set) == 0; }
				bool operator!=(const StateSymbolSet& set) const { return Compare(set) != 0; }
				bool operator< (const StateSymbolSet& set) const { return Compare(set) < 0; }
				bool operator<=(const StateSymbolSet& set) const { return Compare(set) <= 0; }
				bool operator> (const StateSymbolSet& set) const { return Compare(set) > 0; }
				bool operator>=(const StateSymbolSet& set) const { return Compare(set) >= 0; }
			};

			class CompactSyntaxBuilder
			{
				using StateList = collections::List<Ptr<StateSymbol>>;
				using EdgeList = collections::List<Ptr<EdgeSymbol>>;
			protected:
				RuleSymbol*									rule;
				StateList&									newStates;
				EdgeList&									newEdges;
				Dictionary<StateSymbol*, StateSymbol*>		oldToNew;
				Dictionary<StateSymbolSet, StateSymbol*>	oldsToNew;
				Group<StateSymbol*, StateSymbol*>			newToOlds;

			public:
				CompactSyntaxBuilder(RuleSymbol* _rule, StateList& _newStates, EdgeList& _newEdges)
					: rule(_rule)
					, newStates(_newStates)
					, newEdges(_newEdges)
				{
				}

				StateSymbolSet CalculateEpsilonClosure(StateSymbol* state)
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
							if (edge->input.type == EdgeInputType::Epsilon && edge->insBefore.Count() == 0 && edge->insAfter.Count() == 0)
							{
								if (key.Add(edge->To()))
								{
									visited.Add(edge->To());
								}
							}
						}
					}

					return std::move(key);
				}

				StateSymbol* CreateCompactState(StateSymbolSet&& key)
				{
					vint index = oldsToNew.Keys().IndexOf(key);
					if (index != -1)
					{
						return oldsToNew.Values()[index];
					}

					auto newState = new StateSymbol(rule);
					newStates.Add(newState);

					for (auto oldState : key.states)
					{
						newToOlds.Add(newState, oldState);
					}
					oldsToNew.Add(std::move(key), newState);

					return newState;
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
			};

			StateSymbol* SyntaxSymbolManager::EliminateEpsilonEdges(RuleSymbol* rule, StateList& newStates, EdgeList& newEdges)
			{
				auto psuedoState = CreateState(rule);
				psuedoState->label = L"{START}";
				for (auto startState : rule->startStates)
				{
					CreateEdge(psuedoState, startState);
				}

				CompactSyntaxBuilder builder(rule, newStates, newEdges);
				auto compactStartState = builder.CreateCompactState(psuedoState);

				return compactStartState;
			}

			void SyntaxSymbolManager::BuildCompactSyntaxInternal()
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

			void SyntaxSymbolManager::BuildCompactSyntax()
			{
				CHECK_ERROR(!isCompact, L"vl::gre::parsergen::SyntaxSymbolManager::BuildCompactSyntax()#BuildCompactSyntax() can only be called once.");
				BuildCompactSyntaxInternal();
				isCompact = true;
			}
		}
	}
}