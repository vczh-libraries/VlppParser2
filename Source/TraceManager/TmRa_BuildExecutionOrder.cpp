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

			void TraceManager::MarkNewLeafStep(ExecutionStep* step, ExecutionStep*& firstLeaf, ExecutionStep*& currentLeaf)
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

			void TraceManager::AppendStepLink(ExecutionStep* first, ExecutionStep* last, bool leafNode, ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep*& currentStep, ExecutionStep*& currentLeaf)
			{
				if (!root)
				{
					root = first;
				}

				first->parent = currentStep;
				currentStep = last;

				if (leafNode)
				{
					MarkNewLeafStep(last, firstLeaf, currentLeaf);
				}
			}

			void TraceManager::BuildStepTree(Trace* startTrace, vint32_t startIns, Trace* endTrace, vint32_t endIns, ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep* currentStep, ExecutionStep*& currentLeaf)
			{
				// find the next critical trace record which is or after startTrace
				auto critical = GetTrace(GetTraceExec(startTrace->traceExecRef)->branchData.forwardTrace);

				// traverse critical until we hit endTrace
				while (true)
				{
					// skip if critical is before startTrace
					if (critical && critical->traceExecRef < startTrace->traceExecRef)
					{
						goto NEXT_CRITICAL;
					}

					// if critical is empty
					// or critical is after endTrace
					// or critical is endTrace and the first ambiguous instruction is not before endIns

					if (critical)
					{
						if (critical->traceExecRef < endTrace->traceExecRef)
						{
							goto CONTINUE_SEARCHING;
						}
						if (critical == endTrace)
						{
							auto criticalExec = GetTraceExec(critical->traceExecRef);
							if (criticalExec->ambiguityBegins != nullref)
							{
								auto taLink = GetTraceAmbiguityLink(criticalExec->ambiguityBegins);
								CHECK_ERROR(taLink->previous == nullref, L"Not Implemented!");
								auto ta = GetTraceAmbiguity(taLink->ambiguity);
								if (ta->prefix < endIns)
								{
									goto CONTINUE_SEARCHING;
								}
							}
						}
					}

					// it means we have reached the end
					break;

				CONTINUE_SEARCHING:

					// there is three kinds of critical node:
					//   ambiguous trace (could also be a branch tree)
					//   branch trace
					//   predecessor of merge trace

					{
						auto criticalExec = GetTraceExec(critical->traceExecRef);
						if (criticalExec->ambiguityBegins != nullref)
						{
							// if critical is an ambiguous trace
							auto taLink = GetTraceAmbiguityLink(criticalExec->ambiguityBegins);
							CHECK_ERROR(taLink->previous == nullref, L"Not Implemented!");
							auto ta = GetTraceAmbiguity(taLink->ambiguity);
							auto taFirst = GetTrace(ta->firstTrace);
							auto taFirstExec = GetTraceExec(taFirst->traceExecRef);
							auto taLast = GetTrace(ta->lastTrace);
							auto taLastExec = GetTraceExec(taLast->traceExecRef);

							// append a step from current position to the beginning of TraceAmbiguity
							if ( taFirst->traceExecRef > startTrace->traceExecRef ||
								(taFirst->traceExecRef == startTrace->traceExecRef && ta->prefix > startIns))
							{
								if (ta->prefix > taFirstExec->insLists.c3)
								{
									// if the first ambiguous instruction is in successors of the branch trace
									// execution from the current position to the end of the prefix
									if (startTrace != taFirst || startIns < taFirstExec->insLists.c3)
									{
										auto step = GetExecutionStep(executionSteps.Allocate());
										step->et_i.startTrace = startTrace->allocatedIndex;
										step->et_i.startIns = startIns;
										step->et_i.endTrace = taFirst->allocatedIndex;
										step->et_i.endIns = taFirstExec->insLists.c3 - 1;
										AppendStepLink(step, step, false, root, firstLeaf, currentStep, currentLeaf);
									}
									if (ta->prefix > taFirstExec->insLists.c3)
									{
										auto prefixTrace = GetTrace(taFirst->successors.first);
										auto step = GetExecutionStep(executionSteps.Allocate());
										step->et_i.startTrace = prefixTrace->allocatedIndex;
										step->et_i.startIns = 0;
										step->et_i.endTrace = prefixTrace->allocatedIndex;
										step->et_i.endIns = ta->prefix - taFirstExec->insLists.c3 - 1;
										AppendStepLink(step, step, false, root, firstLeaf, currentStep, currentLeaf);
									}
								}
								else
								{
									// execute instructions before the first ambiguous instruction
									if (startTrace != taFirst || startIns < ta->prefix)
									{
										auto step = GetExecutionStep(executionSteps.Allocate());
										step->et_i.startTrace = startTrace->allocatedIndex;
										step->et_i.startIns = startIns;
										step->et_i.endTrace = taFirst->allocatedIndex;
										step->et_i.endIns = ta->prefix - 1;
										AppendStepLink(step, step, false, root, firstLeaf, currentStep, currentLeaf);
									}
								}
							}

							// append the step link from TraceAmbiguity
							{
								ExecutionStep* taStepFirst = nullptr;
								ExecutionStep* taStepLast = nullptr;
								BuildAmbiguousStepLink(ta, taStepFirst, taStepLast);
								AppendStepLink(taStepFirst, taStepLast, false, root, firstLeaf, currentStep, currentLeaf);
							}

							// append a step from the end of TraceAmbiguity to ambiguityEnd
							if (ta->postfix > taLastExec->insLists.c3)
							{
								// if the last ambiguous instruction is in predecessors of the merge trace
								// execute the postfix
								auto postfixTrace = GetTrace(taLast->predecessors.first);
								auto postfixTraceExec = GetTraceExec(postfixTrace->traceExecRef);
								{
									auto step = GetExecutionStep(executionSteps.Allocate());
									step->et_i.startTrace = postfixTrace->allocatedIndex;
									step->et_i.startIns = postfixTraceExec->insLists.c3 - (ta->postfix - taLastExec->insLists.c3);
									step->et_i.endTrace = postfixTrace->allocatedIndex;
									step->et_i.endIns = postfixTraceExec->insLists.c3 - 1;
									AppendStepLink(step, step, false, root, firstLeaf, currentStep, currentLeaf);
								}

								// fix startTrace, startIns
								startTrace = taLast;
								startIns = 0;
							}
							else
							{
								// fix startTrace, startIns
								startTrace = taLast;
								startIns = GetTraceExec(startTrace->traceExecRef)->insLists.c3 - ta->postfix;
							}

							// fix critical
							critical = GetTrace(GetTraceExec(startTrace->traceExecRef)->branchData.forwardTrace);
							continue;
						}
						else if (critical->successors.first != critical->successors.last)
						{
							// if critical is a branch tree

							// append a step current position to the end of critical
							if (startTrace->traceExecRef < critical->traceExecRef ||
								(startTrace->traceExecRef == critical->traceExecRef && startIns < criticalExec->insLists.c3))
							{
								auto step = GetExecutionStep(executionSteps.Allocate());
								step->et_i.startTrace = startTrace->allocatedIndex;
								step->et_i.startIns = startIns;
								step->et_i.endTrace = critical->allocatedIndex;
								step->et_i.endIns = criticalExec->insLists.c3 - 1;
								AppendStepLink(step, step, false, root, firstLeaf, currentStep, currentLeaf);
							}

							// recursively process all successors
							auto successorId = critical->successors.first;
							while (successorId != nullref)
							{
								auto successor = GetTrace(successorId);
								successorId = successor->successors.siblingNext;
								BuildStepTree(successor, 0, endTrace, endIns, root, firstLeaf, currentStep, currentLeaf);
							}
							return;

						}
						else if (critical->predecessors.siblingPrev != critical->predecessors.siblingNext)
						{
							// if critical is a predecessor of a merge tree
							// see if it could be an end
							if (critical->successors.first == endTrace && endIns < 0)
							{
								// fix endTrace and endIns
								endTrace = critical;
								endIns = criticalExec->insLists.c3 + endIns;
								break;
							}
							else
							{
								// otherwise, fix critical
								critical = GetTrace(GetTraceExec(GetTrace(critical->successors.first)->traceExecRef)->branchData.forwardTrace);
								continue;
							}
						}
						else
						{
							// this happens when the forward trace is not critical
						}
					}

				NEXT_CRITICAL:
					auto criticalRef = GetTraceExec(critical->traceExecRef)->nextAmbiguityCriticalTrace;
					critical = criticalRef == nullref ? nullptr : GetTrace(criticalRef);
				}

				if ( startTrace->traceExecRef < endTrace->traceExecRef ||
					(startTrace->traceExecRef == endTrace->traceExecRef && startIns <= endIns))
				{
					auto step = GetExecutionStep(executionSteps.Allocate());
					step->et_i.startTrace = startTrace->allocatedIndex;
					step->et_i.startIns = startIns;
					step->et_i.endTrace = endTrace->allocatedIndex;
					step->et_i.endIns = endIns;
					AppendStepLink(step, step, true, root, firstLeaf, currentStep, currentLeaf);
				}
				else
				{
					MarkNewLeafStep(currentStep, firstLeaf, currentLeaf);
				}
			}

/***********************************************************************
ConvertStepTreeToLink
***********************************************************************/

			void TraceManager::ConvertStepTreeToLink(ExecutionStep* root, ExecutionStep* firstLeaf, ExecutionStep*& first, ExecutionStep*& last)
			{
				// calculate copyCount
				Ref<ExecutionStep> currentLeafRef = firstLeaf;
				while (currentLeafRef != nullref)
				{
					auto currentRef = currentLeafRef;
					while (currentRef != nullref)
					{
						auto current = GetExecutionStep(currentRef);
						current->copyCount++;
						currentRef = current->parent;
					}

					currentLeafRef = GetExecutionStep(currentLeafRef)->next;
				}

				// for each leaf, build a step link from root to the leaf
				// concat all link, fill first and last
				currentLeafRef = firstLeaf;
				while (currentLeafRef != nullref)
				{
					// disconnect currentLeaf to the next leaf
					auto currentLeaf = GetExecutionStep(currentLeafRef);
					auto nextLeafRef = currentLeaf->next;
					currentLeaf->next = nullref;

					// fix next from root to currentLeaf
					auto current = currentLeaf;
					while (current->parent != nullref)
					{
						auto parent = GetExecutionStep(current->parent);
						parent->next = current;
						current = parent;
					}

					// make a step link from root to currentLeaf
					ExecutionStep* linkFirst = nullptr;
					ExecutionStep* linkLast = nullptr;

					Ref<ExecutionStep> currentRef = root;
					while (currentRef != nullref)
					{
						// increase visitCount
						auto current = GetExecutionStep(currentRef);
						current->visitCount++;

						if (current->visitCount == current->copyCount)
						{
							// if visitCount == copyCount
							// it means current will not be copied in the next round
							// sublink from current to currentLeaf copy be used directly
							if (!linkFirst)
							{
								linkFirst = current;
							}
							if (linkLast)
							{
								linkLast->next = current;
							}
							linkLast = currentLeaf;
							break;
						}
						else
						{
							// otherwise, copy current
							static_assert(sizeof(ExecutionStep::ETI) >= sizeof(ExecutionStep::ETRA));
							auto step = GetExecutionStep(executionSteps.Allocate());
							step->type = current->type;
							step->et_i = current->et_i;

							if (!linkFirst)
							{
								linkFirst = step;
							}
							if (linkLast)
							{
								linkLast->next = step;
							}
							linkLast = step;
							currentRef = current->next;
						}
					}

					if (!first)
					{
						first = linkFirst;
					}
					if (last)
					{
						last->next = linkFirst;
					}
					last = linkLast;

					currentLeafRef = nextLeafRef;
				}
			}

/***********************************************************************
BuildAmbiguousStepLink
***********************************************************************/

			void TraceManager::BuildAmbiguousStepLink(TraceAmbiguity* ta, ExecutionStep*& first, ExecutionStep*& last)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CheckMergeTraces()#"
				auto taFirst = GetTrace(ta->firstTrace);
				auto taFirstExec = GetTraceExec(taFirst->traceExecRef);
				auto taLast = GetTrace(ta->lastTrace);
				auto taLastExec = GetTraceExec(taLast->traceExecRef);

				ExecutionStep* root = GetExecutionStep(executionSteps.Allocate());
				root->type = ExecutionType::Empty;

				ExecutionStep* firstLeaf = nullptr;
				ExecutionStep* currentLeaf = nullptr;

				if (ta->prefix < taFirstExec->insLists.c3)
				{
					// if the first ambiguous instruction is in taFirst

					// traverse all successors
					auto successorId = taFirst->successors.first;
					while (successorId != nullref)
					{
						auto successor = GetTrace(successorId);
						successorId = successor->successors.siblingNext;

						// append a step to execute from the first ambiguous instruction
						auto first = GetExecutionStep(executionSteps.Allocate());
						first->parent = root;
						first->et_i.startTrace = taFirst->allocatedIndex;
						first->et_i.startIns = ta->prefix;
						first->et_i.endTrace = taFirst->allocatedIndex;
						first->et_i.endIns = taFirstExec->insLists.c3 - 1;

						// run from successor to the end
						BuildStepTree(
							successor, 0,
							taLast, taLastExec->insLists.c3 - ta->postfix - 1,
							root, firstLeaf, first, currentLeaf
							);
					}
				}
				else
				{
					// if the first ambiguous instruction is in successor traces

					// traverse all successors
					auto successorId = taFirst->successors.first;
					while (successorId != nullref)
					{
						auto successor = GetTrace(successorId);
						successorId = successor->successors.siblingNext;

						// run from the first ambiguous instruction to the last
						BuildStepTree(
							successor, ta->prefix - taFirstExec->insLists.c3,
							taLast, taLastExec->insLists.c3 - ta->postfix - 1,
							root, firstLeaf, root, currentLeaf
							);
					}
				}

				// create the ResolveAmbiguity step
				auto stepRA = GetExecutionStep(executionSteps.Allocate());
				stepRA->type = ExecutionType::ResolveAmbiguity;
				stepRA->et_ra.count = 0;
				stepRA->et_ra.type = -1;
				stepRA->et_ra.trace = taLast->allocatedIndex;
				{
					Ref<ExecutionStep> currentLeafRef = firstLeaf;
					while (currentLeafRef != nullref)
					{
						stepRA->et_ra.count++;
						currentLeafRef = GetExecutionStep(currentLeafRef)->next;
					}
				}
				{
					CHECK_ERROR(typeCallback != nullptr, ERROR_MESSAGE_PREFIX L"Missing ITypeCallback to resolve the type from multiple objects.");
					auto linkRef = ta->bottomObjectIds;
					while (linkRef != nullref)
					{
						auto link = GetInsExec_ObjRefLink(linkRef);
						linkRef = link->previous;

						auto ieObject = GetInsExec_Object(link->id);
						auto ieTrace = GetTrace(ieObject->bo_bolr_Trace);
						auto ieTraceExec = GetTraceExec(ieTrace->traceExecRef);

						auto&& ins = ReadInstruction(ieObject->bo_bolr_Ins, ieTraceExec->insLists);
						if (stepRA->et_ra.type == -1)
						{
							stepRA->et_ra.type = ins.param;
						}
						else
						{
							stepRA->et_ra.type = typeCallback->FindCommonBaseClass(stepRA->et_ra.type, ins.param);
							CHECK_ERROR(stepRA->et_ra.type != -1, ERROR_MESSAGE_PREFIX L"Unable to resolve the type from multiple objects.");
						}
					}
				}

				// append the ResolveAmbiguity step to the step tree
				ConvertStepTreeToLink(root, firstLeaf, first, last);

				auto current = first;
				while (current != last)
				{
					auto next = GetExecutionStep(current->next);
					current->next = nullref;
					next->parent = current;
					current = next;
				}

				stepRA->parent = last;
				last = stepRA;
#undef ERROR_MESSAGE_PREFIX
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
				ExecutionStep* currentLeaf = nullptr;
				BuildStepTree(startTrace, startIns, endTrace, endIns, root, firstLeaf, nullptr, currentLeaf);

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