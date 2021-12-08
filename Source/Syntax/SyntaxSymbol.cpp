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

			StateSymbol::StateSymbol(RuleSymbol* _rule, vint32_t _clauseId)
				: ownerManager(_rule->Owner())
				, rule(_rule)
				, clauseId(_clauseId)
			{
			}

			void StateSymbol::GetOutEdgesInStableOrder(collections::List<StateSymbol*>& orderedStates, EdgeList& orderedEdges)
			{
				CopyFrom(orderedEdges, From(outEdges)
					.OrderBy([&](EdgeSymbol* e1, EdgeSymbol* e2)
					{
						vint result = 0;
						if (e1->input.type != e2->input.type)
						{
							result = (vint)e1->input.type - (vint)e2->input.type;
						}
						else
						{
							switch (e1->input.type)
							{
							case EdgeInputType::Token:
								result = e1->input.token - e2->input.token;
								break;
							case EdgeInputType::Rule:
								result = ownerManager->RuleOrder().IndexOf(e1->input.rule->Name()) - ownerManager->RuleOrder().IndexOf(e2->input.rule->Name());
								break;
							default:;
							}
						}

						if (result != 0) return result;
						return orderedStates.IndexOf(e1->To()) - orderedStates.IndexOf(e2->To());
					}));
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

			StateSymbol* SyntaxSymbolManager::CreateState(RuleSymbol* rule, vint32_t clauseId)
			{
				CHECK_ERROR(phase == SyntaxPhase::EpsilonNFA, L"vl::gre::parsergen::SyntaxSymbolManager::CreateState(RuleSymbol*)#Cannot change the automaton after calling BuildCompactSyntax().");
				auto symbol = new StateSymbol(rule, clauseId);
				states.Add(symbol);
				return symbol;
			}

			EdgeSymbol* SyntaxSymbolManager::CreateEdge(StateSymbol* from, StateSymbol* to)
			{
				CHECK_ERROR(phase == SyntaxPhase::EpsilonNFA, L"vl::gre::parsergen::SyntaxSymbolManager::CreateEdge(StateSymbol*, StateSymbol*)#Cannot change the automaton after calling BuildCompactSyntax().");
				auto symbol = new EdgeSymbol(from, to);
				edges.Add(symbol);
				return symbol;
			}

			void SyntaxSymbolManager::BuildCompactNFA()
			{
				CHECK_ERROR(global.Errors().Count() == 0, L"vl::gre::parsergen::SyntaxSymbolManager::BuildCompactSyntax()#BuildCompactNFA() cannot be called before errors are resolved.");
				CHECK_ERROR(phase == SyntaxPhase::EpsilonNFA, L"vl::gre::parsergen::SyntaxSymbolManager::BuildCompactSyntax()#BuildCompactNFA() can only be called on epsilon NFA.");
				BuildCompactNFAInternal();
				phase = SyntaxPhase::CompactNFA;
			}

			void SyntaxSymbolManager::BuildCrossReferencedNFA()
			{
				CHECK_ERROR(global.Errors().Count() == 0, L"vl::gre::parsergen::SyntaxSymbolManager::BuildCompactSyntax()#BuildCrossReferencedNFA() cannot be called before errors are resolved.");
				CHECK_ERROR(phase == SyntaxPhase::CompactNFA, L"vl::gre::parsergen::SyntaxSymbolManager::BuildCompactSyntax()#BuildCrossReferencedNFA() can only be called on compact NFA.");
				BuildCrossReferencedNFAInternal();
				phase = SyntaxPhase::CrossReferencedNFA;
			}

			void SyntaxSymbolManager::GetStatesInStableOrder(collections::List<StateSymbol*>& order)
			{
				Group<RuleSymbol*, StateSymbol*> groupedStates;
				{
					List<StateSymbol*> visited;
					for (auto ruleName : rules.order)
					{
						auto ruleSymbol = rules.map[ruleName];
						for (auto startState : ruleSymbol->startStates)
						{
							if (!visited.Contains(startState))
							{
								vint startIndex = visited.Add(startState);
								for (vint i = startIndex; i < visited.Count(); i++)
								{
									auto state = visited[i];
									groupedStates.Add(state->Rule(), state);
									for (auto edge : state->OutEdges())
									{
										auto target = edge->To();
										if (!visited.Contains(target))
										{
											visited.Add(target);
										}
									}
								}
							}
						}
					}
				}
				{
					vint counter = 0;
					for (auto ruleName : rules.order)
					{
						auto ruleSymbol = rules.map[ruleName];
						auto orderedStates = From(groupedStates[ruleSymbol])
							.OrderBy([](StateSymbol* s1, StateSymbol* s2)
							{
								return WString::Compare(s1->label, s2->label);
							});
						for (auto state : orderedStates)
						{
							order.Add(state);
						}
					}
				}
			}

			WString SyntaxSymbolManager::GetStateGlobalLabel(StateSymbol* state, vint index)
			{
				return L"[" + itow(index) + L"][" + state->Rule()->Name() + L"]" + state->label + (state->endingState ? L"[ENDING]" : L"");
			}
		}
	}
}