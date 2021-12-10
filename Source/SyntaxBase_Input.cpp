#include "SyntaxBase.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{

/***********************************************************************
Input
***********************************************************************/

			void TraceManager::Input(vint32_t currentTokenIndex, vint32_t token)
			{
				CHECK_ERROR(state == TraceManagerState::WaitingForInput, L"vl::glr::automaton::TraceManager::Input(vint, vint)#Wrong timing to call this function.");
				vint32_t traceCount = concurrentCount;
				vint32_t input = Executable::TokenBegin + token;

				BeginSwap();

				// for each surviving trace
				// step one TokenInput transition
				// followed by multiple and EndingInput, LeftrecInput and their combination
				// one surviving trace could create multiple surviving trace
				for (vint32_t traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					vint32_t transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + input;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongTokenEdges(currentTokenIndex, input, trace, edgeArray);
				}

				// if competitions happen between new surviving traces
				// remove traces that known to have lost the competition
				CheckBackupTracesBeforeSwapping(currentTokenIndex);

				EndSwap();

				// if there are unused spaces in concurrentTraces
				// set them to nullptr to clear out traces from the last round
				for (vint32_t traceIndex = concurrentCount; traceIndex < concurrentTraces->Count(); traceIndex++)
				{
					concurrentTraces->Set(traceIndex, nullptr);
				}
			}

/***********************************************************************
EndOfInput
***********************************************************************/

			void TraceManager::EndOfInput()
			{
				CHECK_ERROR(state == TraceManagerState::WaitingForInput, L"vl::glr::automaton::TraceManager::EndOfInput()#Wrong timing to call this function.");
				state = TraceManagerState::Finished;

				vint32_t traceCount = concurrentCount;
				BeginSwap();

				// check all surviving traces and remove all that
				//   1) does not stay in an ending state
				//   2) return stack is not empty
				// the remaining are all traces that successfully walked to the ending state of the root rule
				for (vint32_t traceIndex = 0; traceIndex < traceCount; traceIndex++)
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