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
				CHECK_ERROR(phase == SyntaxPhase::EpsilonNFA, L"vl::gre::parsergen::SyntaxSymbolManager::CreateState(RuleSymbol*)#Cannot change the automaton after calling BuildCompactSyntax().");
				auto symbol = new StateSymbol(rule);
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
		}
	}
}