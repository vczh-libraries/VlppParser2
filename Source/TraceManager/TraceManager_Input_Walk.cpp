#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{

/***********************************************************************
TraceManager::IsQualifiedTokenForCondition
***********************************************************************/

			bool TraceManager::IsQualifiedTokenForCondition(regex::RegexToken* token, StringLiteral condition)
			{
				if (condition.start == -1) return true;
				if (token->length != condition.count) return false;
				auto reading = executable.stringLiteralBuffer.Buffer();
				if (memcmp(token->reading, reading + condition.start, sizeof(wchar_t) * condition.count) != 0) return false;
				return true;
			}

/***********************************************************************
TraceManager::IsQualifiedTokenForEdgeArray
***********************************************************************/

			bool TraceManager::IsQualifiedTokenForEdgeArray(regex::RegexToken* token, EdgeArray& edgeArray)
			{
				for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint32_t byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[byEdge];
					if (IsQualifiedTokenForCondition(token, edgeDesc.condition)) return true;
				}
				return false;
			}

/***********************************************************************
TraceManager::RunEdgeConditionChecking
***********************************************************************/

			vint32_t TraceManager::PushSwitchFrame(Switches* currentSV, vuint32_t* values)
			{
				auto previous = currentSV->allocatedIndex;
				vint32_t& checking = currentSV->firstChild;
				while (checking != -1)
				{
					currentSV = switches.Get(checking);
					checking = currentSV->nextSibling;
					if (memcmp(values, currentSV->values, sizeof(currentSV->values)) == 0)
					{
						return currentSV->allocatedIndex;
					}
				}

				checking = switches.Allocate();
				auto newSV = switches.Get(checking);
				newSV->previous = previous;
				memcpy(newSV->values, values, sizeof(newSV->values));
				return checking;
			}

/***********************************************************************
TraceManager::RunEdgeConditionChecking
***********************************************************************/

			vint32_t TraceManager::RunEdgeConditionChecking(vint32_t currentSwitchValues, EdgeDesc& edgeDesc)
			{
				if (edgeDesc.insSwitch.count == 0) return currentSwitchValues;

				constexpr vint32_t ComponentCount = sizeof(static_cast<Switches*>(nullptr)->values) / sizeof(vuint32_t);
				vuint32_t values[ComponentCount];

				auto currentSV = switches.Get(currentSwitchValues);
				static_assert(sizeof(values) == sizeof(currentSV->values));
				memcpy(values, currentSV->values, sizeof(currentSV->values));

				bool frameToPush = false;

#define ENSURE_FRAME_PUSHED																	\
					do																		\
					{																		\
						if (frameToPush)													\
						{																	\
							currentSV = switches.Get(PushSwitchFrame(currentSV, values));	\
							frameToPush = false;											\
						}																	\
					} while(false)															\

#define DECODE_SWITCH_ITEM																	\
					vint32_t row = ins.param / 8 * sizeof(vuint32_t);						\
					vint32_t column = ins.param % 8 * sizeof(vuint32_t);					\
					vuint32_t& value = values[row]											\

#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::RunEdgeConditionChecking(vint32_t, EdgeDesc&)#"
#define ENSURE_NEW_FRAME CHECK_ERROR(frameToPush, ERROR_MESSAGE_PREFIX L"Switch instruction corrupted.")

				for (vint32_t insRef = 0; insRef < edgeDesc.insSwitch.count; insRef++)
				{
					auto&& ins = executable.switchInstructions[edgeDesc.insSwitch.start + insRef];
					switch (ins.type)
					{
					case SwitchInsType::SwitchPushFrame:
						// the new frame inherits from current values
						// no need to update "values" after changing "currentSV"
						ENSURE_FRAME_PUSHED;
						frameToPush = true;
						break;
					case SwitchInsType::SwitchWriteTrue:
						{
							ENSURE_NEW_FRAME;
							DECODE_SWITCH_ITEM;
							value |= (vuint32_t)1 << column;
						}
						break;
					case SwitchInsType::SwitchWriteFalse:
						{
							ENSURE_NEW_FRAME;
							DECODE_SWITCH_ITEM;
							value &= ~((vuint32_t)1 << column);
						}
						break;
					case SwitchInsType::SwitchPopFrame:
						if (frameToPush)
						{
							// if the new frame has not been submitted
							// just cancel it
							frameToPush = false;
						}
						else
						{
							currentSV = switches.Get(currentSV->previous);
						}
						memcpy(values, currentSV->values, sizeof(values));
						break;
					case SwitchInsType::ConditionRead:
						{
							ENSURE_FRAME_PUSHED;
							DECODE_SWITCH_ITEM;
							bool read = (value >> column) % 2 == 1;
							if (temporaryConditionStackSize < temporaryConditionStack.Count())
							{
								temporaryConditionStack[temporaryConditionStackSize] = read;
							}
							else
							{
								temporaryConditionStack.Add(read);
							}
							temporaryConditionStackSize++;
						}
						break;
					case SwitchInsType::ConditionNot:
						{
							CHECK_ERROR(temporaryConditionStackSize >= 1, ERROR_MESSAGE_PREFIX L"Switch stack corrupted.");
							bool& operand = temporaryConditionStack[temporaryConditionStackSize - 1];
							operand = !operand;
						}
						break;
					case SwitchInsType::ConditionAnd:
						{
							CHECK_ERROR(temporaryConditionStackSize >= 2, ERROR_MESSAGE_PREFIX L"Switch stack corrupted.");
							bool& left = temporaryConditionStack[temporaryConditionStackSize - 2];
							bool right = temporaryConditionStack[temporaryConditionStackSize - 1];
							temporaryConditionStackSize--;
							left &= right;
						}
						break;
					case SwitchInsType::ConditionOr:
						{
							CHECK_ERROR(temporaryConditionStackSize >= 2, ERROR_MESSAGE_PREFIX L"Switch stack corrupted.");
							bool& left = temporaryConditionStack[temporaryConditionStackSize - 2];
							bool right = temporaryConditionStack[temporaryConditionStackSize - 1];
							temporaryConditionStackSize--;
							left |= right;
						}
						break;
					case SwitchInsType::ConditionTest:
						{
							CHECK_ERROR(temporaryConditionStackSize == 1, ERROR_MESSAGE_PREFIX L"Switch stack corrupted.");
							bool operand = temporaryConditionStack[temporaryConditionStackSize - 1];
							temporaryConditionStackSize--;
							if (!operand)
							{
								return -1;
							}
						}
						break;
					}
				}
				ENSURE_FRAME_PUSHED;

				CHECK_ERROR(temporaryConditionStackSize == 0, ERROR_MESSAGE_PREFIX L"Switch stack corrupted.");
				return currentSV->allocatedIndex;

#undef ERROR_MESSAGE_PREFIX
#undef ENSURE_NEW_FRAME
#undef DECODE_SWITCH_ITEM
#undef ENSURE_FRAME_PUSHED
			}

/***********************************************************************
TraceManager::WalkAlongSingleEdge
***********************************************************************/

			WalkingTrace TraceManager::WalkAlongSingleEdge(
				vint32_t currentTokenIndex,
				vint32_t input,
				WalkingTrace trace,
				vint32_t byEdge,
				EdgeDesc& edgeDesc
			)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::WalkAlongSingleEdge(vint, vint, vint, Trace*, vint, EdgeDesc&)#"
				vint32_t state = edgeDesc.toState;
				vint32_t returnStack = -1;
				vint32_t attendingCompetitions = -1;
				vint32_t carriedCompetitions = -1;
				vint32_t executedReturnStack = -1;
				vint32_t switchValues = rootSwitchValues == -1 ? -1 : RunEdgeConditionChecking(trace.stateTrace->switchValues, edgeDesc);
				Trace* ambiguityTraceToMerge = nullptr;

				if (rootSwitchValues != -1 && switchValues == -1)
				{
					// stop if condition checking failed
					return { nullptr,nullptr };
				}

				// attend a competition hold by the current trace if the priority is set for this output transition
				AttendCompetitionIfNecessary(trace.stateTrace, currentTokenIndex, edgeDesc, attendingCompetitions, carriedCompetitions, returnStack);

				if (input == Executable::EndingInput)
				{
					// an EndingInput transition consume return record in the return stack
					// such return will be popped from the return stack and stored in Trace::executedReturnStack
					CHECK_ERROR(edgeDesc.returnIndices.count == 0, ERROR_MESSAGE_PREFIX L"Ending input edge is not allowed to push something into the return stack.");
					if (returnStack != -1)
					{
						executedReturnStack = returnStack;
						auto rs = GetReturnStack(returnStack);
						returnStack = rs->previous;
						state = executable.returns[rs->returnIndex].returnState;
					}

					// an EndingInput transition also settle a competition if
					//   1) there is a competition
					//   2) the returnStack of the trace holding the competition is the same to the current returnStack
					//   3) the target trace bets high priority
					// in this case, high priority traces wins the competition
					// but no traces are being removed for now, just mark the competition
					CheckAttendingCompetitionsOnEndingEdge(trace.stateTrace, edgeDesc, attendingCompetitions, trace.stateTrace->returnStack);
				}

				// create a new trace for this current move
				auto newTrace = AllocateTrace();
				AddTraceToCollection(newTrace, trace.currentTrace, &Trace::predecessors);
				newTrace->state = state;
				newTrace->returnStack = returnStack;
				newTrace->executedReturnStack = executedReturnStack;
				newTrace->switchValues = switchValues;
				newTrace->byEdge = byEdge;
				newTrace->byInput = input;
				newTrace->currentTokenIndex = currentTokenIndex;
				newTrace->competitionRouting.attendingCompetitions = attendingCompetitions;
				newTrace->competitionRouting.carriedCompetitions = carriedCompetitions;

				if (input == Executable::EndingInput)
				{
					// see if the target trace has the same state to any other surviving trace
					for (vint i = 0; i < concurrentCount; i++)
					{
						auto& candidate = backupTraces->operator[](i);
						if (AreTwoEndingInputTraceEqual(newTrace, candidate))
						{
							// create a merging 
							MergeTwoEndingInputTrace(newTrace, candidate);
							return { nullptr,nullptr };
						}
					}
				}

				// add to the current trace list only if it is not involved in ambiguity resolving
				AddTrace(newTrace);
				return { newTrace,newTrace };
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
TraceManager::WalkAlongEpsilonEdges
***********************************************************************/

			void TraceManager::WalkAlongLeftrecEdges(
				vint32_t currentTokenIndex,
				regex::RegexToken* lookAhead,
				WalkingTrace trace,
				EdgeArray& edgeArray
			)
			{
				// if there is no more token
				// then it is not possible for more left recursions
				if (!lookAhead) return;

				for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint32_t byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[byEdge];

					// see if the target state could consume that token
					vint32_t lookAheadTransitionIndex = executable.GetTransitionIndex(edgeDesc.toState, Executable::TokenBegin + (vint32_t)lookAhead->token);
					auto& lookAheadEdgeArray = executable.transitions[lookAheadTransitionIndex];
					if (!IsQualifiedTokenForEdgeArray(lookAhead, lookAheadEdgeArray)) continue;

					// proceed only if it can
					WalkAlongSingleEdge(currentTokenIndex, Executable::LeftrecInput, trace, byEdge, edgeDesc);

					// A LeftrecInput transition points to a non ending state in another clause
					// so there is no need to find other epsilon transitions after LeftrecInput
				}
			}

			void TraceManager::WalkAlongEpsilonEdges(
				vint32_t currentTokenIndex,
				regex::RegexToken* lookAhead,
				WalkingTrace trace
			)
			{
				// if we could walk along multiple EndingInput transition
				// but the last several transition will fail
				// then creating them is wasting the performance
				// so we count how many EndingInput transition we could walk along first

				vint32_t endingCount = -1;

				if (!lookAhead)
				{
					// if there is no more tokens
					// then we have to go all the way to the end anyway
					vint32_t currentState = trace.stateTrace->state;
					vint32_t currentReturnStack = trace.stateTrace->returnStack;

					while (currentState != -1)
					{
						vint32_t transitionIndex = executable.GetTransitionIndex(currentState, Executable::EndingInput);
						auto&& edgeArray = executable.transitions[transitionIndex];

						// at most one EndingInput transition could exist from any state
						CHECK_ERROR(edgeArray.count < 2, L"vl::glr::automaton::TraceManager::WalkAlongEpsilonEdges(vint32_t, vint32_t, Trace*)#Too many EndingInput transitions.");

						if (edgeArray.count == 0)
						{
							// if there is no more EndingInput to go
							// and the current state is not an ending state
							// then we just give up

							auto&& stateDesc = executable.states[currentState];
							if (stateDesc.endingState)
							{
								currentState = -1;
							}
							else
							{
								return;
							}
						}
						else if (currentReturnStack == -1)
						{
							vint32_t byEdge = edgeArray.start;
							auto& edgeDesc = executable.edges[byEdge];
							currentState = edgeDesc.toState;
						}
						else
						{
							auto rs = GetReturnStack(currentReturnStack);
							currentReturnStack = rs->previous;
							currentState = executable.returns[rs->returnIndex].returnState;
						}
					}
				}
				else
				{
					// otherwise we see how many EndingInput transition we need to walk along
					vint32_t currentCount = 0;
					vint32_t currentState = trace.stateTrace->state;
					vint32_t currentReturnStack = trace.stateTrace->returnStack;

					while (currentState != -1)
					{
						currentCount++;

						// try LeftrecInput + lookAhead
						{
							vint32_t transitionIndex = executable.GetTransitionIndex(currentState, Executable::LeftrecInput);
							auto&& edgeArray = executable.transitions[transitionIndex];
							for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
							{
								vint32_t byEdge = edgeArray.start + edgeRef;
								auto& edgeDesc = executable.edges[byEdge];
								vint32_t lookAheadTransitionIndex = executable.GetTransitionIndex(edgeDesc.toState, Executable::TokenBegin + (vint32_t)lookAhead->token);
								auto& lookAheadEdgeArray = executable.transitions[lookAheadTransitionIndex];

								// mark this EndingInput if any LeftrecInput + lookAhead transition exists
								if (IsQualifiedTokenForEdgeArray(lookAhead, lookAheadEdgeArray))
								{
									endingCount = currentCount;
									goto TRY_ENDING_INPUT;
								}
							}
						}

						// try lookAhead
						{
							vint32_t transitionIndex = executable.GetTransitionIndex(currentState, Executable::TokenBegin + (vint32_t)lookAhead->token);
							auto&& edgeArray = executable.transitions[transitionIndex];

							// mark this EndingInput if lookAhead transition exists
							if (IsQualifiedTokenForEdgeArray(lookAhead, edgeArray))
							{
								endingCount = currentCount;
							}
						}

						// try EndingInput
					TRY_ENDING_INPUT:
						{
							vint32_t transitionIndex = executable.GetTransitionIndex(currentState, Executable::EndingInput);
							auto&& edgeArray = executable.transitions[transitionIndex];

							// at most one EndingInput transition could exist from any state
							CHECK_ERROR(edgeArray.count < 2, L"vl::glr::automaton::TraceManager::WalkAlongEpsilonEdges(vint32_t, vint32_t, Trace*)#Too many EndingInput transitions.");

							if (edgeArray.count == 0 || currentReturnStack == -1)
							{
								// currentReturnStack == -1 means this is the last possible EndingInput
								// no need to test forward
								// because if the current EndingInput is doable
								// it would have already been marked
								currentState = -1;
							}
							else
							{
								auto rs = GetReturnStack(currentReturnStack);
								currentReturnStack = rs->previous;
								currentState = executable.returns[rs->returnIndex].returnState;
							}
						}
					}
				}

				for (vint32_t i = 0; trace && (i < endingCount || endingCount == -1); i++)
				{
					{
						// LeftrecInput transition is an epsilon transition
						vint32_t transitionIndex = executable.GetTransitionIndex(trace.stateTrace->state, Executable::LeftrecInput);
						auto&& edgeArray = executable.transitions[transitionIndex];
						WalkAlongLeftrecEdges(currentTokenIndex, lookAhead, trace, edgeArray);
					}

					// EndingInput transition is an epsilon transition
					vint32_t transitionIndex = executable.GetTransitionIndex(trace.stateTrace->state, Executable::EndingInput);
					auto&& edgeArray = executable.transitions[transitionIndex];

					// it has been ensured that edgeArray.count < 2
					if (edgeArray.count == 0)
					{
						trace = { nullptr,nullptr };
					}
					else
					{
						vint32_t byEdge = edgeArray.start;
						auto& edgeDesc = executable.edges[byEdge];
						trace = WalkAlongSingleEdge(currentTokenIndex, Executable::EndingInput, trace, byEdge, edgeDesc);

						// EndingInput could be followed by EndingInput or LeftrecInput
					}
				}
			}

/***********************************************************************
TraceManager::WalkAlongTokenEdges
***********************************************************************/

			void TraceManager::WalkAlongTokenEdges(
				vint32_t currentTokenIndex,
				vint32_t input,
				regex::RegexToken* token,
				regex::RegexToken* lookAhead,
				WalkingTrace trace,
				EdgeArray& edgeArray
			)
			{
				// find all transitions that has the expected input
				// there could be multiple transitions with the same input
				// but with different instructions and destinations
				for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint32_t byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					if (IsQualifiedTokenForCondition(token, edgeDesc.condition))
					{
						if (auto newTrace = WalkAlongSingleEdge(currentTokenIndex, input, trace, byEdge, edgeDesc))
						{
							// continue with as much EndingInput and LeftrecInput transitions as possible
							// TokenInput could be followed by EndingInput or LeftrecInput
							WalkAlongEpsilonEdges(currentTokenIndex, lookAhead, newTrace);
						}
					}
				}
			}
		}
	}
}