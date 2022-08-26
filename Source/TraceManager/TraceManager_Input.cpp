#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;

/***********************************************************************
Input
***********************************************************************/

			bool TraceManager::Input(vint32_t currentTokenIndex, regex::RegexToken* token, regex::RegexToken* lookAhead)
			{
				CHECK_ERROR(state == TraceManagerState::WaitingForInput, L"vl::glr::automaton::TraceManager::Input(vint, vint)#Wrong timing to call this function.");
				vint32_t traceCount = concurrentCount;
				vint32_t input = Executable::TokenBegin + (vint32_t)token->token;

				BeginSwap();

				// for each surviving trace
				// step one TokenInput transition
				// followed by multiple and EndingInput, LeftrecInput and their combination
				// one surviving trace could create multiple surviving trace
				for (vint32_t traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					vint32_t transitionIndex = executable.GetTransitionIndex(trace->state, input);
					auto&& edgeArray = executable.transitions[transitionIndex];
					WalkAlongTokenEdges(currentTokenIndex, input, token, lookAhead, trace, edgeArray);
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

				return concurrentCount > 0;
			}

/***********************************************************************
FillSuccessorsAfterEndOfInput
***********************************************************************/

			void TraceManager::FillSuccessorsAfterEndOfInput()
			{
				List<Trace*> traces;
				for (vint32_t traceIndex = 0; traceIndex < concurrentCount; traceIndex++)
				{
					traces.Add(concurrentTraces->Get(traceIndex));
				}

				while (traces.Count() > 0)
				{
					auto current = traces[traces.Count() - 1];
					traces.RemoveAt(traces.Count() - 1);

					vint32_t predecessorId = current->predecessors.first;
					while (predecessorId != -1)
					{
						auto predecessor = GetTrace(predecessorId);
						predecessorId = predecessor->predecessors.siblingNext;

						if (predecessor->successors.first == current->allocatedIndex) continue;
						if (predecessor->successors.last == current->allocatedIndex) continue;
						if (current->successors.siblingPrev != -1) continue;
						if (current->successors.siblingNext != -1) continue;

						AddTraceToCollection(predecessor, current, &Trace::successors);
						traces.Add(predecessor);
					}
				}
			}

/***********************************************************************
EndOfInput
***********************************************************************/

			Trace* TraceManager::EndOfInput()
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
				if (concurrentCount == 0) return nullptr;

				FillSuccessorsAfterEndOfInput();
				return initialTrace;
			}
		}
	}
}