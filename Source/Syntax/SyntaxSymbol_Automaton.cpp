#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;

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

				// metadata.switchNames and executable.switchDefaultValues
				if (switches.Count() > 0)
				{
					metadata.switchNames.Resize(switches.Count());
					executable.switchDefaultValues.Resize(switches.Count());
					for (auto [pair, index] : indexed(switches))
					{
						metadata.switchNames[index] = pair.key;
						executable.switchDefaultValues[index] = pair.value.defaultValue;
					}
				}

				executable.tokenCount = (vint32_t)tokenCount;
				executable.ruleCount = (vint32_t)rulesInOrder.Count();

				// executable.ruleStartStates
				executable.ruleStartStates.Resize(rulesInOrder.Count());
				for (auto [rule, index] : indexed(rulesInOrder))
				{
					executable.ruleStartStates[index] = (vint32_t)statesInOrder.IndexOf(rule->startStates[0]);
				}

				// executable.states
				executable.states.Resize(statesInOrder.Count());
				for (auto [state, index] : indexed(statesInOrder))
				{
					auto&& stateDesc = executable.states[index];
					stateDesc.rule = (vint32_t)rulesInOrder.IndexOf(state->Rule());
					stateDesc.clause = state->ClauseId();
					stateDesc.endingState = state->endingState;
				}

				List<EdgeSymbol*> edgesInOrder;
				List<EdgeSymbol*> returnEdgesInOrder;
				List<vint32_t> returnIndicesInOrder;
				List<automaton::SwitchIns> switchInsInOrder;
				List<AstIns> astInsInOrder;

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
						transition.start = (vint32_t)edgesInOrder.Count();
						CopyFrom(edgesInOrder, orderedEdges, true);
						transition.count = (vint32_t)edgesInOrder.Count() - transition.start;
						if (transition.count == 0)
						{
							transition.start = -1;
							continue;
						}
					}
				}

				// executable.stringLiteralBuffer
				{
					MemoryStream stringLiteralBuffer;
					{
						StreamWriter stringLiteralWriter(stringLiteralBuffer);
						for (auto edge : edgesInOrder)
						{
							if (edge->input.condition)
							{
								stringLiteralWriter.WriteString(edge->input.condition.Value());
							}
						}
					}
					{
						stringLiteralBuffer.SeekFromBegin(0);
						StreamReader stringLiteralReader(stringLiteralBuffer);
						executable.stringLiteralBuffer = stringLiteralReader.ReadToEnd();
					}
				}

				// executable.edges
				executable.edges.Resize(edgesInOrder.Count());
				vint32_t stringLiteralIndex = 0;
				for (auto [edge, edgeIndex] : indexed(edgesInOrder))
				{
					auto&& edgeDesc = executable.edges[edgeIndex];
					edgeDesc.fromState = (vint32_t)statesInOrder.IndexOf(edge->From());
					edgeDesc.toState = (vint32_t)statesInOrder.IndexOf(edge->To());
					if (edge->input.condition)
					{
						vint32_t length = (vint32_t)edge->input.condition.Value().Length();
						edgeDesc.condition.start = stringLiteralIndex;
						edgeDesc.condition.count = length;
						stringLiteralIndex += length;
					}

					switch (edge->importancy)
					{
					case EdgeImportancy::HighPriority:
						edgeDesc.priority = automaton::EdgePriority::HighPriority;
						break;
					case EdgeImportancy::LowPriority:
						edgeDesc.priority = automaton::EdgePriority::LowPriority;
						break;
					default:;
					}

					edgeDesc.insSwitch.start = (vint32_t)switchInsInOrder.Count();
					CopyFrom(switchInsInOrder, edge->insSwitch, true);
					edgeDesc.insSwitch.count = (vint32_t)switchInsInOrder.Count() - edgeDesc.insSwitch.start;

					edgeDesc.insBeforeInput.start = (vint32_t)astInsInOrder.Count();
					CopyFrom(astInsInOrder, edge->insBeforeInput, true);
					edgeDesc.insBeforeInput.count = (vint32_t)astInsInOrder.Count() - edgeDesc.insBeforeInput.start;

					edgeDesc.insAfterInput.start = (vint32_t)astInsInOrder.Count();
					CopyFrom(astInsInOrder, edge->insAfterInput, true);
					edgeDesc.insAfterInput.count = (vint32_t)astInsInOrder.Count() - edgeDesc.insAfterInput.start;

					edgeDesc.returnIndices.start = (vint32_t)returnIndicesInOrder.Count();
					for (auto returnEdge : edge->returnEdges)
					{
						vint index = returnEdgesInOrder.IndexOf(returnEdge);
						if (index == -1)
						{
							index = returnEdgesInOrder.Add(returnEdge);
						}
						returnIndicesInOrder.Add((vint32_t)index);
					}
					edgeDesc.returnIndices.count = (vint32_t)returnIndicesInOrder.Count() - edgeDesc.returnIndices.start;

					if (edgeDesc.insSwitch.count == 0) edgeDesc.insSwitch.start = -1;
					if (edgeDesc.insBeforeInput.count == 0) edgeDesc.insBeforeInput.start = -1;
					if (edgeDesc.insAfterInput.count == 0) edgeDesc.insAfterInput.start = -1;
					if (edgeDesc.returnIndices.count == 0) edgeDesc.returnIndices.start = -1;
				}

				// executable.returns
				executable.returns.Resize(returnEdgesInOrder.Count());
				for (auto [edge, edgeIndex] : indexed(returnEdgesInOrder))
				{
					auto&& returnDesc = executable.returns[edgeIndex];
					returnDesc.consumedRule = (vint32_t)rulesInOrder.IndexOf(edge->input.rule);
					returnDesc.returnState = (vint32_t)statesInOrder.IndexOf(edge->To());
					switch (edge->importancy)
					{
					case EdgeImportancy::HighPriority:
						returnDesc.priority = automaton::EdgePriority::HighPriority;
						break;
					case EdgeImportancy::LowPriority:
						returnDesc.priority = automaton::EdgePriority::LowPriority;
						break;
					default:;
					}

					returnDesc.ruleType = edge->input.ruleType;
					returnDesc.insAfterInput.start = (vint32_t)astInsInOrder.Count();
					CopyFrom(astInsInOrder, edge->insAfterInput, true);
					returnDesc.insAfterInput.count = (vint32_t)astInsInOrder.Count() - returnDesc.insAfterInput.start;
					if (returnDesc.insAfterInput.count == 0) returnDesc.insAfterInput.start = -1;
				}

				// executable.returnIndices
				CopyFrom(executable.returnIndices, returnIndicesInOrder);

				// executable.switchInstructions
				CopyFrom(executable.switchInstructions, switchInsInOrder);

				// executable.astInstructions
				CopyFrom(executable.astInstructions, astInsInOrder);
			}
		}
	}
}