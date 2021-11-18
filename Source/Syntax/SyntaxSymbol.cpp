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
				StateSymbolSet(StateSymbolSet&&) = default;
				StateSymbolSet(const StateSymbolSet&) = delete;
				StateSymbolSet& operator=(StateSymbolSet&&) = default;
				StateSymbolSet& operator=(const StateSymbolSet&) = delete;

				void Add(StateSymbol* state)
				{
					if (!states.Contains(state))
					{
						states.Add(state);
					}
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

			StateSymbol* SyntaxSymbolManager::EliminateEpsilonEdges(RuleSymbol* rule, StateList& newStates, EdgeList& newEdges)
			{
				auto psuedoState = CreateState(rule);
				for (auto startState : rule->startStates)
				{
					CreateEdge(psuedoState, startState);
				}
				throw 0;
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