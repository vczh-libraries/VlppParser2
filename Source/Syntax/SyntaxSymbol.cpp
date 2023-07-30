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
						std::strong_ordering result = e1->input.type <=> e2->input.type;
						if (result == 0)
						{
							switch (e1->input.type)
							{
							case EdgeInputType::Token:
								result = e1->input.token <=> e2->input.token;
								if (result == 0)
								{
									if (e1->input.condition && e2->input.condition)
									{
										result = e1->input.condition.Value() <=> e2->input.condition.Value();
									}
									else if (e1->input.condition)
									{
										result = std::strong_ordering::greater;
									}
									else if (e2->input.condition)
									{
										result = std::strong_ordering::less;
									}
								}
								break;
							case EdgeInputType::Rule:
								result = ownerManager->RuleOrder().IndexOf(e1->input.rule->Name()) <=> ownerManager->RuleOrder().IndexOf(e2->input.rule->Name());
								break;
							default:;
							}
						}

						if (result != 0) return result;
						return orderedStates.IndexOf(e1->To()) <=> orderedStates.IndexOf(e2->To());
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

			RuleSymbol::RuleSymbol(SyntaxSymbolManager* _ownerManager, const WString& _name, vint _fileIndex)
				: ownerManager(_ownerManager)
				, name(_name)
				, fileIndex(_fileIndex)
			{
			}

/***********************************************************************
SyntaxSymbolManager
***********************************************************************/

			SyntaxSymbolManager::SyntaxSymbolManager(ParserSymbolManager& _global)
				: global(_global)
			{
			}

			RuleSymbol* SyntaxSymbolManager::CreateRule(const WString& name, vint fileIndex, bool isPublic, bool isParser, ParsingTextRange codeRange)
			{
				CHECK_ERROR(states.Count() + edges.Count() == 0, L"vl::gre::parsergen::SyntaxSymbolManager::CreateRule(const WString&)#Cannot create new rules after building the automaton.");
				auto rule = new RuleSymbol(this, name, fileIndex);
				if (!rules.Add(name, rule))
				{
					AddError(
						ParserErrorType::DuplicatedRule,
						codeRange,
						name
						);
				}
				rule->isPublic = isPublic;
				rule->isParser = isParser;
				return rule;
			}

			void SyntaxSymbolManager::RemoveRule(const WString& name)
			{
				rules.Remove(name);
			}

			StateSymbol* SyntaxSymbolManager::CreateState(RuleSymbol* rule, vint32_t clauseId)
			{
				CHECK_ERROR(phase == SyntaxPhase::EpsilonNFA, L"vl::gre::parsergen::SyntaxSymbolManager::CreateState(RuleSymbol*)#Cannot change the automaton after calling BuildCompactSyntax().");
				auto symbol = Ptr(new StateSymbol(rule, clauseId));
				states.Add(symbol);
				return symbol.Obj();
			}

			EdgeSymbol* SyntaxSymbolManager::CreateEdge(StateSymbol* from, StateSymbol* to)
			{
				CHECK_ERROR(phase == SyntaxPhase::EpsilonNFA, L"vl::gre::parsergen::SyntaxSymbolManager::CreateEdge(StateSymbol*, StateSymbol*)#Cannot change the automaton after calling BuildCompactSyntax().");
				auto symbol = Ptr(new EdgeSymbol(from, to));
				edges.Add(symbol);
				return symbol.Obj();
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
								// TODO: (enumerable) Linq:Skip
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
							.OrderByKey([](StateSymbol* s)
							{
								return s->label;
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