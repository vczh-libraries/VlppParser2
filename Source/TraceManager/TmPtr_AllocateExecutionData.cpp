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
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::AllocateExecutionData()#"
				vint32_t insExecCount = 0;
				auto nextBranchTrace = &firstBranchTrace;
				auto nextMergeTrace = &firstMergeTrace;
				IterateSurvivedTraces([&](Trace* trace, Trace* predecessor, vint32_t visitCount, vint32_t predecessorCount)
				{
					// ensure traceExecRef reflects the partial order of the execution order of traces
					if (predecessorCount > 1 && visitCount != predecessorCount) return;

					CHECK_ERROR(trace->traceExecRef == nullref, ERROR_MESSAGE_PREFIX L"Internal error: IterateSurvivedTraces unexpectedly revisit a trace.");
					trace->traceExecRef = traceExecs.Allocate();

					auto traceExec = GetTraceExec(trace->traceExecRef);
					traceExec->traceId = trace;
					ReadInstructionList(trace, traceExec->insLists);
					if (traceExec->insLists.c3 > 0)
					{
						traceExec->insExecRefs.start = insExecCount;
						traceExec->insExecRefs.count = traceExec->insLists.c3;
						insExecCount += traceExec->insLists.c3;
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
#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}