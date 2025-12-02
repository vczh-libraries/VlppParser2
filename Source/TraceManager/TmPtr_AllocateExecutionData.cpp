#include "TraceManager.h"
#include "TraceManager_Common.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
AllocateExecutionData
***********************************************************************/

			void TraceManager::AllocateExecutionData()
			{
#define TRACE_MAMAGER_PHRASE L"PrepareTraceRoute/AllocateExecutionData"
				vint32_t insExecCount = 0;
				auto nextBranchTrace = &firstBranchTrace;
				auto nextMergeTrace = &firstMergeTrace;
				IterateSurvivedTraces([&](Trace* trace, Trace* predecessor, vint32_t visitCount, vint32_t predecessorCount)
				{
					// ensure traceExecRef reflects the partial order of the execution order of traces
					if (predecessorCount > 1 && visitCount != predecessorCount) return;

					if (trace->traceExecRef != nullref)
					{
						throw TraceException(*this, trace, nullptr, TRACE_MAMAGER_PHRASE, L"IterateSurvivedTraces unexpectedly revisit a trace.");
					}
					trace->traceExecRef = traceExecs.Allocate();

					auto traceExec = GetTraceExec(trace->traceExecRef);
					traceExec->traceId = trace;
					ReadInstructionList(trace, traceExec->insLists);
					if (traceExec->insLists.countAll > 0)
					{
						traceExec->insExecRefs.start = insExecCount;
						traceExec->insExecRefs.count = traceExec->insLists.countAll;
						insExecCount += traceExec->insLists.countAll;
					}

					// fill branch trace linked list
					if (trace->successors.first != trace->successors.last)
					{
						*nextBranchTrace = trace;
						nextBranchTrace = &traceExec->nextBranchTrace;
					}

					// fill merge branch linked list
					if (trace->predecessors.first != trace->predecessors.last)
					{
						*nextMergeTrace = trace;
						nextMergeTrace = &traceExec->nextMergeTrace;
					}
				});
				insExecs.Resize(insExecCount);
#undef TRACE_MAMAGER_PHRASE
			}
		}
	}
}