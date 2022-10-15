#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
BuildStepTree
***********************************************************************/

			void TraceManager::BuildStepTree(Trace* startTrace, vint32_t startIns, Trace* endTrace, vint32_t endIns, ExecutionStep*& root, ExecutionStep*& firstLeaf)
			{
				CHECK_FAIL(L"BuildStepTree not implemented!");
			}

/***********************************************************************
ConvertStepTreeToLink
***********************************************************************/

			void TraceManager::ConvertStepTreeToLink(ExecutionStep* root, ExecutionStep* firstLeaf, ExecutionStep*& first, ExecutionStep*& last)
			{
				CHECK_FAIL(L"ConvertStepTreeToLink not implemented!");
			}

/***********************************************************************
BuildExecutionOrder
***********************************************************************/

			void TraceManager::BuildExecutionOrder()
			{
				auto startTrace = initialTrace;
				vint32_t startIns = 0;
				auto endTrace = concurrentTraces->Get(0);
				vint32_t endIns = GetTraceExec(endTrace->traceExecRef)->insLists.c3 - 1;

				ExecutionStep* root = nullptr;
				ExecutionStep* firstLeaf = nullptr;
				BuildStepTree(startTrace, startIns, endTrace, endIns, root, firstLeaf);

				ExecutionStep* first = nullptr;
				ExecutionStep* last = nullptr;
				ConvertStepTreeToLink(root, firstLeaf, first, last);
				firstStep = first;
			}
		}
	}
}