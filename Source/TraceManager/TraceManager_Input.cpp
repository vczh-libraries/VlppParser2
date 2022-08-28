#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;

/***********************************************************************
Initialize
***********************************************************************/

			void TraceManager::Initialize(vint32_t startState)
			{
				state = TraceManagerState::WaitingForInput;

				returnStacks.Clear();
				traces.Clear();
				competitions.Clear();
				attendingCompetitions.Clear();
				switches.Clear();

				traces1.Clear();
				traces2.Clear();
				concurrentTraces = &traces1;
				backupTraces = &traces2;

				activeCompetitions = -1;
				initialReturnStackCache = {};

				if (executable.switchDefaultValues.Count() == 0)
				{
					rootSwitchValues = -1;
				}
				else
				{
					rootSwitchValues = switches.Allocate();
					auto sv = switches.Get(rootSwitchValues);
					for (vint32_t i = 0; i < executable.switchDefaultValues.Count(); i++)
					{
						if (executable.switchDefaultValues[i])
						{
							vint32_t row = i / 8 * sizeof(vuint32_t);
							vint32_t column = i % 8 * sizeof(vuint32_t);
							vuint32_t& value = sv->values[row];
							value |= (vuint32_t)1 << column;
						}
					}
				}

				temporaryConditionStack.Clear();
				temporaryConditionStackSize = 0;

				traceExecs.Clear();
				insExecs.Resize(0);

				initialTrace = AllocateTrace();
				initialTrace->state = startState;
				initialTrace->switchValues = rootSwitchValues;
				concurrentCount = 1;
				concurrentTraces->Add(initialTrace);
			}

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
					auto currentTrace = concurrentTraces->Get(traceIndex);
					auto stateTrace = EnsureTraceWithValidStates(currentTrace);
					vint32_t transitionIndex = executable.GetTransitionIndex(stateTrace->state, input);
					auto&& edgeArray = executable.transitions[transitionIndex];
					WalkAlongTokenEdges(currentTokenIndex, input, token, lookAhead, { currentTrace, stateTrace }, edgeArray);
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

					vint32_t predecessorId = current->predecessors.last;
					while (predecessorId != -1)
					{
						auto predecessor = GetTrace(predecessorId);
						predecessorId = predecessor->predecessors.siblingPrev;

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
					auto actualTrace = EnsureTraceWithValidStates(trace);
					auto& stateDesc = executable.states[actualTrace->state];
					if (actualTrace->returnStack == -1 && stateDesc.endingState)
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