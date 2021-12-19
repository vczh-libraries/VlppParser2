#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{

/***********************************************************************
TraceManager::WalkAlongSingleEdge
***********************************************************************/

			Trace* TraceManager::WalkAlongSingleEdge(
				vint32_t currentTokenIndex,
				vint32_t input,
				Trace* trace,
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
				Trace* ambiguityTraceToMerge = nullptr;

				// attend a competition hold by the current trace if the priority is set for this output transition
				AttendCompetitionIfNecessary(trace, currentTokenIndex, edgeDesc, attendingCompetitions, carriedCompetitions, returnStack);

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
					CheckAttendingCompetitionsOnEndingEdge(trace, edgeDesc, attendingCompetitions, trace->returnStack);

					// if the target trace has exactly the same to another surviving trace
					// stop creating a Trace instance for the target trace
					// instead connect the correct trace to that surviving trace and form a ambiguity resolving structure
					for (vint i = 0; i < concurrentCount; i++)
					{
						auto candidate = backupTraces->Get(i);
						if (AreTwoEndingInputTraceEqual(state, returnStack, executedReturnStack, attendingCompetitions, candidate))
						{
							ambiguityTraceToMerge = candidate;
							break;
						}
					}
				}

				if (ambiguityTraceToMerge)
				{
					MergeTwoEndingInputTrace(
						trace,
						ambiguityTraceToMerge,
						currentTokenIndex,
						input,
						byEdge,
						edgeDesc,
						state,
						returnStack,
						attendingCompetitions,
						carriedCompetitions,
						executedReturnStack);

					// return nullptr so that there is no WalkAlongEpsilonEdges following WalkAlongSingleEdge
					return nullptr;
				}
				else
				{
					// if ambiguity resolving doesn't happen
					// create an instance of the target trace
					// and connect the current trace to this target trace
					auto newTrace = AllocateTrace();
					AddTrace(newTrace);
					AddTraceToCollection(newTrace, trace, &Trace::predecessors);
					newTrace->state = state;
					newTrace->returnStack = returnStack;
					newTrace->executedReturnStack = executedReturnStack;
					newTrace->byEdge = byEdge;
					newTrace->byInput = input;
					newTrace->currentTokenIndex = currentTokenIndex;
					newTrace->competitionRouting.attendingCompetitions = attendingCompetitions;
					newTrace->competitionRouting.carriedCompetitions = carriedCompetitions;

					return newTrace;
				}
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
TraceManager::WalkAlongEpsilonEdges
***********************************************************************/

			void TraceManager::WalkAlongLeftrecEdges(
				vint32_t currentTokenIndex,
				vint32_t lookAhead,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				// if there is no more token
				// then it is not possible for more left recursions
				if (lookAhead == -1) return;

				for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint32_t byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[byEdge];

					// see if the target state could consume that token
					vint32_t lookAheadTransitionIndex = executable.GetTransitionIndex(edgeDesc.toState, Executable::TokenBegin + lookAhead);
					auto& lookAheadEdgeArray = executable.transitions[lookAheadTransitionIndex];
					if (lookAheadEdgeArray.count == 0) continue;

					// proceed only if it can
					WalkAlongSingleEdge(currentTokenIndex, Executable::LeftrecInput, trace, byEdge, edgeDesc);

					// A LeftrecInput transition points to a non ending state in another clause
					// so there is no need to find other epsilon transitions after LeftrecInput
				}
			}

			void TraceManager::WalkAlongEpsilonEdges(
				vint32_t currentTokenIndex,
				vint32_t lookAhead,
				Trace* trace
			)
			{
				// if we could walk along multiple EndingInput transition
				// but the last several transition will fail
				// then creating them is wasting the performance
				// so we count how many EndingInput transition we could walk along first

				vint32_t endingCount = -1;

				if (lookAhead == -1)
				{
					// if there is no more tokens
					// then we have to go all the way to the end anyway
					vint32_t currentState = trace->state;
					vint32_t currentReturnStack = trace->returnStack;

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
					vint32_t currentState = trace->state;
					vint32_t currentReturnStack = trace->returnStack;

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
								vint32_t lookAheadTransitionIndex = executable.GetTransitionIndex(edgeDesc.toState, Executable::TokenBegin + lookAhead);
								auto& lookAheadEdgeArray = executable.transitions[lookAheadTransitionIndex];

								// mark this EndingInput if any LeftrecInput + lookAhead transition exists
								if (lookAheadEdgeArray.count > 0)
								{
									endingCount = currentCount;
									goto TRY_ENDING_INPUT;
								}
							}
						}

						// try lookAhead
						{
							vint32_t transitionIndex = executable.GetTransitionIndex(currentState, Executable::TokenBegin + lookAhead);
							auto&& edgeArray = executable.transitions[transitionIndex];

							// mark this EndingInput if lookAhead transition exists
							if (edgeArray.count > 0)
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
						vint32_t transitionIndex = executable.GetTransitionIndex(trace->state, Executable::LeftrecInput);
						auto&& edgeArray = executable.transitions[transitionIndex];
						WalkAlongLeftrecEdges(currentTokenIndex, lookAhead, trace, edgeArray);
					}

					// EndingInput transition is an epsilon transition
					vint32_t transitionIndex = executable.GetTransitionIndex(trace->state, Executable::EndingInput);
					auto&& edgeArray = executable.transitions[transitionIndex];

					// it has been ensured that edgeArray.count < 2
					if (edgeArray.count == 0)
					{
						trace = nullptr;
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
				vint32_t lookAhead,
				Trace* trace,
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