#include "TraceManager.h"

#define DEFINE_EXECUTION_STEP_CONTEXT ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep*& currentStep, ExecutionStep*& currentLeaf
#define PASS_EXECUTION_STEP_CONTEXT root, firstLeaf, currentStep, currentLeaf

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;
#define TRACE_MAMAGER_PHRASE L"ResolveAmbiguity/BuildExecutionOrder"

/***********************************************************************
AppendStepAfterList
***********************************************************************/

			void TraceManager::AppendStepsAfterList(ExecutionStepLinkedList steps, ExecutionStepLinkedList& current)
			{
				if (!steps.first)
				{
					return;
				}
				if (!current.first)
				{
					current = steps;
				}
				else
				{
					steps.first->parent = current.last;
					current.last = steps.last;
				}
			}

/***********************************************************************
BuildStepListForAmbiguity
***********************************************************************/

			ExecutionStepLinkedList TraceManager::BuildStepListForAmbiguity(TraceAmbiguity* ta)
			{
				ExecutionStepLinkedList result;
				auto taFirst = GetTrace(ta->firstTrace);
				auto taLast = GetTrace(ta->lastTrace);
				auto taBranch = GetTrace(ta->branchTrace);
				auto taMerge = GetTrace(ta->mergeTrace);
				auto taFirstExec = GetTraceExec(taFirst->traceExecRef);
				auto taLastExec = GetTraceExec(taLast->traceExecRef);
				auto taBranchExec = GetTraceExec(taBranch->traceExecRef);
				auto taMergeExec = GetTraceExec(taMerge->traceExecRef);
				vint32_t prefixExtra = ta->prefix - taFirstExec->insLists.countAll;
				vint32_t postfixExtra = ta->postfix - taLastExec->insLists.countAll;

				// Find the first nested TraceAmbiguity between taFirst and taBranch
				{
					auto criticalTrace = taFirst;
					while (criticalTrace && criticalTrace->traceExecRef <= taBranch->traceExecRef)
					{
						auto criticalTraceExec = GetTraceExec(criticalTrace->traceExecRef);
						if (criticalTraceExec->ambiguityBegins != nullref)
						{
							auto taLink = GetTraceAmbiguityLink(criticalTraceExec->ambiguityBegins);
							auto nestedTa = GetTraceAmbiguity(taLink->ambiguity);
							if (nestedTa != ta)
							{
								auto nestedTaLast = GetTrace(nestedTa->lastTrace);
								if (nestedTaLast->traceExecRef > taBranch->traceExecRef)
								{
									CHECK_FAIL(L"Not Implemented!");
								}
								else
								{
									criticalTrace = GetTrace(nestedTa->mergeTrace);
									continue;
								}
							}
						}

						auto criticalRef = criticalTraceExec->nextAmbiguityCriticalTrace;
						criticalTrace = criticalRef == nullref ? nullptr : GetTrace(criticalRef);
					}
				}

				// Append RA_BEGIN
				{
					auto step = GetExecutionStep(executionSteps.Allocate());
					step->type = ExecutionType::RA_Begin;
					step->et_ra.trace = taFirst->allocatedIndex;
					step->et_ra.type = -1;
					AppendStepsAfterList({ step,step }, result);
				}

				// Nested TraceAmbiguity is not implemented, so all successors of taBranch are needed
				auto successorId = taBranch->successors.first;
				while (successorId != nullref)
				{
					auto successor = GetTrace(successorId);
					successorId = successor->successors.siblingNext;

					// Execute from taFirst to taBranch
					if (prefixExtra < 0)
					{
						auto steps = BuildStepList(
							taFirst,
							ta->prefix,
							taBranch,
							taBranchExec->insLists.countAll - 1
						);
						AppendStepsAfterList(steps, result);
					}

					// Execute the branch
					{
						auto steps = BuildStepList(
							successor,
							(prefixExtra <= 0 ? 0 : prefixExtra),
							taMerge,
							(taMergeExec->insLists.countAll - 1 - (postfixExtra <= 0 ? 0 : postfixExtra))
						);
						AppendStepsAfterList(steps, result);
					}

					// Execute from taMerge to taLast
					if (postfixExtra < 0)
					{
						auto steps = BuildStepList(
							taMerge,
							0,
							taLast,
							-postfixExtra - 1
						);
						AppendStepsAfterList(steps, result);
					}

					// Append RA_BRANCH
					{
						auto step = GetExecutionStep(executionSteps.Allocate());
						step->type = ExecutionType::RA_Branch;
						step->et_ra.trace = taLast->allocatedIndex;
						step->et_ra.type = -1;
						AppendStepsAfterList({ step,step }, result);
					}
				}

				// Append RA_END
				{
					auto step = GetExecutionStep(executionSteps.Allocate());
					step->type = ExecutionType::RA_End;
					step->et_ra.type = -1;
					step->et_ra.trace = taLast->allocatedIndex;
					{
						if (typeCallback == nullptr)
						{
							throw TraceException(*this, TRACE_MAMAGER_PHRASE, L"Missing ITypeCallback to resolve the type from multiple objects.");
						}
						auto currentStackLinkRef = ta->bottomCreateObjectStacks;
						while (currentStackLinkRef != nullref)
						{
							auto currentStackLink = GetInsExec_StackRefLink(currentStackLinkRef);
							currentStackLinkRef = currentStackLink->previous;
							auto ieObject = GetInsExec_Stack(currentStackLink->id);

							// find all CreateObject instructions in that stack
							if (ieObject->summarizing.indirectCreateObjectInsRefs == nullref)
							{
								throw TraceException(*this, ieObject, TRACE_MAMAGER_PHRASE, L"indirectCreateObjectInsRefs should not be null.");
							}
							auto coInsRefLink = ieObject->summarizing.indirectCreateObjectInsRefs;
							while (coInsRefLink != nullref)
							{
								auto coInsRef = GetInsExec_InsRefLink(coInsRefLink);
								coInsRefLink = coInsRef->previous;

								auto coTrace = GetTrace(coInsRef->insRef.trace);
								auto coTraceExec = GetTraceExec(coTrace->traceExecRef);
								auto&& coIns = ReadInstruction(coInsRef->insRef.ins, coTraceExec->insLists);
								if (coIns.type != AstInsType::CreateObject || coIns.param == -1)
								{
									throw TraceException(*this, ieObject, TRACE_MAMAGER_PHRASE, L"indirectCreateObjectInsRefs points to an unexpected instruction.");
								}

								if (step->et_ra.type == -1)
								{
									step->et_ra.type = coIns.param;
								}
								else if (step->et_ra.type != coIns.param)
								{
									vint32_t baseClass = typeCallback->FindCommonBaseClass(step->et_ra.type, coIns.param);
									if (baseClass == -1)
									{
										throw UnableToResolveAmbiguityException(
											WString::Unmanaged(L"Unable to resolve ambiguity type from ") +
											typeCallback->GetClassName(step->et_ra.type) +
											WString::Unmanaged(L" and ") +
											typeCallback->GetClassName(coIns.param) +
											WString::Unmanaged(L"."),
											step->et_ra.type,
											coIns.param,
											EnsureTraceWithValidStates(taFirst)->currentTokenIndex,
											EnsureTraceWithValidStates(taLast)->currentTokenIndex
										);
									}
									step->et_ra.type = baseClass;
								}
							}
						}
					}
					AppendStepsAfterList({ step,step }, result);
				}

				return result;
			}

/***********************************************************************
BuildStepList
***********************************************************************/

			ExecutionStepLinkedList TraceManager::BuildStepListUntilFirstRawBranchTrace(Trace* startTrace, vint32_t startIns, Trace* endTrace, vint32_t endIns, Trace** rawBranchTrace)
			{
				ExecutionStepLinkedList result;
				Trace* currentTrace = startTrace;
				vint32_t currentIns = startIns;

				while (currentTrace)
				{
					auto currentTraceExec = GetTraceExec(currentTrace->traceExecRef);

					// Find the next critical trace 
					Trace* criticalTrace = nullptr;
					if (currentTraceExec->nextAmbiguityCriticalTrace != nullref)
					{
						criticalTrace = currentTrace;
					}
					else
					{
						criticalTrace = GetTrace(currentTraceExec->branchData.forwardTrace);
					}

					while (criticalTrace && criticalTrace->traceExecRef <= currentTrace->traceExecRef)
					{
						auto nextRef = GetTraceExec(criticalTrace->traceExecRef)->nextAmbiguityCriticalTrace;
						if (nextRef == nullref)
						{
							if (criticalTrace->successors.first != criticalTrace->successors.last)
							{
								throw TraceException(*this, currentTrace, nullptr, TRACE_MAMAGER_PHRASE, L"Failed to find a TraceAmbiguity between this trace and the next branch trace.");
							}
							else
							{
								goto NO_CRITICAL_TRACE;
							}
						}
						criticalTrace = nextRef == nullref ? nullptr : GetTrace(nextRef);
						if (criticalTrace->traceExecRef >= endTrace->traceExecRef)
						{
							goto NO_CRITICAL_TRACE;
						}
						else if (endIns < 0 && criticalTrace->successors.first == endTrace)
						{
							goto NO_CRITICAL_TRACE;
						}
					}

					auto criticalTraceExec = GetTraceExec(criticalTrace->traceExecRef);
					if (criticalTraceExec->ambiguityBegins == nullref)
					{
						if (rawBranchTrace && criticalTrace->successors.first != criticalTrace->successors.last)
						{
							*rawBranchTrace = criticalTrace;
							endTrace = criticalTrace;
							endIns = GetTraceExec(endTrace->traceExecRef)->insLists.countAll - 1;
							goto NO_CRITICAL_TRACE;
						}
						throw TraceException(*this, currentTrace, criticalTrace, TRACE_MAMAGER_PHRASE, L"The next critical trace after the current trace is not associated with a TraceAmbiguity.");
					}

					// Execute from (currentTrace, currentIns) until the next TraceAmbiguity
					auto ta = GetTraceAmbiguity(GetTraceAmbiguityLink(criticalTraceExec->ambiguityBegins)->ambiguity);
					auto taFirst = GetTrace(ta->firstTrace);
					auto taLast = GetTrace(ta->lastTrace);
					auto taFirstExec = GetTraceExec(taFirst->traceExecRef);
					auto taLastExec = GetTraceExec(taLast->traceExecRef);
					vint32_t prefixExtra = ta->prefix - taFirstExec->insLists.countAll;
					vint32_t postfixExtra = ta->postfix - taLastExec->insLists.countAll;

					if (currentTrace->traceExecRef < taFirst->traceExecRef || currentIns < ta->prefix)
					{
						auto step = GetExecutionStep(executionSteps.Allocate());
						step->et_i.startTrace = currentTrace->allocatedIndex;
						step->et_i.startIns = currentIns;
						if (ta->prefix == 0)
						{
							if (taFirst->predecessors.first != taFirst->predecessors.last)
							{
								throw TraceException(*this, ta, nullptr, TRACE_MAMAGER_PHRASE, L"The prefix of the TraceAmbiguity is 0, but its firstTrace is a merge trace.");
							}
							auto taFirstPrev = GetTrace(taFirst->predecessors.first);
							auto taFirstPrevExec = GetTraceExec(taFirstPrev->traceExecRef);
							step->et_i.endTrace = taFirstPrev->allocatedIndex;
							step->et_i.endIns = taFirstPrevExec->insLists.countAll - 1;
						}
						else if (prefixExtra <= 0)
						{
							step->et_i.endTrace = taFirst->allocatedIndex;
							step->et_i.endIns = ta->prefix - 1;
						}
						else
						{
							step->et_i.endTrace = taFirst->allocatedIndex;
							step->et_i.endIns = taFirstExec->insLists.countAll - 1;
						}
						AppendStepsAfterList({ step, step }, result);
					}

					if (prefixExtra > 0)
					{
						// at the moment taFirst should be taBranch
						// TraceAmbiguity begins at each successors of taBranch, instead of before taBranch
						auto taFirstSuccessor = GetTrace(taFirst->successors.first);
						auto step = GetExecutionStep(executionSteps.Allocate());
						step->et_i.startTrace = taFirstSuccessor->allocatedIndex;
						step->et_i.startIns = 0;
						step->et_i.endTrace = taFirstSuccessor->allocatedIndex;
						step->et_i.endIns = prefixExtra - 1;
						AppendStepsAfterList({ step, step }, result);
					}

					// Execute the next TraceAmbiguity
					auto taSteps = BuildStepListForAmbiguity(ta);
					AppendStepsAfterList(taSteps, result);

					// Step (currentTrace, currentIns) forward to right after TraceAmbiguity
					if (postfixExtra > 0)
					{
						// at the moment taLast should be taMerge
						// TraceAmbiguity ends at each predecessor of taMerge, instead of after taMerge
						auto taLastPredecessor = GetTrace(taLast->predecessors.first);
						auto taLastPredecessorExec = GetTraceExec(taLastPredecessor->traceExecRef);
						auto step = GetExecutionStep(executionSteps.Allocate());
						step->et_i.startTrace = taLastPredecessor->allocatedIndex;
						step->et_i.startIns = taLastPredecessorExec->insLists.countAll - postfixExtra;
						step->et_i.endTrace = taLastPredecessor->allocatedIndex;
						step->et_i.endIns = taLastPredecessorExec->insLists.countAll - 1;
						AppendStepsAfterList({ step, step }, result);
					}

					if (ta->postfix == 0)
					{
						if (taLast->successors.first == nullref)
						{
							currentTrace = nullptr;
						}
						else if (taLast->successors.first != taLast->successors.last)
						{
							throw TraceException(*this, ta, nullptr, TRACE_MAMAGER_PHRASE, L"The postfix of the TraceAmbiguity is 0, but its lastTrace is a branch trace.");
						}
						else
						{
							currentTrace = GetTrace(taLast->successors.first);
							currentIns = 0;
						}
					}
					else
					{
						currentTrace = taLast;
						currentIns = -(postfixExtra <= 0 ? postfixExtra : 0);
					}
				}
			NO_CRITICAL_TRACE:

				if (endIns < 0 && endIns != GetTraceExec(endTrace->traceExecRef)->insLists.countAll - 1)
				{
					// The real endTrace is a predecessor of endTrace, but we need to find out which
					auto realEndTrace = currentTrace;
					while (realEndTrace->successors.first != endTrace)
					{
						realEndTrace = GetTrace(realEndTrace->successors.first);
					}

					auto realEndTraceExec = GetTraceExec(realEndTrace->traceExecRef);
					endTrace = realEndTrace;
					endIns = realEndTraceExec->insLists.countAll + endIns;
				}

				if (currentTrace)
				{
					if (
						currentTrace->traceExecRef < endTrace->traceExecRef ||
						(currentTrace->traceExecRef == endTrace->traceExecRef && currentIns <= endIns)
						)
					{
						// Execute from (currentTrace, currentIns) to (endTrace, endIns)
						auto step = GetExecutionStep(executionSteps.Allocate());
						step->et_i.startTrace = currentTrace->allocatedIndex;
						step->et_i.startIns = currentIns;
						step->et_i.endTrace = endTrace->allocatedIndex;
						step->et_i.endIns = endIns;
						AppendStepsAfterList({ step, step }, result);
					}
					else
					{
						throw TraceException(*this, currentTrace, endTrace, TRACE_MAMAGER_PHRASE, L"BuildStepList corrupted with a currentTrace after startIns.");
					}
				}

				return result;
			}

			ExecutionStepLinkedList TraceManager::BuildStepList(Trace* startTrace, vint32_t startIns, Trace* endTrace, vint32_t endIns)
			{
				return BuildStepListUntilFirstRawBranchTrace(startTrace, startIns, endTrace, endIns, nullptr);
			}

/***********************************************************************
BuildExecutionOrder
***********************************************************************/

			void TraceManager::BuildExecutionOrder()
			{
				// get the instruction range
				auto startTrace = initialTrace;
				vint32_t startIns = 0;
				auto endTrace = concurrentTraces->Get(0);
				vint32_t endIns = GetTraceExec(endTrace->traceExecRef)->insLists.countAll - 1;

				auto steps = BuildStepList(startTrace, startIns, endTrace, endIns);
				{
					auto current = steps.last;
					while (current != steps.first)
					{
						auto parent = GetExecutionStep(current->parent);
						parent->next = current;
						current = parent;
					}
				}
				firstStep = steps.first;
			}
#undef TRACE_MAMAGER_PHRASE
		}
	}
}

#undef PASS_EXECUTION_STEP_CONTEXT
#undef DEFINE_EXECUTION_STEP_CONTEXT