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

				// metadata.ruleNames
				List<RuleSymbol*> rulesInOrder;
				CopyFrom(rulesInOrder, From(rules.order).Select([this](auto&& name) { return rules.map[name]; }));
				metadata.ruleNames.Resize(rulesInOrder.Count());
				for (auto [rule, index] : indexed(rulesInOrder))
				{
					metadata.ruleNames[index] = rule->Name();
				}

				// metadata.stateLabels
				List<StateSymbol*> statesInOrder;
				GetStatesInStableOrder(statesInOrder);
				metadata.stateLabels.Resize(statesInOrder.Count());
				for (auto [state, index] : indexed(statesInOrder))
				{
					metadata.stateLabels[index] = GetStateGlobalLabel(state, index);
				}

				executable.tokenCount = tokenCount;
				executable.ruleCount = rulesInOrder.Count();

				// executable.ruleStartStates
				executable.ruleStartStates.Resize(rulesInOrder.Count());
				for (auto [rule, index] : indexed(rulesInOrder))
				{
					executable.ruleStartStates[index] = statesInOrder.IndexOf(rule->startStates[0]);
				}

				// executable.states
				executable.states.Resize(statesInOrder.Count());
				for (auto [state, index] : indexed(statesInOrder))
				{
					auto&& stateDesc = executable.states[index];
					stateDesc.rule = rulesInOrder.IndexOf(state->Rule());
					stateDesc.endingState = state->endingState;
				}

				List<EdgeSymbol*> edgesInOrder;
				List<EdgeSymbol*> returnEdgesInOrder;
				List<AstIns> instructionsInOrder;

				// executable.transitions
				vint inputCount = automaton::Executable::TokenBegin + tokenCount;
				executable.transitions.Resize(statesInOrder.Count() * inputCount);
				for (auto [state, stateIndex] : indexed(statesInOrder))
				{
					for (vint input = 0; input < inputCount; input++)
					{
						auto&& transition = executable.transitions[stateIndex * inputCount + input];
						auto orderedEdges = From(state->OutEdges())
							.Where([&](EdgeSymbol* edge)
							{
								switch (input)
								{
								case automaton::Executable::EndingInput:
									return edge->input.type == EdgeInputType::Ending;
								case automaton::Executable::LeftrecInput:
									return edge->input.type == EdgeInputType::LeftRec;
								default:
									return edge->input.type == EdgeInputType::Token && edge->input.token == input - automaton::Executable::TokenBegin;
								}
							})
							.OrderBy([&](EdgeSymbol* e1, EdgeSymbol* e2)
							{
								return statesInOrder.IndexOf(e1->To()) - statesInOrder.IndexOf(e2->To());
							});
						transition.start = edgesInOrder.Count();
						CopyFrom(edgesInOrder, orderedEdges, true);
						transition.count = edgesInOrder.Count() - transition.start;
						if (transition.count == 0)
						{
							transition.start = -1;
							continue;
						}
					}
				}

				// executable.edges
				executable.edges.Resize(edgesInOrder.Count());
				for (auto [edge, edgeIndex] : indexed(edgesInOrder))
				{
					auto&& edgeDesc = executable.edges[edgeIndex];
					edgeDesc.fromState = statesInOrder.IndexOf(edge->From());
					edgeDesc.toState = statesInOrder.IndexOf(edge->To());

					edgeDesc.insBeforeInput.start = instructionsInOrder.Count();
					CopyFrom(instructionsInOrder, edge->insBeforeInput, true);
					edgeDesc.insBeforeInput.count = instructionsInOrder.Count() - edgeDesc.insBeforeInput.start;

					edgeDesc.insAfterInput.start = instructionsInOrder.Count();
					CopyFrom(instructionsInOrder, edge->insBeforeInput, true);
					edgeDesc.insAfterInput.count = instructionsInOrder.Count() - edgeDesc.insAfterInput.start;

					edgeDesc.returns.start = returnEdgesInOrder.Count();
					CopyFrom(returnEdgesInOrder, edge->returnEdges, true);
					edgeDesc.returns.count = returnEdgesInOrder.Count() - edgeDesc.returns.start;

					if (edgeDesc.insBeforeInput.count == 0) edgeDesc.insBeforeInput.start = -1;
					if (edgeDesc.insAfterInput.count == 0) edgeDesc.insAfterInput.start = -1;
					if (edgeDesc.returns.count == 0) edgeDesc.returns.start = -1;
				}

				// executable.returns
				executable.returns.Resize(returnEdgesInOrder.Count());
				for (auto [edge, edgeIndex] : indexed(returnEdgesInOrder))
				{
					auto&& returnDesc = executable.returns[edgeIndex];
					returnDesc.returnState = statesInOrder.IndexOf(edge->To());

					returnDesc.insAfterInput.start = instructionsInOrder.Count();
					CopyFrom(instructionsInOrder, edge->insBeforeInput, true);
					returnDesc.insAfterInput.count = instructionsInOrder.Count() - returnDesc.insAfterInput.start;
					if (returnDesc.insAfterInput.count == 0) returnDesc.insAfterInput.start = -1;
				}

				// executable.instructions
				CopyFrom(executable.instructions, instructionsInOrder);
			}
		}
	}
}