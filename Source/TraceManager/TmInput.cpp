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

				activeCompetitions = nullref;
				initialReturnStackCache = {};

				if (executable.switchDefaultValues.Count() == 0)
				{
					rootSwitchValues = nullref;
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
				MergeStack_MagicCounter = 0;

				traceExecs.Clear();
				insExecs.Resize(0);
				insExec_Objects.Clear();
				insExec_InsRefLinks.Clear();
				insExec_ObjRefLinks.Clear();
				insExec_ObjectStacks.Clear();
				insExec_CreateStacks.Clear();

				firstBranchTrace = nullref;
				firstMergeTrace = nullref;
				firstStep = nullref;
				traceAmbiguities.Clear();
				traceAmbiguityLinks.Clear();
				executionSteps.Clear();

				initialTrace = AllocateTrace();
				initialTrace->state = startState;
				initialTrace->switchValues = rootSwitchValues;
				concurrentCount = 1;
				concurrentTraces->Add(initialTrace);
			}

/***********************************************************************
GetInitialTrace
***********************************************************************/

			Trace* TraceManager::GetInitialTrace()
			{
				return initialTrace;
			}

/***********************************************************************
GetInitialTrace
***********************************************************************/

			ExecutionStep* TraceManager::GetInitialExecutionStep()
			{
				return firstStep == nullref ? nullptr : GetExecutionStep(firstStep);
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

			void TraceManager::FillSuccessorsAfterEndOfInput(bool& ambiguityInvolved)
			{
				ambiguityInvolved = false;
				List<Trace*> visiting;

				// create a merge trace for multiple surviving traces
				if (concurrentCount > 1)
				{
					auto newTrace = GetTrace(traces.Allocate());
					for (vint32_t traceIndex = 0; traceIndex < concurrentCount; traceIndex++)
					{
						auto trace = concurrentTraces->Get(traceIndex);
						auto first = trace;
						auto last = trace;

						if (trace->state == -1)
						{
							// a surviving trace could also be a merge trace
							// in this case we move predecessors to the new trace
							first = GetTrace(trace->predecessors.first);
							last = GetTrace(trace->predecessors.last);
						}

						if (newTrace->predecessors.first == nullref)
						{
							newTrace->predecessors.first = first;
							newTrace->predecessors.last = last;
						}
						else
						{
							GetTrace(newTrace->predecessors.last)->predecessors.siblingNext = first;
							first->predecessors.siblingPrev = newTrace->predecessors.last;
							newTrace->predecessors.last = last;
						}
					}
					BeginSwap();
					AddTrace(newTrace);
					EndSwap();
				}
				visiting.Add(concurrentTraces->Get(0));

				// fill successors based on predecessors
				bool initialTraceVisited = false;
				while (visiting.Count() > 0)
				{
					auto current = visiting[visiting.Count() - 1];
					visiting.RemoveAt(visiting.Count() - 1);

					// if (current->predecessorCount != 0)
					// it means this trace has been processed when comming from another sibling
					// but initialTrace->predecessorCount is always 0
					// so initialTraceVisited is introduced
					if (current == initialTrace)
					{
						if (initialTraceVisited) continue;
						initialTraceVisited = true;
					}
					else if (current->predecessorCount != 0)
					{
						continue;
					}

					// fill successors
					{
						auto predecessorId = current->predecessors.first;
						while (predecessorId != nullref)
						{
							auto predecessor = GetTrace(predecessorId);
							predecessorId = predecessor->predecessors.siblingNext;
							current->predecessorCount++;
							predecessor->successorCount++;
							AddTraceToCollection(predecessor, current, &Trace::successors);
						}
					}

					// add predecessors to the list to continue
					{
						auto predecessorId = current->predecessors.last;
						while (predecessorId != nullref)
						{
							auto predecessor = GetTrace(predecessorId);
							predecessorId = predecessor->predecessors.siblingPrev;
							visiting.Add(predecessor);
						}
					}

					// set ambiguityInvolved when a trace has multiple predecessors
					if (current->predecessorCount > 1)
					{
						ambiguityInvolved = true;
					}
				}
			}

/***********************************************************************
EndOfInput
***********************************************************************/

			bool TraceManager::EndOfInput(bool& ambiguityInvolved)
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
					if (actualTrace->returnStack == nullref && stateDesc.endingState)
					{
						AddTrace(trace);
					}
				}

				EndSwap();
				if (concurrentCount == 0) return false;

				FillSuccessorsAfterEndOfInput(ambiguityInvolved);
				if (!ambiguityInvolved)
				{
					state = TraceManagerState::ResolvedAmbiguity;
					auto step = GetExecutionStep(executionSteps.Allocate());
					firstStep = step;

					auto lastTrace = concurrentTraces->Get(0);
					TraceInsLists insList;
					ReadInstructionList(lastTrace, insList);

					step->et_i.startTrace = initialTrace->allocatedIndex;
					step->et_i.startIns = 0;
					step->et_i.endTrace = lastTrace->allocatedIndex;
					step->et_i.endIns = insList.c3 - 1;
				}
				return initialTrace;
			}
		}
	}
}