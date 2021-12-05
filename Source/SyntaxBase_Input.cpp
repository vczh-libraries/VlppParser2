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

			bool TraceManager::AreReturnDescEqual(vint ri1, vint ri2)
			{
				// TODO: create a cache to compare two returnIndex directly
				// instead of repeatly scanning the content here
				if (ri1 == ri2) return true;
				auto& rd1 = executable.returns[ri1];
				auto& rd2 = executable.returns[ri2];
				if (rd1.returnState != rd2.returnState) return false;
				if (rd1.insAfterInput.count != rd2.insAfterInput.count) return false;
				for (vint insRef = 0; insRef < rd1.insAfterInput.count; insRef++)
				{
					auto& ins1 = executable.instructions[rd1.insAfterInput.start + insRef];
					auto& ins2 = executable.instructions[rd2.insAfterInput.start + insRef];
					if (ins1 != ins2) return false;
				}
				return true;
			}

			bool TraceManager::AreReturnStackEqual(vint r1, vint r2)
			{
				while (true)
				{
					if (r1 == r2) return true;
					if (r1 == -1 || r2 == -1) return false;
					auto rs1 = GetReturnStack(r1);
					auto rs2 = GetReturnStack(r2);
					if (!AreReturnDescEqual(rs1->returnIndex, rs2->returnIndex))
					{
						return false;
					}
					r1 = rs1->previous;
					r2 = rs2->previous;
				}
			}

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

				// from the same competition trace, if walked alone transitions with priority
				// predecessors will take a mark from the competition trace
				// when the high priority ending transition from the competition trace is picked up
				//     > ending state and the state of the competition trace is in the same clause
				//     > and return stack before executing the ending transition is the same to the competition trace
				// all other traces talking the connected low priority mark fails
				// high priority marks from this competition trace will be removed
				// after a step of input
				// if all high priority or all low priority marks from the same competition trace are gone
				// then all marks from this competition trace will also be removed
				//
				// a competition trace could maintain a TraceCollection
				// when a new step of input begins, the competition trace clear its collection, but the record in elements don't change
				// if a new trace is created, and the original trace has a non-empty record
				// the record is inherited, and the new trace adds itself to the competition trace's collection
				// if a competition trace is closed, it is flagged, and don't accept new elements
				// a linked list of living competition traces are maintained

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

				if (input == Executable::EndingInput)
				{
					for (vint i = 0; i < concurrentCount; i++)
					{
						auto candidate = backupTraces->Get(i);
						if (state == candidate->state && executedReturn == candidate->executedReturn)
						{
							auto r1 = returnStack;
							auto r2 = candidate->returnStack;
							if (AreReturnStackEqual(r1, r2))
							{
								AddTraceToCollection(candidate, trace, &Trace::predecessors);
								return nullptr;
							}
						}
					MERGABLE_TRACE_NOT_FOUND:;
					}
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