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
ExecutionStep Operations
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

			void TraceManager::AppendLeafToTree(ExecutionStep* leaf, ExecutionStepTree& tree)
			{
				if (!tree.firstLeaf)
				{
					tree.firstLeaf = leaf;
					tree.lastLeaf = leaf;
				}
				else
				{
					tree.lastLeaf->leafNext = leaf;
					tree.lastLeaf = leaf;
				}
			}

			ExecutionStepLinkedList TraceManager::ConvertStepTreeToList(ExecutionStepTree tree)
			{
				ExecutionStepLinkedList result;

				// initialize visitCount and copyCount
				{
					Ref<ExecutionStep> currentLeafRef = tree.firstLeaf;
					while (currentLeafRef != nullref)
					{
						auto currentLeaf = GetExecutionStep(currentLeafRef);
						currentLeafRef = currentLeaf->leafNext;

						Ref<ExecutionStep> currentStepRef = currentLeaf;
						while (currentStepRef != nullref)
						{
							auto currentStep = GetExecutionStep(currentStepRef);
							currentStepRef = currentStep->parent;

							currentStep->visitCount = 0;
							currentStep->copyCount = 0;
						}
					}
				}
				{
					Ref<ExecutionStep> currentLeafRef = tree.firstLeaf;
					while (currentLeafRef != nullref)
					{
						auto currentLeaf = GetExecutionStep(currentLeafRef);
						currentLeafRef = currentLeaf->leafNext;

						Ref<ExecutionStep> currentStepRef = currentLeaf;
						while (currentStepRef != nullref)
						{
							auto currentStep = GetExecutionStep(currentStepRef);
							currentStepRef = currentStep->parent;

							currentStep->copyCount++;
						}
					}
				}

				// traverse through each leaf
				// make a list from root to leaf
				// join them
				{
					Ref<ExecutionStep> currentLeafRef = tree.firstLeaf;
					while (currentLeafRef != nullref)
					{
						auto currentLeaf = GetExecutionStep(currentLeafRef);
						currentLeafRef = currentLeaf->leafNext;

						ExecutionStepLinkedList leafList{ currentLeaf,currentLeaf };
						auto walkingStep = currentLeaf;
						while (walkingStep->parent != nullref)
						{
							walkingStep = GetExecutionStep(walkingStep->parent);

							if (walkingStep->visitCount++ == walkingStep->copyCount)
							{
								leafList.first = walkingStep;
							}
							else
							{
								auto stepCopy = GetExecutionStep(executionSteps.Allocate());
								static_assert(sizeof(stepCopy->et_i) >= sizeof(stepCopy->et_ra));
								stepCopy->type = walkingStep->type;
								stepCopy->et_i = walkingStep->et_i;
								leafList.first->parent = stepCopy;
								leafList.first = stepCopy;
							}
						}
						AppendStepsAfterList(leafList, result);
					}
				}

				// clean leafPrev and leafNext
				{
					Ref<ExecutionStep> currentLeafRef = tree.firstLeaf;
					while (currentLeafRef != nullref)
					{
						auto currentLeaf = GetExecutionStep(currentLeafRef);
						currentLeafRef = currentLeaf->leafNext;

						currentLeaf->leafNext = nullref;
					}
				}

				return result;
			}

/***********************************************************************
CreateResolveAmbiguityStep
***********************************************************************/

			ExecutionStep* TraceManager::CreateResolveAmbiguityStep(TraceAmbiguity* ta)
			{
				auto taFirst = GetTrace(ta->firstTrace);
				auto taLast = GetTrace(ta->lastTrace);
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
				return step;
			}

/***********************************************************************
CollectNestedAmbiguities
***********************************************************************/

			Ptr<TraceManager::NestedAmbiguityInfo> TraceManager::CollectNestedAmbiguities(TraceAmbiguity* ta)
			{
				auto taFirst = GetTrace(ta->firstTrace);
				auto taBranch = GetTrace(ta->branchTrace);

				Ptr<NestedAmbiguityInfo> nestedTas;
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
									if (!nestedTas) nestedTas = Ptr(new NestedAmbiguityInfo);
									nestedTas->nestedAmbiguities.Add(nestedTa);
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

				if (nestedTas)
				{
					nestedTas->branchTraces.Add(GetTrace(ta->branchTrace), ta);
					for (auto nestedTa : nestedTas->nestedAmbiguities)
					{
						auto branchTrace = GetTrace(nestedTa->branchTrace);
						nestedTas->branchTraces.Add(branchTrace, nestedTa);
					}

					for (auto nestedTa : nestedTas->nestedAmbiguities)
					{
						auto currentTraceRef = nestedTa->branchTrace;
						while (currentTraceRef != nullref)
						{
							auto currentTrace = GetTrace(currentTraceRef);
							if (currentTrace->traceExecRef < taFirst->traceExecRef)
							{
								break;
							}

							if (currentTrace->successors.siblingNext != currentTrace->successors.siblingPrev)
							{
								auto predecessor = GetTrace(currentTrace->predecessors.first);
								if (nestedTas->branchTraces.Keys().Contains(predecessor))
								{
									nestedTas->branchSelections.Add(nestedTa, currentTrace);
								}
							}

							auto currentTraceExec = GetTraceExec(currentTrace->traceExecRef);
							if (currentTraceExec->branchData.forwardTrace == currentTrace)
							{
								currentTraceRef = currentTrace->predecessors.first;
							}
							else
							{
								currentTraceRef = currentTraceExec->branchData.forwardTrace;
							}
						}
					}
				}

				return nestedTas;
			}

/***********************************************************************
BuildStepLeafsForAmbiguityBranch
***********************************************************************/

			void TraceManager::BuildStepLeafsForAmbiguityBranch(
				TraceAmbiguity* ta,
				ExecutionStep* lastSharedStep,
				Trace* ambiguityBranchStartTrace,
				vint32_t* ambiguityBranchStartIns,
				ExecutionStepTree& ambiguityStepTree)
			{
				auto taFirst = GetTrace(ta->firstTrace);
				auto taLast = GetTrace(ta->lastTrace);
				auto taMerge = GetTrace(ta->mergeTrace);
				auto taFirstExec = GetTraceExec(taFirst->traceExecRef);
				auto taLastExec = GetTraceExec(taLast->traceExecRef);
				auto taMergeExec = GetTraceExec(taMerge->traceExecRef);
				vint32_t prefixExtra = ta->prefix - taFirstExec->insLists.countAll;
				vint32_t postfixExtra = ta->postfix - taLastExec->insLists.countAll;

				ExecutionStepLinkedList branchList;
				// Execute the branch
				{
					Trace* rawBranchTrace = nullptr;
					auto steps = BuildStepListUntilFirstRawBranchTrace(
						ambiguityBranchStartTrace,
						ambiguityBranchStartIns ? *ambiguityBranchStartIns : (prefixExtra <= 0 ? 0 : prefixExtra),
						taMerge,
						(taMergeExec->insLists.countAll - 1 - (postfixExtra <= 0 ? 0 : postfixExtra)),
						nullptr,
						&rawBranchTrace
					);

					if (rawBranchTrace)
					{
						if (steps.first)
						{
							steps.first->parent = lastSharedStep;
						}

						auto successorId = rawBranchTrace->successors.first;
						while (successorId != nullref)
						{
							auto successor = GetTrace(successorId);
							successorId = successor->successors.siblingNext;
							BuildStepLeafsForAmbiguityBranch(ta, (steps.first ? steps.last : lastSharedStep), successor, nullptr, ambiguityStepTree);
						}
						return;
					}
					AppendStepsAfterList(steps, branchList);
				}

				// Execute from taMerge to taLast
				if (postfixExtra < 0)
				{
					auto steps = BuildStepList(
						taMerge,
						0,
						taLast,
						-postfixExtra - 1,
						nullptr
					);
					AppendStepsAfterList(steps, branchList);
				}

				// Append RA_BRANCH
				{
					auto step = GetExecutionStep(executionSteps.Allocate());
					step->type = ExecutionType::RA_Branch;
					step->et_ra.trace = taLast->allocatedIndex;
					step->et_ra.type = -1;
					AppendStepsAfterList({ step,step }, branchList);
				}

				branchList.first->parent = lastSharedStep;
				AppendLeafToTree(branchList.last, ambiguityStepTree);
			}

/***********************************************************************
BuildStepLeafsForNestedAmbiguityBranch
***********************************************************************/

			void TraceManager::BuildStepLeafsForNestedAmbiguityBranch(
				TraceAmbiguity* ta,
				ExecutionStep* lastSharedStep,
				BSLA_Guidance* guidance,
				ExecutionStepTree& ambiguityStepTree)
			{
				ExecutionStepLinkedList stepsBeforeBranch;
				auto nta = guidance->nestedTas->nestedAmbiguities[guidance->nextAmbiguityIndex];

				auto taFirst = GetTrace(ta->firstTrace);
				auto ntaFirst = GetTrace(nta->firstTrace);

				// Execute from taFirst until the nested TraceAmbiguity
				{
					auto ntaFirst = GetTrace(nta->firstTrace);
					auto steps = BuildStepList(
						taFirst,
						ta->prefix,
						ntaFirst,
						-1,
						nullptr
					);
					AppendStepsAfterList(steps, stepsBeforeBranch);
				}

				// Execute the nested TraceAmbiguity
				Trace* currentTrace = ntaFirst;
				vint32_t currentIns = 0;
				{
					auto steps = BuildStepListThroughAmbiguity(
						currentTrace,
						currentIns,
						nta,
						guidance
					);
					AppendStepsAfterList(steps, stepsBeforeBranch);
				}


				// Execute the rest
				stepsBeforeBranch.first->parent = lastSharedStep;
				lastSharedStep = stepsBeforeBranch.last;
				BuildStepLeafsForAmbiguityBranch(
					ta,
					lastSharedStep,
					currentTrace,
					&currentIns,
					ambiguityStepTree
				);
			}

/***********************************************************************
BuildStepListForAmbiguity
***********************************************************************/

			ExecutionStepLinkedList TraceManager::BuildStepListForAmbiguity(
				TraceAmbiguity* ta,
				BSLA_Guidance* guidance)
			{
				BSLA_Guidance DoNotUse_BSLA_Guidance;
				BSL_Guidance DoNotUse_BSL_Guidance;

				ExecutionStepLinkedList result;
				auto taFirst = GetTrace(ta->firstTrace);
				auto taBranch = GetTrace(ta->branchTrace);
				auto taFirstExec = GetTraceExec(taFirst->traceExecRef);
				auto taBranchExec = GetTraceExec(taBranch->traceExecRef);
				vint32_t prefixExtra = ta->prefix - taFirstExec->insLists.countAll;

				// Find the first nested TraceAmbiguity between taFirst and taBranch
				if (!guidance && (DoNotUse_BSLA_Guidance.nestedTas = CollectNestedAmbiguities(ta)))
				{
					guidance = &DoNotUse_BSLA_Guidance;
				}

				// Append RA_BEGIN
				{
					auto step = GetExecutionStep(executionSteps.Allocate());
					step->type = ExecutionType::RA_Begin;
					step->et_ra.trace = taFirst->allocatedIndex;
					step->et_ra.type = -1;
					AppendStepsAfterList({ step,step }, result);
				}

				{
					ExecutionStepTree branchSteps;
					// If there is a nested TraceAmbiguity, Execute it first
					if (guidance)
					{
						guidance->nextAmbiguityIndex++;
						BuildStepLeafsForNestedAmbiguityBranch(ta, nullptr, guidance, branchSteps);
						guidance->nextAmbiguityIndex--;
					}

					ExecutionStepLinkedList sharedSteps;
					// Execute from taFirst to taBranch
					if (prefixExtra < 0)
					{
						if (guidance)
						{
							auto nta = guidance->nestedTas->nestedAmbiguities[guidance->nextAmbiguityIndex];
							DoNotUse_BSL_Guidance = {
								&guidance->nestedTas->branchSelections[nta],
								&guidance->nestedTas->nestedAmbiguities,
								guidance->nextAmbiguityIndex
							};
						}
						sharedSteps = BuildStepList(
							taFirst,
							ta->prefix,
							taBranch,
							taBranchExec->insLists.countAll - 1,
							(guidance ? &DoNotUse_BSL_Guidance : nullptr)
						);
					}

					// Nested TraceAmbiguity is not implemented, so all successors of taBranch are needed
					auto successorId = taBranch->successors.first;
					while (successorId != nullref)
					{
						auto successor = GetTrace(successorId);
						successorId = successor->successors.siblingNext;

						if (guidance)
						{
							// Skip visited branches
							CHECK_FAIL(L"Not Implemented!");
						}
						BuildStepLeafsForAmbiguityBranch(ta, sharedSteps.last, successor, nullptr, branchSteps);
					}

					AppendStepsAfterList(ConvertStepTreeToList(branchSteps), result);
				}

				// Append RA_END
				{
					auto step = CreateResolveAmbiguityStep(ta);
					AppendStepsAfterList({ step,step }, result);
				}

				return result;
			}

/***********************************************************************
BuildStepListThroughAmbiguity
***********************************************************************/

			ExecutionStepLinkedList TraceManager::BuildStepListThroughAmbiguity(
				Trace*& currentTrace,
				vint32_t& currentIns,
				TraceAmbiguity* ta,
				BSLA_Guidance* guidance
			)
			{
				ExecutionStepLinkedList result;

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
				auto taSteps = BuildStepListForAmbiguity(ta, guidance);
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

				return result;
			}

/***********************************************************************
BuildStepList
***********************************************************************/

			ExecutionStepLinkedList TraceManager::BuildStepListUntilFirstRawBranchTrace(
				Trace* startTrace,
				vint32_t startIns,
				Trace* endTrace,
				vint32_t endIns,
				BSL_Guidance* guidance,
				Trace** rawBranchTrace)
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
								// When there is no more next critical trace before a branch trace
								auto criticalTraceExec = GetTraceExec(criticalTrace->traceExecRef);
								if (criticalTraceExec->ambiguityBegins != nullref)
								{
									// If it is associated with a TraceAmbiguity, it is a critical trace we are looking for
									break;
								}
								throw TraceException(*this, currentTrace, nullptr, TRACE_MAMAGER_PHRASE, L"Failed to find a TraceAmbiguity between this trace and the next branch trace.");
							}
							else
							{
								// When there is no more next critical trace, we are about to reach the end
								// Ignore the current critical trace, stop searching, just run through the end
								goto NO_CRITICAL_TRACE;
							}
						}

						// When it runs past (endTrace, endIns)
						// Ignore the current critical trace, stop searching, just run through the end
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

					// If the current critical trace is associated with a TraceAmbiguity
					// and the TraceAmbiguity is what configured to skip
					// treat it as an ordinary trace
					auto criticalTraceExec = GetTraceExec(criticalTrace->traceExecRef);
					bool ignoreCriticalAmgiguity = false;
					if (guidance && criticalTraceExec->ambiguityBegins != nullref)
					{
						auto ta = GetTraceAmbiguity(GetTraceAmbiguityLink(criticalTraceExec->ambiguityBegins)->ambiguity);
						for (vint i = guidance->ambiguitiesToSkipStart; i < guidance->ambiguitiesToSkip->Count(); i++)
						{
							if (guidance->ambiguitiesToSkip->Get(i) == ta)
							{
								ignoreCriticalAmgiguity = true;
								break;
							}
						}
					}

					if (ignoreCriticalAmgiguity || criticalTraceExec->ambiguityBegins == nullref)
					{
						// If the current critical trace is a branch trace
						// and there is a specified successor to execute
						// treat it as an ordinary trace
						// otherwise, exit properly
						Trace* specifieidBranchSelection = nullptr;
						if (criticalTrace->successors.first != criticalTrace->successors.last)
						{
							if (guidance)
							{
								for (auto selection : *guidance->branchSelections)
								{
									if (criticalTrace == selection->predecessors.first)
									{
										specifieidBranchSelection = selection;
										break;
									}
								}
							}

							if (!specifieidBranchSelection)
							{
								if (rawBranchTrace)
								{
									*rawBranchTrace = criticalTrace;
									endTrace = criticalTrace;
									endIns = GetTraceExec(endTrace->traceExecRef)->insLists.countAll - 1;
									goto NO_CRITICAL_TRACE;
								}
								else
								{
									throw TraceException(*this, currentTrace, criticalTrace, TRACE_MAMAGER_PHRASE, L"The next critical trace after the current trace is not associated with a TraceAmbiguity.");
								}
							}
						}

						// A critical trace could be a predecessor of a merge trace
						// Execute until here and continue
						auto step = GetExecutionStep(executionSteps.Allocate());
						step->et_i.startTrace = currentTrace->allocatedIndex;
						step->et_i.startIns = currentIns;
						step->et_i.endTrace = criticalTrace->allocatedIndex;
						step->et_i.endIns = criticalTraceExec->insLists.countAll - 1;
						AppendStepsAfterList({ step, step }, result);

						if(specifieidBranchSelection)
						{
							currentTrace = specifieidBranchSelection;
						}
						else if (criticalTrace->successors.first == nullref)
						{
							currentTrace = nullptr;
						}
						else
						{
							currentTrace = GetTrace(criticalTrace->successors.first);
							currentIns = 0;
						}
						continue;
					}

					// Execute from (currentTrace, currentIns) until the next TraceAmbiguity
					auto ta = GetTraceAmbiguity(GetTraceAmbiguityLink(criticalTraceExec->ambiguityBegins)->ambiguity);
					auto steps = BuildStepListThroughAmbiguity(currentTrace, currentIns, ta, nullptr);
					AppendStepsAfterList(steps, result);
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

			ExecutionStepLinkedList TraceManager::BuildStepList(
				Trace* startTrace,
				vint32_t startIns,
				Trace* endTrace,
				vint32_t endIns,
				BSL_Guidance* guidance)
			{
				return BuildStepListUntilFirstRawBranchTrace(startTrace, startIns, endTrace, endIns, guidance, nullptr);
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

				auto steps = BuildStepList(startTrace, startIns, endTrace, endIns, nullptr);
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