#include "SyntaxBase.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
TraceManager::Input
***********************************************************************/

			Trace* TraceManager::WalkAlongSingleEdge(
				vint previousTokenIndex,
				vint currentTokenIndex,
				vint input,
				Trace* trace,
				vint byEdge,
				EdgeDesc& edgeDesc
			)
			{
				vint state = edgeDesc.toState;
				vint returnStack = trace->returnStack;
				vint executedReturn = -1;

				// push competition stack if edge has priority
				// pop competition stack if executedReturn has priority
				// delete all failed backupTraces brought by EdgePriority (could delete itself)
				// then continue with ambiguity resolving

				if (input == Executable::EndingInput)
				{
					CHECK_ERROR(edgeDesc.returnIndices.count == 0, L"vl::glr::automaton::TraceManager::WalkAlongSingleEdge(vint, vint, vint, Trace*, vint, EdgeDesc&)#Ending input edge is not allowed to push the return stack.");
					if (returnStack != -1)
					{
						auto rs = GetReturnStack(returnStack);
						returnStack = rs->previous;
						executedReturn = rs->returnIndex;
						state = executable.returns[executedReturn].returnState;
					}
				}

				for (vint i = 0; i < concurrentCount; i++)
				{
					auto candidate = backupTraces->Get(i);
					if (state == candidate->state && executedReturn == candidate->executedReturn)
					{
						auto r1 = returnStack;
						auto r2 = candidate->returnStack;
						while (true)
						{
							if (r1 == r2) goto MERGABLE_TRACE_FOUND;
							auto rs1 = GetReturnStack(r1);
							auto rs2 = GetReturnStack(r2);
							if (rs1->returnIndex != rs2->returnIndex)
							{
								auto& rd1 = executable.returns[rs1->returnIndex];
								auto& rd2 = executable.returns[rs2->returnIndex];
								if (rd1.returnState != rd2.returnState) goto MERGABLE_TRACE_NOT_FOUND;
								if (rd1.insAfterInput.count != rd2.insAfterInput.count) goto MERGABLE_TRACE_NOT_FOUND;
								for (vint insRef = 0; insRef < rd1.insAfterInput.count; insRef++)
								{
									auto& ins1 = executable.instructions[rd1.insAfterInput.start + insRef];
									auto& ins2 = executable.instructions[rd2.insAfterInput.start + insRef];
									if (ins1 != ins2) goto MERGABLE_TRACE_NOT_FOUND;
								}
							}
							r1 = rs1->previous;
							r2 = rs2->previous;
						}
					MERGABLE_TRACE_FOUND:
						AddTraceToCollection(candidate, trace, &Trace::predecessors);
						return nullptr;
					}
				MERGABLE_TRACE_NOT_FOUND:;
				}

				auto newTrace = AllocateTrace();
				AddTrace(newTrace);

				newTrace->predecessors.first = trace->allocatedIndex;
				newTrace->predecessors.last = trace->allocatedIndex;
				newTrace->state = state;
				newTrace->returnStack = returnStack;
				newTrace->executedReturn = executedReturn;
				newTrace->byEdge = byEdge;
				newTrace->byInput = input;
				newTrace->previousTokenIndex = previousTokenIndex;
				newTrace->currentTokenIndex = currentTokenIndex;

				for (vint returnRef = 0; returnRef < edgeDesc.returnIndices.count; returnRef++)
				{
					vint returnIndex = executable.returnIndices[edgeDesc.returnIndices.start + returnRef];
					auto returnStack = AllocateReturnStack();
					returnStack->previous = newTrace->returnStack;
					returnStack->returnIndex = returnIndex;
					newTrace->returnStack = returnStack->allocatedIndex;
				}

				return newTrace;
			}

			void TraceManager::WalkAlongTokenEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				vint input,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					if (auto newTrace = WalkAlongSingleEdge(previousTokenIndex, currentTokenIndex, input, trace, byEdge, edgeDesc))
					{
						WalkAlongEpsilonEdges(previousTokenIndex, currentTokenIndex, newTrace);
					}
				}
			}

			void TraceManager::WalkAlongEpsilonEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				Trace* trace
			)
			{
				{
					vint transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + Executable::LeftrecInput;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongLeftrecEdges(previousTokenIndex, currentTokenIndex, trace, edgeArray);
				}
				{
					vint transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + Executable::EndingInput;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongEndingEdges(previousTokenIndex, currentTokenIndex, trace, edgeArray);
				}
			}

			void TraceManager::WalkAlongLeftrecEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					WalkAlongSingleEdge(previousTokenIndex, currentTokenIndex, Executable::LeftrecInput, trace, byEdge, edgeDesc);
				}
			}

			void TraceManager::WalkAlongEndingEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					if (auto newTrace = WalkAlongSingleEdge(previousTokenIndex, currentTokenIndex, Executable::EndingInput, trace, byEdge, edgeDesc))
					{
						WalkAlongEpsilonEdges(previousTokenIndex, currentTokenIndex, newTrace);
					}
				}
			}

			void TraceManager::Input(vint currentTokenIndex, vint token)
			{
				CHECK_ERROR(state == TraceManagerState::WaitingForInput, L"vl::glr::automaton::TraceManager::Input(vint, vint)#Wrong timing to call this function.");
				vint traceCount = concurrentCount;
				vint previousTokenIndex = currentTokenIndex - 1;
				vint input = Executable::TokenBegin + token;

				BeginSwap();
				for (vint traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					vint transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + input;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongTokenEdges(previousTokenIndex, currentTokenIndex, input, trace, edgeArray);
				}
				EndSwap();

				for (vint traceIndex = concurrentCount; traceIndex < concurrentTraces->Count(); traceIndex++)
				{
					concurrentTraces->Set(traceIndex, nullptr);
				}
			}

			void TraceManager::EndOfInput()
			{
				CHECK_ERROR(state == TraceManagerState::WaitingForInput, L"vl::glr::automaton::TraceManager::EndOfInput()#Wrong timing to call this function.");
				state = TraceManagerState::Finished;

				vint traceCount = concurrentCount;
				BeginSwap();
				for (vint traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					auto& stateDesc = executable.states[trace->state];
					if (trace->returnStack == -1 && stateDesc.endingState)
					{
						AddTrace(trace);
					}
				}
				EndSwap();
			}
		}
	}
}