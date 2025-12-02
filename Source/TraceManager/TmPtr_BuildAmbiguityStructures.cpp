#include "TraceManager.h"
#include "TraceManager_Common.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
#define NEW_MERGE_STACK_MAGIC_COUNTER (void)(MergeStack_MagicCounter++)

/***********************************************************************
BuildAmbiguityStructures
***********************************************************************/

			Trace* TraceManager::StepForward(Trace* trace)
			{
				auto traceExec = GetTraceExec(trace->traceExecRef);

				// for ordinary trace, go to its forwardTrace
				if (traceExec->branchData.forwardTrace != trace)
				{
					return GetTrace(traceExec->branchData.forwardTrace);
				}

				// for initialTrace, stop
				if (trace->predecessors.first == nullref)
				{
					return nullptr;
				}

				// for merge trace, go to the forwardTrace of its commonForwardTrace
				if (trace->predecessors.first != trace->predecessors.last)
				{
					return GetTrace(GetTraceExec(GetTrace(traceExec->branchData.commonForwardBranch)->traceExecRef)->branchData.forwardTrace);
				}

				// otherwise, it is a successor of a branch trace
				// go to its predecessor's forwardTrace
				return GetTrace(GetTraceExec(GetTrace(trace->predecessors.first)->traceExecRef)->branchData.forwardTrace);
			}

			void TraceManager::BuildAmbiguityStructures()
			{
#define TRACE_MAMAGER_PHRASE L"PrepareTraceRoute/BuildAmbiguityStructures"
				IterateSurvivedTraces(
					[this](Trace* trace, Trace* predecessor, vint32_t visitCount, vint32_t predecessorCount)
					{
						auto traceExec = GetTraceExec(trace->traceExecRef);
						if (predecessorCount == 0)
						{
							// for initialTrace, forwardTrace is itself
							traceExec->branchData.forwardTrace = trace;
						}
						else if (predecessorCount == 1)
						{
							if (predecessor->successors.first != predecessor->successors.last)
							{
								// for any successors of a branch trace, forwardTrace is itself
								traceExec->branchData.forwardTrace = trace;
							}
							else
							{
								// if any ordinary trace, use the data from its predecessor
								traceExec->branchData.forwardTrace = GetTraceExec(predecessor->traceExecRef)->branchData.forwardTrace ;
							}
						}
						else
						{
							if (predecessor->state == -1)
							{
								throw TraceException(*this, trace, predecessor, TRACE_MAMAGER_PHRASE, L"Predecessor trace of a merge trace cannot be a merge trace.");
							}

							if (visitCount == 1)
							{
								// for any merge trace, forwardTrace is itself
								traceExec->branchData.forwardTrace = trace;

								// for the first visiting, set commonForwardBranch to the forwardTrace of its first predecessor
								traceExec->branchData.commonForwardBranch = GetTrace(GetTraceExec(predecessor->traceExecRef)->branchData.forwardTrace);
							}
							else
							{
								// find the latest forwardTrace of its commonForwardBranch and the forwardTrace of the predecessor
								NEW_MERGE_STACK_MAGIC_COUNTER;
								auto magicCommonForward = MergeStack_MagicCounter;

								auto currentTrace = GetTrace(traceExec->branchData.commonForwardBranch);
								while (currentTrace)
								{
									GetTraceExec(currentTrace->traceExecRef)->branchData.mergeCounter = magicCommonForward;
									currentTrace = StepForward(currentTrace);
								}

								currentTrace = GetTrace(GetTraceExec(predecessor->traceExecRef)->branchData.forwardTrace);
								while (currentTrace)
								{
									if (GetTraceExec(currentTrace->traceExecRef)->branchData.mergeCounter == magicCommonForward)
									{
										break;
									}
									currentTrace = StepForward(currentTrace);
								}
								if (currentTrace == nullptr)
								{
									throw TraceException(*this, currentTrace, nullptr, TRACE_MAMAGER_PHRASE, L"Cannot determine commonForwardBranch of a merge trace.");
								}
								traceExec->branchData.commonForwardBranch = currentTrace;
							}
						}
					}
				);
#undef TRACE_MAMAGER_PHRASE
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}