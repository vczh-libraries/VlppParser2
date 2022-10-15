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

			void TraceManager::AppendStepNode(ExecutionStep* step, bool leafNode, ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep*& currentStep, ExecutionStep*& currentLeaf)
			{
				if (!root)
				{
					root = step;
				}

				step->parent = currentStep;
				currentStep = step;

				if (leafNode)
				{
					if (!firstLeaf)
					{
						firstLeaf = step;
					}

					if (currentLeaf)
					{
						currentLeaf->next = step;
					}
					currentLeaf = step;
				}
			}

			void TraceManager::BuildStepTree(Trace* startTrace, vint32_t startIns, Trace* endTrace, vint32_t endIns, ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep* currentStep, ExecutionStep* currentLeaf)
			{
				// find the next critical trace record which is or after startTrace
				auto forwardTrace = GetTrace(GetTraceExec(startTrace->traceExecRef)->branchData.forwardTrace);
				auto critical = GetTrace(GetTraceExec(forwardTrace->traceExecRef)->nextAmbiguityCriticalTrace);
				while (critical)
				{
					if (critical->traceExecRef >= startTrace->traceExecRef)
					{
						break;
					}
				}

				// traverse critical until we hit endTrace
				while (true)
				{
					// if critical is empty, it means nothing is between start and end
					if (!critical)
					{
						auto step = GetExecutionStep(executionSteps.Allocate());
						step->et_i.startTrace = startTrace->allocatedIndex;
						step->et_i.startIns = startIns;
						step->et_i.endTrace = endTrace->allocatedIndex;
						step->et_i.endIns = endIns;
						AppendStepNode(step, true, root, firstLeaf, currentStep, currentLeaf);
						return;
					}

					// there is three kinds of critical node:
					//   ambiguous trace (could also be a branch tree)
					//   branch trace
					//   predecessor of merge trace

					auto criticalExec = GetTraceExec(critical->traceExecRef);
					if (criticalExec->ambiguityBegin != nullref)
					{
						// if critical is an ambiguous trace
						CHECK_FAIL(L"BuildStepTree not implemented!");
					}
					else if (critical->successors.first != critical->successors.last)
					{
						// if critical is a branch tree
						CHECK_FAIL(L"BuildStepTree not implemented!");
					}
					else
					{
						// if critical is a predecessor of a merge tree
						CHECK_FAIL(L"BuildStepTree not implemented!");
					}
				}
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
				BuildStepTree(startTrace, startIns, endTrace, endIns, root, firstLeaf, nullptr, nullptr);

				ExecutionStep* first = nullptr;
				ExecutionStep* last = nullptr;
				ConvertStepTreeToLink(root, firstLeaf, first, last);
				firstStep = first;
			}
		}
	}
}