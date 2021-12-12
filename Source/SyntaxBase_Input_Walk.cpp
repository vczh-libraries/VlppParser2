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
				vint32_t returnStack = -1;
				vint32_t attendingCompetitions = -1;
				vint32_t carriedCompetitions = -1;
				vint32_t executedReturn = -1;
				Trace* ambiguityTraceToMerge = nullptr;

				// attend a competition hold by the current trace if the priority is set for this output transition
				AttendCompetitionIfNecessary(trace, edgeDesc, attendingCompetitions, carriedCompetitions, returnStack);

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
					CheckAttendingCompetitionsOnEndingEdge(trace, edgeDesc, attendingCompetitions, trace->returnStack);

					// if the target trace has exactly the same to another surviving trace
					// stop creating a Trace instance for the target trace
					// instead connect the correct trace to that surviving trace and form a ambiguity resolving structure
					for (vint i = 0; i < concurrentCount; i++)
					{
						auto candidate = backupTraces->Get(i);
						if (AreTwoEndingInputTraceEqual(state, returnStack, executedReturn, attendingCompetitions, candidate))
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
						executedReturn);

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

					newTrace->predecessors.first = trace->allocatedIndex;
					newTrace->predecessors.last = trace->allocatedIndex;
					newTrace->state = state;
					newTrace->returnStack = returnStack;
					newTrace->executedReturn = executedReturn;
					newTrace->byEdge = byEdge;
					newTrace->byInput = input;
					newTrace->currentTokenIndex = currentTokenIndex;
					newTrace->runtimeRouting.attendingCompetitions = attendingCompetitions;
					newTrace->runtimeRouting.carriedCompetitions = carriedCompetitions;

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