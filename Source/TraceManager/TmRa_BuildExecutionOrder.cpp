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
				auto critical = GetTrace(GetTraceExec(startTrace->traceExecRef)->branchData.forwardTrace);
				while (critical)
				{
					if (critical->traceExecRef >= startTrace->traceExecRef)
					{
						break;
					}
					auto criticalRef = GetTraceExec(critical->traceExecRef)->nextAmbiguityCriticalTrace;
					critical = criticalRef == nullref ? nullptr : GetTrace(criticalRef);
				}

				// traverse critical until we hit endTrace
				while (true)
				{
					// if critical is empty
					// or critical is after endTrace
					// or critical is endTrace and its ambiguous prefix is after endIns

					if (critical)
					{
						if (critical->traceExecRef < endTrace->traceExecRef)
						{
							goto CONTINUE_SEARCHING;
						}
						if (critical == endTrace)
						{
							auto criticalExec = GetTraceExec(critical->traceExecRef);
							if (criticalExec->ambiguityBegin != nullref)
							{
								auto ta = GetTraceAmbiguity(criticalExec->ambiguityBegin);
								if (ta->prefix > endIns)
								{
									goto CONTINUE_SEARCHING;
								}
							}
						}
					}

					// it means we have reached the end
					{
						auto step = GetExecutionStep(executionSteps.Allocate());
						step->et_i.startTrace = startTrace->allocatedIndex;
						step->et_i.startIns = startIns;
						step->et_i.endTrace = endTrace->allocatedIndex;
						step->et_i.endIns = endIns;
						AppendStepNode(step, true, root, firstLeaf, currentStep, currentLeaf);
						return;
					}

				CONTINUE_SEARCHING:

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
					else if (critical->predecessors.siblingPrev != critical->predecessors.siblingNext)
					{
						// if critical is a predecessor of a merge tree
						CHECK_FAIL(L"BuildStepTree not implemented!");
					}
					else
					{
						// this happens when the forward trace is not critical
					}

					auto criticalRef = GetTraceExec(critical->traceExecRef)->nextAmbiguityCriticalTrace;
					critical = criticalRef == nullref ? nullptr : GetTrace(criticalRef);
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
BuildAmbiguousStepLink
***********************************************************************/

			void TraceManager::BuildAmbiguousStepLink(TraceAmbiguity* ta, ExecutionStep*& first, ExecutionStep*& last)
			{
				CHECK_FAIL(L"BuildAmbiguousStepLink not implemented!");
			}

/***********************************************************************
BuildExecutionOrder
***********************************************************************/

			void TraceManager::BuildExecutionOrder()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CheckMergeTraces()#"
				// get the instruction range
				auto startTrace = initialTrace;
				vint32_t startIns = 0;
				auto endTrace = concurrentTraces->Get(0);
				vint32_t endIns = GetTraceExec(endTrace->traceExecRef)->insLists.c3 - 1;

				// build step tree
				ExecutionStep* root = nullptr;
				ExecutionStep* firstLeaf = nullptr;
				BuildStepTree(startTrace, startIns, endTrace, endIns, root, firstLeaf, nullptr, nullptr);

				// BuildAmbiguousStepLink should have merged a tree to a link
				CHECK_ERROR(firstLeaf != nullptr, ERROR_MESSAGE_PREFIX L"Ambiguity is not fully identified.");
				CHECK_ERROR(firstLeaf->next == nullref, ERROR_MESSAGE_PREFIX L"Ambiguity is not fully identified.");

				// fill firstStep
				ExecutionStep* first = nullptr;
				ExecutionStep* last = nullptr;
				ConvertStepTreeToLink(root, firstLeaf, first, last);
				firstStep = first;
#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}