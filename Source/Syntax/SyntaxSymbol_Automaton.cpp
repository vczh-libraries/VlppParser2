#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
SyntaxSymbolManager::BuildAutomaton
***********************************************************************/

			void SyntaxSymbolManager::BuildAutomaton(vint tokenCount, automaton::Executable& executable, automaton::Metadata& metadata)
			{
				CHECK_ERROR(global.Errors().Count() == 0, L"vl::gre::parsergen::SyntaxSymbolManager::BuildAutomaton(Executable&, Metadata&)#BuildAutomaton() cannot be called before errors are resolved.");
				CHECK_ERROR(phase == SyntaxPhase::CrossReferencedNFA, L"vl::gre::parsergen::SyntaxSymbolManager::BuildAutomaton(Executable&, Metadata&)#BuildAutomaton() can only be called on cross referenced NFA.");

				List<RuleSymbol*> rulesInOrder;
				CopyFrom(rulesInOrder, From(rules.order).Select([this](auto&& name) { return rules.map[name]; }));
				metadata.ruleNames.Resize(rulesInOrder.Count());
				for (auto [rule, index] : indexed(rulesInOrder))
				{
					metadata.ruleNames[index] = rule->Name();
				}

				List<StateSymbol*> statesInOrder;
				GetStatesInStableOrder(statesInOrder);
				metadata.stateLabels.Resize(statesInOrder.Count());
				for (auto [state, index] : indexed(statesInOrder))
				{
					metadata.stateLabels[index] = GetStateGlobalLabel(state, index);
				}
			}
		}
	}
}