#include "SyntaxBase.h"

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
				vint32_t returnStack = trace->returnStack;
				vint32_t executedReturn = -1;
				Trace* ambiguityTraceToMerge = nullptr;

				// attend a competition hold by the current trace if the priority is set for this output transition
				vint32_t acId = AttendCompetitionIfNecessary(trace, edgeDesc);

				if (input == Executable::EndingInput)
				{
					// an EndingInput transition consume return record in the return stack
					// such return will be popped from the return stack and stored in Trace::executedReturn
					CHECK_ERROR(edgeDesc.returnIndices.count == 0, ERROR_MESSAGE_PREFIX L"Ending input edge is not allowed to push something into the return stack.");
					if (returnStack != -1)
					{
						auto rs = GetReturnStack(returnStack);
						returnStack = rs->previous;
						executedReturn = rs->returnIndex;
						state = executable.returns[executedReturn].returnState;
					}

					// an EndingInput transition also settle a competition if
					//   1) there is a competition
					//   2) the returnStack of the trace holding the competition is the same to the current returnStack
					//   3) the target trace bets high priority
					// in this case, high priority traces wins the competition
					// but no traces are being removed for now, just mark the competition
					CheckAttendingCompetitionsOnEndingEdge(trace, edgeDesc, acId, trace->returnStack);

					// if the target trace has exactly the same to another surviving trace
					// stop creating a Trace instance for the target trace
					// instead connect the correct trace to that surviving trace and form a ambiguity resolving structure
					for (vint i = 0; i < concurrentCount; i++)
					{
						auto candidate = backupTraces->Get(i);
						if (AreTwoTraceEqual(state, returnStack, executedReturn, acId, candidate))
						{
							ambiguityTraceToMerge = candidate;
							break;
						}
					}
				}

				if (ambiguityTraceToMerge)
				{
					// if ambiguity resolving happens
					// find the instruction postfix
					// the instruction postfix starts from EndObject of a trace
					// and both instruction postfix should equal
					auto& oldEdge = executable.edges[ambiguityTraceToMerge->byEdge];
					vint32_t postfix = GetInstructionPostfix(oldEdge, edgeDesc);

					if (ambiguityTraceToMerge->ambiguityInsPostfix == -1)
					{
						if (oldEdge.insBeforeInput.count == postfix)
						{
							// if EndObject is the first instruction
							// no need to insert another trace
							ambiguityTraceToMerge->ambiguityInsPostfix = postfix;
						}
						else
						{
							// if EndObject is not the first instruction
							// insert another trace before ambiguityTraceMerge
							// and ambiguityTraceMerge should not have had multiple predecessors at this moment
							CHECK_ERROR(ambiguityTraceToMerge->predecessors.first == ambiguityTraceToMerge->predecessors.last, ERROR_MESSAGE_PREFIX L"An ambiguity resolving traces should have been cut.");

							auto formerTrace = AllocateTrace();
							{
								vint32_t formerId = formerTrace->allocatedIndex;
								*formerTrace = *ambiguityTraceToMerge;
								formerTrace->allocatedIndex = formerId;
							}

							// executedReturn is from the EndObject instruction
							// which is available in the instruction postfix
							// so formerTrace->executedReturn should be -1 and keep the previous return stack
							formerTrace->executedReturn = -1;
							if (ambiguityTraceToMerge->predecessors.first != -1)
							{
								auto predecessor = GetTrace(ambiguityTraceToMerge->predecessors.first);
								formerTrace->returnStack = predecessor->returnStack;
							}

							// ambiguity is filled by PrepareTraceRoute, skipped
							// runtimeRouting.holdingCompetition always belong to the second trace
							// runtimeRouting.attendingCompetitions is inherited
							formerTrace->runtimeRouting = {};
							formerTrace->runtimeRouting.attendingCompetitions = ambiguityTraceToMerge->runtimeRouting.attendingCompetitions;

							// both traces need to have the same ambiguityInsPostfix
							formerTrace->ambiguityInsPostfix = postfix;
							ambiguityTraceToMerge->ambiguityInsPostfix = postfix;

							// connect two traces
							// formerTrace has already copied predecessors, skipped
							// successors of both traces are filled byPrepareTraceRoute, skipped
							// insert formerTrace before ambiguityTraceToMerge because
							// we don't successors of ambiguityTraceToMerge, cannot redirect their predecessors
							ambiguityTraceToMerge->predecessors.first = formerTrace->allocatedIndex;
							ambiguityTraceToMerge->predecessors.last = formerTrace->allocatedIndex;
						}
					}

					if (edgeDesc.insBeforeInput.count == postfix)
					{
						// if EndObject is the first instruction of the new trace
						// then no need to create the new trace
						AddTraceToCollection(ambiguityTraceToMerge, trace, &Trace::predecessors);
					}
					else
					{
						// otherwise, create a new trace with the instruction prefix
						auto newTrace = AllocateTrace();
						newTrace->predecessors.first = trace->allocatedIndex;
						newTrace->predecessors.last = trace->allocatedIndex;
						newTrace->state = state;
						newTrace->returnStack = trace->returnStack;
						newTrace->byEdge = byEdge;
						newTrace->byInput = input;
						newTrace->currentTokenIndex = currentTokenIndex;

						// executedReturn == ambiguityTraceToMerge->executedReturn is ensured
						// so no need to assign executedReturn to newTrace
						// acid == ambiguityTraceToMerge->runtimeRouting.attendingCompetitions is ensure
						// and ambiguityTraceToMerge is supposed to inherit this value
						newTrace->runtimeRouting.attendingCompetitions = acId;

						newTrace->ambiguityInsPostfix = postfix;

						AddTraceToCollection(ambiguityTraceToMerge, newTrace, &Trace::predecessors);
					}

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

					newTrace->predecessors.first = trace->allocatedIndex;
					newTrace->predecessors.last = trace->allocatedIndex;
					newTrace->state = state;
					newTrace->returnStack = returnStack;
					newTrace->executedReturn = executedReturn;
					newTrace->byEdge = byEdge;
					newTrace->byInput = input;
					newTrace->currentTokenIndex = currentTokenIndex;
					newTrace->runtimeRouting.attendingCompetitions = acId;

					// push returns to the return stack if the transition requires
					for (vint returnRef = 0; returnRef < edgeDesc.returnIndices.count; returnRef++)
					{
						auto returnIndex = executable.returnIndices[edgeDesc.returnIndices.start + returnRef];
						auto returnStack = AllocateReturnStack();
						returnStack->previous = newTrace->returnStack;
						returnStack->returnIndex = returnIndex;
						newTrace->returnStack = returnStack->allocatedIndex;
					}

					return newTrace;
				}
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
TraceManager::WalkAlongEpsilonEdges
***********************************************************************/

			void TraceManager::WalkAlongLeftrecEdges(
				vint32_t currentTokenIndex,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint32_t byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					WalkAlongSingleEdge(currentTokenIndex, Executable::LeftrecInput, trace, byEdge, edgeDesc);

					// A LeftrecInput transition points to a non ending state in another clause
					// so there is no need to find other epsilon transitions after LeftrecInput
				}
			}

			void TraceManager::WalkAlongEndingEdges(
				vint32_t currentTokenIndex,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint32_t byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					if (auto newTrace = WalkAlongSingleEdge(currentTokenIndex, Executable::EndingInput, trace, byEdge, edgeDesc))
					{
						// EndingInput could be followed by EndingInput or LeftrecInput
						WalkAlongEpsilonEdges(currentTokenIndex, newTrace);
					}
				}
			}

			void TraceManager::WalkAlongEpsilonEdges(
				vint32_t currentTokenIndex,
				Trace* trace
			)
			{
				{
					// LeftrecInput transition is an epsilon transition
					vint32_t transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + Executable::LeftrecInput;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongLeftrecEdges(currentTokenIndex, trace, edgeArray);
				}
				{
					// EndingInput transition is an epsilon transition
					vint32_t transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + Executable::EndingInput;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongEndingEdges(currentTokenIndex, trace, edgeArray);
				}
			}

/***********************************************************************
TraceManager::WalkAlongTokenEdges
***********************************************************************/

			void TraceManager::WalkAlongTokenEdges(
				vint32_t currentTokenIndex,
				vint32_t input,
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
						WalkAlongEpsilonEdges(currentTokenIndex, newTrace);
					}
				}
			}
		}
	}
}