#include "TraceManager.h"

#define DEFINE_EXECUTION_STEP_CONTEXT ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep*& currentStep, ExecutionStep*& currentLeaf
#define PASS_EXECUTION_STEP_CONTEXT root, firstLeaf, currentStep, currentLeaf

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
MarkNewLeafStep
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
/***********************************************************************
AppendStepLink
***********************************************************************/

			void TraceManager::AppendStepLink(ExecutionStep* first, ExecutionStep* last, DEFINE_EXECUTION_STEP_CONTEXT)
			{
				if (!root)
				{
					root = first;
				}

				first->parent = currentStep;
				currentStep = last;
			}

/***********************************************************************
AppendStepsBeforeAmbiguity
***********************************************************************/

			void TraceManager::AppendStepsBeforeAmbiguity(Trace* startTrace, vint32_t startIns, TraceAmbiguity* ta, DEFINE_EXECUTION_STEP_CONTEXT)
			{
				// append a step from current position to the beginning of TraceAmbiguity
				auto taFirst = GetTrace(ta->firstTrace);
				auto taFirstExec = GetTraceExec(taFirst->traceExecRef);
				if ( taFirst->traceExecRef > startTrace->traceExecRef ||
					(taFirst->traceExecRef == startTrace->traceExecRef && ta->prefix > startIns))
				{
					if (ta->prefix > taFirstExec->insLists.countAll)
					{
						// if the first ambiguous instruction is in successors of the branch trace
						// execution from the current position to the end of the prefix
						if (startTrace != taFirst || startIns < taFirstExec->insLists.countAll)
						{
							auto step = GetExecutionStep(executionSteps.Allocate());
							step->et_i.startTrace = startTrace->allocatedIndex;
							step->et_i.startIns = startIns;
							step->et_i.endTrace = taFirst->allocatedIndex;
							step->et_i.endIns = taFirstExec->insLists.countAll - 1;
							AppendStepLink(step, step, PASS_EXECUTION_STEP_CONTEXT);
						}
						if (ta->prefix > taFirstExec->insLists.countAll)
						{
							auto prefixTrace = GetTrace(taFirst->successors.first);
							auto step = GetExecutionStep(executionSteps.Allocate());
							step->et_i.startTrace = prefixTrace->allocatedIndex;
							step->et_i.startIns = 0;
							step->et_i.endTrace = prefixTrace->allocatedIndex;
							step->et_i.endIns = ta->prefix - taFirstExec->insLists.countAll - 1;
							AppendStepLink(step, step, PASS_EXECUTION_STEP_CONTEXT);
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
							AppendStepLink(step, step, PASS_EXECUTION_STEP_CONTEXT);
						}
					}
				}
				{
					auto step = GetExecutionStep(executionSteps.Allocate());
					step->type = ExecutionType::RA_Begin;
					step->et_ra.trace = startTrace->allocatedIndex;
					step->et_ra.type = -1;
					step->et_ra.count = -1;
					AppendStepLink(step, step, PASS_EXECUTION_STEP_CONTEXT);
				}
			}

/***********************************************************************
AppendStepsAfterAmbiguity
***********************************************************************/

			void TraceManager::AppendStepsAfterAmbiguity(Trace*& startTrace, vint32_t& startIns, TraceAmbiguity* ta, DEFINE_EXECUTION_STEP_CONTEXT)
			{
				auto taLast = GetTrace(ta->lastTrace);
				auto taLastExec = GetTraceExec(taLast->traceExecRef);
				if (ta->postfix > taLastExec->insLists.countAll)
				{
					// if the last ambiguous instruction is in predecessors of the merge trace
					// execute the postfix
					auto postfixTrace = GetTrace(taLast->predecessors.first);
					auto postfixTraceExec = GetTraceExec(postfixTrace->traceExecRef);
					{
						auto step = GetExecutionStep(executionSteps.Allocate());
						step->et_i.startTrace = postfixTrace->allocatedIndex;
						step->et_i.startIns = postfixTraceExec->insLists.countAll - (ta->postfix - taLastExec->insLists.countAll);
						step->et_i.endTrace = postfixTrace->allocatedIndex;
						step->et_i.endIns = postfixTraceExec->insLists.countAll - 1;
						AppendStepLink(step, step, PASS_EXECUTION_STEP_CONTEXT);
					}

					// set the corrent position to the beginning of taList
					startTrace = taLast;
					startIns = 0;
				}
				else
				{
					// otherwise set the current position to the instruction after the last ambiguous instruction
					startTrace = taLast;
					startIns = GetTraceExec(startTrace->traceExecRef)->insLists.countAll - ta->postfix;
				}
			}

/***********************************************************************
AppendStepsForAmbiguity
***********************************************************************/

			void TraceManager::AppendStepsForAmbiguity(TraceAmbiguity* ta, bool checkCoveredMark, DEFINE_EXECUTION_STEP_CONTEXT)
			{
				ExecutionStep* taStepFirst = nullptr;
				ExecutionStep* taStepLast = nullptr;
				BuildAmbiguousStepLink(ta, checkCoveredMark, taStepFirst, taStepLast);
				AppendStepLink(taStepFirst, taStepLast, PASS_EXECUTION_STEP_CONTEXT);
			}

/***********************************************************************
AppendStepsBeforeBranch
***********************************************************************/

			void TraceManager::AppendStepsBeforeBranch(Trace* startTrace, vint32_t startIns, Trace* branchTrace, TraceExec* branchTraceExec, DEFINE_EXECUTION_STEP_CONTEXT)
			{
				if (startTrace->traceExecRef < branchTrace->traceExecRef ||
					(startTrace->traceExecRef == branchTrace->traceExecRef && startIns < branchTraceExec->insLists.countAll))
				{
					auto step = GetExecutionStep(executionSteps.Allocate());
					step->et_i.startTrace = startTrace->allocatedIndex;
					step->et_i.startIns = startIns;
					step->et_i.endTrace = branchTrace->allocatedIndex;
					step->et_i.endIns = branchTraceExec->insLists.countAll - 1;
					AppendStepLink(step, step, PASS_EXECUTION_STEP_CONTEXT);
				}
			}

/***********************************************************************
BuildStepTree
***********************************************************************/

			void TraceManager::BuildStepTree(Trace* startTrace, vint32_t startIns, Trace* endTrace, vint32_t endIns, ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep* currentStep, ExecutionStep*& currentLeaf, bool ambiguityBranch)
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
								auto taLinkRef = criticalExec->ambiguityBegins;
								while (taLinkRef != nullref)
								{
									auto taLink = GetTraceAmbiguityLink(taLinkRef);
									taLinkRef = taLink->previous;

									auto ta = GetTraceAmbiguity(taLink->ambiguity);
									if (ta->prefix < endIns)
									{
										goto CONTINUE_SEARCHING;
									}
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
							// check if the only one TraceAmbiguity covers all successors
							bool singleCompleteAmbiguity = true;
							{
								auto firstSuccessor = GetTrace(critical->successors.first);
								auto successorId = firstSuccessor->successors.siblingNext;
								auto covered = GetTraceExec(firstSuccessor->traceExecRef)->ambiguityCoveredInForward;
								while (successorId != nullref)
								{
									auto successor = GetTrace(successorId);
									successorId = successor->successors.siblingNext;

									if (covered != GetTraceExec(successor->traceExecRef)->ambiguityCoveredInForward)
									{
										singleCompleteAmbiguity = false;
										break;
									}
								}
							}

							if (singleCompleteAmbiguity)
							{
								// if yes, it means the TraceAmbiguity will cover all successors
								// run the ambiguity in place, no need for recursion
								auto taLink = GetTraceAmbiguityLink(criticalExec->ambiguityBegins);
								auto ta = GetTraceAmbiguity(taLink->ambiguity);

								// append steps for ambiguity and fix the current position
								AppendStepsBeforeAmbiguity(startTrace, startIns, ta, PASS_EXECUTION_STEP_CONTEXT);
								AppendStepsForAmbiguity(ta, false, PASS_EXECUTION_STEP_CONTEXT);
								AppendStepsAfterAmbiguity(startTrace, startIns, ta, PASS_EXECUTION_STEP_CONTEXT);

								// fix critical
								critical = GetTrace(GetTraceExec(startTrace->traceExecRef)->branchData.forwardTrace);
								continue;
							}
							else
							{
								// there could be one or more TraceAmbiguity
								// there could also be successors that are not covered by any TraceAmbiguity
								auto taLinkRef = criticalExec->ambiguityBegins;
								while (taLinkRef != nullref)
								{
									auto taLink = GetTraceAmbiguityLink(taLinkRef);
									taLinkRef = taLink->previous;
									auto ta = GetTraceAmbiguity(taLink->ambiguity);

									auto branchStartTrace = startTrace;
									auto branchStartIns = startIns;
									auto branchStep = currentStep;

#define PASS_BRANCH_STEP_CONTEXT	root, firstLeaf, branchStep, currentLeaf
									AppendStepsBeforeAmbiguity(branchStartTrace, branchStartIns, ta, PASS_BRANCH_STEP_CONTEXT);
									AppendStepsForAmbiguity(ta, true, PASS_BRANCH_STEP_CONTEXT);
									AppendStepsAfterAmbiguity(branchStartTrace, branchStartIns, ta, PASS_BRANCH_STEP_CONTEXT);
									BuildStepTree(branchStartTrace, branchStartIns, endTrace, endIns, PASS_BRANCH_STEP_CONTEXT, ambiguityBranch);
#undef PASS_BRANCH_STEP_CONTEXT
								}

								// treat the remaining successors as from a branch trace
								AppendStepsBeforeBranch(startTrace, startIns, critical, criticalExec, PASS_EXECUTION_STEP_CONTEXT);

								auto successorId = critical->successors.first;
								while (successorId != nullref)
								{
									auto successor = GetTrace(successorId);
									successorId = successor->successors.siblingNext;
									if (GetTraceExec(successor->traceExecRef)->ambiguityCoveredInForward == nullref)
									{
										BuildStepTree(successor, 0, endTrace, endIns, PASS_EXECUTION_STEP_CONTEXT, true);
									}
								}
								return;
							}
						}
						else if (critical->successors.first != critical->successors.last)
						{
							// if critical is a branch tree

							// append a step current position to the end of critical
							AppendStepsBeforeBranch(startTrace, startIns, critical, criticalExec, PASS_EXECUTION_STEP_CONTEXT);

							// recursively process all successors
							auto successorId = critical->successors.first;
							while (successorId != nullref)
							{
								auto successor = GetTrace(successorId);
								successorId = successor->successors.siblingNext;
								BuildStepTree(successor, 0, endTrace, endIns, PASS_EXECUTION_STEP_CONTEXT, true);
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
								endIns = criticalExec->insLists.countAll + endIns;
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
					AppendStepLink(step, step, PASS_EXECUTION_STEP_CONTEXT);
				}

				if (ambiguityBranch)
				{
					auto step = GetExecutionStep(executionSteps.Allocate());
					step->type = ExecutionType::RA_Branch;
					step->et_ra.trace = startTrace->allocatedIndex;
					step->et_ra.type = -1;
					step->et_ra.count = -1;
					AppendStepLink(step, step, PASS_EXECUTION_STEP_CONTEXT);
				}
				MarkNewLeafStep(currentStep, firstLeaf, currentLeaf);
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

			void TraceManager::BuildAmbiguousStepLink(TraceAmbiguity* ta, bool checkCoveredMark, ExecutionStep*& first, ExecutionStep*& last)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::BuildAmbiguousStepLink()#"
				auto taFirst = GetTrace(ta->firstTrace);
				auto taFirstExec = GetTraceExec(taFirst->traceExecRef);
				auto taLast = GetTrace(ta->lastTrace);
				auto taLastExec = GetTraceExec(taLast->traceExecRef);

				ExecutionStep* root = GetExecutionStep(executionSteps.Allocate());
				root->type = ExecutionType::Empty;

				ExecutionStep* firstLeaf = nullptr;
				ExecutionStep* currentLeaf = nullptr;

				if (ta->prefix < taFirstExec->insLists.countAll)
				{
					// if the first ambiguous instruction is in taFirst

					// traverse all successors
					auto successorId = taFirst->successors.first;
					while (successorId != nullref)
					{
						auto successor = GetTrace(successorId);
						successorId = successor->successors.siblingNext;
						if (checkCoveredMark && GetTraceExec(successor->traceExecRef)->ambiguityCoveredInForward != ta)
						{
							continue;
						}

						// append a step to execute from the first ambiguous instruction
						auto first = GetExecutionStep(executionSteps.Allocate());
						first->parent = root;
						first->et_i.startTrace = taFirst->allocatedIndex;
						first->et_i.startIns = ta->prefix;
						first->et_i.endTrace = taFirst->allocatedIndex;
						first->et_i.endIns = taFirstExec->insLists.countAll - 1;

						// run from successor to the end
						BuildStepTree(
							successor, 0,
							taLast, taLastExec->insLists.countAll - ta->postfix - 1,
							root, firstLeaf, first, currentLeaf,
							true
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
						if (checkCoveredMark && GetTraceExec(successor->traceExecRef)->ambiguityCoveredInForward != ta)
						{
							continue;
						}

						// run from the first ambiguous instruction to the last
						BuildStepTree(
							successor, ta->prefix - taFirstExec->insLists.countAll,
							taLast, taLastExec->insLists.countAll - ta->postfix - 1,
							root, firstLeaf, root, currentLeaf,
							true
							);
					}
				}

				// create the ResolveAmbiguity step
				auto stepRA = GetExecutionStep(executionSteps.Allocate());
				stepRA->type = ExecutionType::RA_End;
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
					auto currentStackLinkRef = ta->bottomCreateObjectStacks;
					while (currentStackLinkRef != nullref)
					{
						auto currentStackLink = GetInsExec_StackRefLink(currentStackLinkRef);
						currentStackLinkRef = currentStackLink->previous;

						auto ieObject = GetInsExec_Stack(currentStackLink->id);

						// find the first StackBegin instruction
						auto ieInsRefLocal = ieObject->summarizing.earliestLocalInsRef;
						auto ieLocalTrace = GetTrace(ieInsRefLocal.trace);
						auto ieLocalTraceExec = GetTraceExec(ieLocalTrace->traceExecRef);
						auto ieLocal = GetInsExec(ieLocalTraceExec->insExecRefs.start + ieInsRefLocal.ins);

						// find the stack of that StackBegin instruction
						auto&& ieInsRef = ReadInstruction(ieInsRefLocal.ins, ieLocalTraceExec->insLists);
						CHECK_ERROR(ieInsRef.type == AstInsType::StackBegin, ERROR_MESSAGE_PREFIX L"earliestLocalInsRef should be StackBegin.");
						CHECK_ERROR(ieLocal->operatingStacks != nullref, ERROR_MESSAGE_PREFIX L"StackBegin should produce exactly one operatingStacks.");
						auto ieOSRefLink = GetInsExec_StackRefLink(ieLocal->operatingStacks);
						CHECK_ERROR(ieOSRefLink->previous == nullref, ERROR_MESSAGE_PREFIX L"StackBegin should produce exactly one operatingStacks.");
						auto ieLocalObject = GetInsExec_Stack(ieOSRefLink->id);

						// find all CreateObject instructions in that stack
						CHECK_ERROR(ieLocalObject->createObjectInsRefs != nullref, ERROR_MESSAGE_PREFIX L"Stack from earliestLocalInsRef should have CreateObject executed.");
						auto coInsRefLink = ieLocalObject->createObjectInsRefs;
						while (coInsRefLink != nullref)
						{
							auto coInsRef = GetInsExec_InsRefLink(coInsRefLink);
							coInsRefLink = coInsRef->previous;

							auto coTrace = GetTrace(coInsRef->insRef.trace);
							auto coTraceExec = GetTraceExec(coTrace->traceExecRef);
							auto&& coIns = ReadInstruction(coInsRef->insRef.ins, coTraceExec->insLists);
							CHECK_ERROR(coIns.type == AstInsType::CreateObject && coIns.param != -1, ERROR_MESSAGE_PREFIX L"createObjectInsRefs points to an unexpected instruction.");

							if (stepRA->et_ra.type == -1)
							{
								stepRA->et_ra.type = coIns.param;
							}
							else if (stepRA->et_ra.type != coIns.param)
							{
								vint32_t baseClass = typeCallback->FindCommonBaseClass(stepRA->et_ra.type, coIns.param);
								if (baseClass == -1)
								{
									throw UnableToResolveAmbiguityException(
										WString::Unmanaged(L"Unable to resolve ambiguity type from ") +
										typeCallback->GetClassName(stepRA->et_ra.type) +
										WString::Unmanaged(L" and ") +
										typeCallback->GetClassName(coIns.param) +
										WString::Unmanaged(L"."),
										stepRA->et_ra.type,
										coIns.param,
										EnsureTraceWithValidStates(taFirst)->currentTokenIndex,
										EnsureTraceWithValidStates(taLast)->currentTokenIndex
									);
								}
								stepRA->et_ra.type = baseClass;
							}
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
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::BuildExecutionOrder()#"
				// get the instruction range
				auto startTrace = initialTrace;
				vint32_t startIns = 0;
				auto endTrace = concurrentTraces->Get(0);
				vint32_t endIns = GetTraceExec(endTrace->traceExecRef)->insLists.countAll - 1;

				// build step tree
				ExecutionStep* root = nullptr;
				ExecutionStep* firstLeaf = nullptr;
				ExecutionStep* currentLeaf = nullptr;
				BuildStepTree(startTrace, startIns, endTrace, endIns, root, firstLeaf, nullptr, currentLeaf, false);

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

#undef PASS_EXECUTION_STEP_CONTEXT
#undef DEFINE_EXECUTION_STEP_CONTEXT