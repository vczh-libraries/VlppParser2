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
TraceManager::TestLeftrecEdgeQualification
***********************************************************************/

			void TraceManager::TestLeftrecEdgeQualification(EdgeDesc& edgeDesc, regex::RegexToken* lookAhead, bool& acceptLookAhead, bool& acceptEndingInput)
			{
				if (lookAhead)
				{
					vint32_t lookAheadTransitionIndex = executable.GetTransitionIndex(edgeDesc.toState, Executable::TokenBegin + (vint32_t)lookAhead->token);
					auto& lookAheadEdgeArray = executable.transitions[lookAheadTransitionIndex];

					// mark this EndingInput if any LeftrecInput + lookAhead transition exists
					if (IsQualifiedTokenForEdgeArray(lookAhead, lookAheadEdgeArray))
					{
						acceptLookAhead = true;
					}
				}

				{
					vint32_t endingInputTransitionIndex = executable.GetTransitionIndex(edgeDesc.toState, Executable::EndingInput);
					auto& endingInputEdgeArray = executable.transitions[endingInputTransitionIndex];
					if (endingInputEdgeArray.count > 0)
					{
						acceptEndingInput = true;
					}
				}
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
				Ref<ReturnStack> returnStack;
				Ref<AttendingCompetitions> attendingCompetitions;
				Ref<AttendingCompetitions> carriedCompetitions;
				Ref<ReturnStack> executedReturnStack;
				Trace* ambiguityTraceToMerge = nullptr;

				// attend a competition hold by the current trace if the priority is set for this output transition
				AttendCompetitionIfNecessary(trace.stateTrace, currentTokenIndex, edgeDesc, attendingCompetitions, carriedCompetitions, returnStack);

				if (input == Executable::EndingInput)
				{
					// an EndingInput transition consume return record in the return stack
					// such return will be popped from the return stack and stored in Trace::executedReturnStack
					CHECK_ERROR(edgeDesc.returnIndices.count == 0, ERROR_MESSAGE_PREFIX L"Ending input edge is not allowed to push something into the return stack.");
					if (returnStack != nullref)
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
						if (candidate->byInput == Executable::EndingInput || candidate->state == -1)
						{
							if (AreTwoEndingInputTraceEqual(newTrace, candidate))
							{
								// create a merging 
								MergeTwoEndingInputTrace(newTrace, candidate);
								return { nullptr,nullptr };
							}
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
				for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint32_t byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[byEdge];

					// see if the target state could consume that token
					bool acceptLookAhead = false;
					bool acceptEndingInput = false;
					TestLeftrecEdgeQualification(edgeDesc, lookAhead, acceptLookAhead, acceptEndingInput);

					if (acceptLookAhead || acceptEndingInput)
					{
						// proceed only if it can
						auto nextTrace = WalkAlongSingleEdge(currentTokenIndex, Executable::LeftrecInput, trace, byEdge, edgeDesc);

						if (acceptEndingInput && nextTrace)
						{
							// A LeftrecInput will be generated because of
							//   A real left-recursive rule
							//   Merging prefix inside a rule
							//   Merging prefix crossed-reference
							// The last two cases could connect LeftrecInput transitions to an ending state
							WalkAlongEpsilonEdges(currentTokenIndex, lookAhead, nextTrace);
						}
					}
				}
			}

			void TraceManager::WalkAlongEpsilonEdges(
				vint32_t currentTokenIndex,
				regex::RegexToken* lookAhead,
				WalkingTrace trace
			)
			{
				if (!lookAhead)
				{
					int a = 0;
				}
				// if we could walk along multiple EndingInput transition
				// but the last several transition will fail
				// then creating them is wasting the performance
				// so we count how many EndingInput transition we could walk along first

				vint32_t endingCount = -1;
				{
					// if there is no more tokens
					// then we have to go all the way to the end anyway
					// otherwise we see how many EndingInput transition we need to walk along
					vint32_t currentCount = 0;
					vint32_t currentState = trace.stateTrace->state;
					auto currentReturnStack = trace.stateTrace->returnStack;

#define MARK_AT_LEAST_EXECUTE_TO_THIS_LEVEL endingCount = currentCount

					while (currentState != -1)
					{
						currentCount++;

						// try LeftrecInput + (lookAhead or EndingInput)
						{
							vint32_t transitionIndex = executable.GetTransitionIndex(currentState, Executable::LeftrecInput);
							auto&& edgeArray = executable.transitions[transitionIndex];
							for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
							{
								vint32_t byEdge = edgeArray.start + edgeRef;
								auto& edgeDesc = executable.edges[byEdge];

								bool acceptLookAhead = false;
								bool acceptEndingInput = false;
								TestLeftrecEdgeQualification(edgeDesc, lookAhead, acceptLookAhead, acceptEndingInput);

								// mark this EndingInput if any LeftrecInput + (lookAhead or EndingInput) transition exists
								if (acceptLookAhead || acceptEndingInput)
								{
									MARK_AT_LEAST_EXECUTE_TO_THIS_LEVEL;
									goto TRY_ENDING_INPUT;
								}
							}
						}

						// try lookAhead
						if (lookAhead)
						{
							vint32_t transitionIndex = executable.GetTransitionIndex(currentState, Executable::TokenBegin + (vint32_t)lookAhead->token);
							auto&& edgeArray = executable.transitions[transitionIndex];

							// mark this EndingInput if lookAhead transition exists
							if (IsQualifiedTokenForEdgeArray(lookAhead, edgeArray))
							{
								MARK_AT_LEAST_EXECUTE_TO_THIS_LEVEL;
							}
						}

						// try EndingInput
					TRY_ENDING_INPUT:
						{
							vint32_t transitionIndex = executable.GetTransitionIndex(currentState, Executable::EndingInput);
							auto&& edgeArray = executable.transitions[transitionIndex];

							if (edgeArray.count > 1)
							{
								// if there are multiple EndingInput transitions
								// assume they would all succeed, and do recursive calls later
								MARK_AT_LEAST_EXECUTE_TO_THIS_LEVEL;
								break;
							}
							else if (edgeArray.count == 1 && currentReturnStack != nullref)
							{
								auto rs = GetReturnStack(currentReturnStack);
								currentReturnStack = rs->previous;
								currentState = executable.returns[rs->returnIndex].returnState;
							}
							else if (lookAhead)
							{
								// lookAhead && (edgeArray.count == 0 || currentReturnStack == nullref)
								// if edgeArray.count == 0
								//   no further EndingInput transition could be walked
								// if currentReturnStack == nullref
								//   it means this is the last possible EndingInput
								//   no need to test forward
								//   because if the current EndingInput is doable
								//   it would have already been marked
								break;
							}
							else if (edgeArray.count == 0)
							{
								// !lookAhead && edgeArray.count == 0
								// if there is no more EndingInput to go
								// and the current state is not an ending state
								// then we just give up
								// it is possible that a LeftrecInput transition is available

								auto&& stateDesc = executable.states[currentState];
								if (stateDesc.endingState)
								{
									MARK_AT_LEAST_EXECUTE_TO_THIS_LEVEL;
								}
								break;
							}
							else
							{
								// !lookAhead && edgeArray.count == 1 && currentReturnStack == nullref
								vint32_t byEdge = edgeArray.start;
								auto& edgeDesc = executable.edges[byEdge];
								currentState = edgeDesc.toState;
							}
						}
					}
				}

#undef MARK_AT_LEAST_EXECUTE_TO_THIS_LEVEL

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
						for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
						{
							vint32_t byEdge = edgeArray.start + edgeRef;
							auto& edgeDesc = executable.edges[byEdge];
							auto nextTrace = WalkAlongSingleEdge(currentTokenIndex, Executable::EndingInput, trace, byEdge, edgeDesc);

							if (edgeArray.count > 1)
							{
								// if the current trace has multiple EndingInput
								// we don't know if the current trace will survive or not
								// a following recursive call is necessary
								if (nextTrace)
								{
									WalkAlongEpsilonEdges(currentTokenIndex, lookAhead, nextTrace);
								}
							}
							else
							{
								trace = nextTrace;
							}
						}

						if (edgeArray.count > 1)
						{
							// when this for-loop ends, the outer for-loop also ends
							trace = { nullptr,nullptr };
						}

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
					auto& edgeDesc = executable.edges[byEdge];
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