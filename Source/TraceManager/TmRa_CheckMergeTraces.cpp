#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;

#define NEW_MERGE_STACK_MAGIC_COUNTER (void)(MergeStack_MagicCounter++)

/***********************************************************************
CheckMergeTrace
***********************************************************************/

			template<typename TCallback>
			bool TraceManager::EnumerateObjects(Ref<InsExec_StackRefLink> stackRefLinkStartSet, bool withCounter, TCallback&& callback)
			{
				// check every object in the link
				auto magicIterating = MergeStack_MagicCounter;
				auto linkId = stackRefLinkStartSet;
				while (linkId != nullref)
				{
					auto stackRefLink = GetInsExec_StackRefLink(linkId);
					linkId = stackRefLink->previous;
					auto ieObject = GetInsExec_Stack(stackRefLink->id);

					if (withCounter)
					{
						// skip if it has been searched
						if (ieObject->mergeCounter == magicIterating) goto CHECK_NEXT_OBJECT;
						ieObject->mergeCounter = magicIterating;
					}

					if (!callback(ieObject)) return false;
				CHECK_NEXT_OBJECT:;
				}
				return true;
			}

			template<typename TCallback>
			bool TraceManager::EnumerateBottomInstructions(InsExec_Stack* ieObject, TCallback&& callback)
			{
				auto insRefLinkId = ieObject->endWithCreateInsRefs;
				while (insRefLinkId != nullref)
				{
					auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
					insRefLinkId = insRefLink->previous;
					if (!callback(GetTrace(insRefLink->insRef.trace), insRefLink->insRef.ins)) return false;
				}

				insRefLinkId = ieObject->endWithReuseInsRefs;
				while (insRefLinkId != nullref)
				{
					auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
					insRefLinkId = insRefLink->previous;
					if (!callback(GetTrace(insRefLink->insRef.trace), insRefLink->insRef.ins)) return false;
				}

				return true;
			}

			bool TraceManager::ComparePrefix(TraceExec* baselineTraceExec, TraceExec* commingTraceExec, vint32_t prefix)
			{
				if (commingTraceExec->insLists.countAll < prefix) return false;
				for (vint32_t i = 0; i < prefix; i++)
				{
					auto&& insBaseline = ReadInstruction(i, baselineTraceExec->insLists);
					auto&& insComming = ReadInstruction(i, baselineTraceExec->insLists);
					if (insBaseline != insComming) return false;
				}

				return true;
			}

			bool TraceManager::ComparePostfix(TraceExec* baselineTraceExec, TraceExec* commingTraceExec, vint32_t postfix)
			{
				if (commingTraceExec->insLists.countAll < postfix) return false;
				for (vint32_t i = 0; i < postfix; i++)
				{
					auto&& insBaseline = ReadInstruction(baselineTraceExec->insLists.countAll - i - 1, baselineTraceExec->insLists);
					auto&& insComming = ReadInstruction(baselineTraceExec->insLists.countAll - i - 1, baselineTraceExec->insLists);
					if (insBaseline != insComming) return false;
				}

				return true;
			}

			template<typename TCallback>
			bool TraceManager::CheckAmbiguityResolution(TraceAmbiguity* ta, collections::List<Ref<InsExec_StackRefLink>>& visitingIds, TCallback&& callback)
			{
				// following conditions need to be satisfies if multiple objects could be the result of ambiguity
				//
				// StackBegin that create objects must be
				//   the same instruction in the same trace
				//   in different trace
				//     these traces share the same predecessor
				//     prefix in these traces are the same
				//
				// StackEnd that end objects must be
				//   the same instruction in the same trace
				//   in different trace
				//     these traces share the same successor
				//     postfix in these traces are the same

				// initialize TraceAmbiguity
				Trace* first = nullptr;
				Trace* last = nullptr;
				TraceExec* firstTraceExec = nullptr;
				TraceExec* lastTraceExec = nullptr;
				bool foundBeginSame = false;
				bool foundBeginPrefix = false;
				bool foundEndSame = false;
				bool foundEndPostfix = false;
				bool succeeded = false;

				// iterate all top objects
				succeeded = callback([&](Ref<InsExec_StackRefLink> objRefLink)
				{
					return EnumerateObjects(objRefLink, false, [&](InsExec_Stack* ieObject)
					{
						auto createTrace = GetTrace(ieObject->summarizing.earliestInsRef.trace);
						if (!first)
						{
							first = createTrace;
							firstTraceExec = GetTraceExec(first->traceExecRef);
							ta->firstTrace = createTrace;
							ta->prefix = ieObject->summarizing.earliestInsRef.ins;
						}
						else if (first == createTrace)
						{
							// check if two instruction is the same
							if (ta->prefix != ieObject->summarizing.earliestInsRef.ins) return false;
							foundBeginSame = true;
						}
						else
						{
							// check if two instruction shares the same prefix
							if (first->predecessors.first != createTrace->predecessors.first) return false;
							auto createTraceExec = GetTraceExec(createTrace->traceExecRef);
							if (!ComparePrefix(firstTraceExec, createTraceExec, ta->prefix)) return false;
							foundBeginPrefix = true;
						}

						return true;
					});
				});
				if (!succeeded) return false;

				// iterate all bottom instructions
				{
					// bottomInsRefs need to be filtered again
					// because the object from the first branch could be a field in the object from the second branch
					// in this case, that object could have multiple incompatible bottomInsRefs
					// so we try eoTrace and the unique and existing eoTrace->successors.first
					// see which wins
					Group<Trace*, InsRef> postfixesAtSelf, postfixesAtSuccessor;

					NEW_MERGE_STACK_MAGIC_COUNTER;
					callback([&](Ref<InsExec_StackRefLink> objRefLink)
					{
						return EnumerateObjects(objRefLink, true, [&](InsExec_Stack* ieObject)
						{
							PushStackRefLink(ta->bottomCreateObjectStacks, ieObject);

							// check if EO satisfies the condition
							return EnumerateBottomInstructions(ieObject, [&](Trace* eoTrace, vint32_t eoIns)
							{
								auto eoTraceExec = GetTraceExec(eoTrace->traceExecRef);
								InsRef insRef{ eoTrace,eoTraceExec->insLists.countAll - eoIns - 1 };
								postfixesAtSelf.Add(eoTrace, insRef);

								Trace* successorTrace = nullptr;
								if (eoTrace->successorCount == 1)
								{
									successorTrace = GetTrace(eoTrace->successors.first);
								}
								postfixesAtSuccessor.Add(successorTrace, insRef);
								return true;
							});
						});
					});

					// find the most possible answer from postfixesAtSelf and postfixesAtSuccessor
					// bottom bottomInsRefs are splitted into multiple group
					// find the unique one that has the maximum capacity
					vint maxOccurences = -1;
					for (vint i = 0; i < postfixesAtSelf.Count(); i++)
					{
						vint count = postfixesAtSelf.GetByIndex(i).Count();
						if (count > maxOccurences)
						{
							maxOccurences = count;
						}
					}
					for (vint i = 0; i < postfixesAtSuccessor.Count(); i++)
					{
						vint count = postfixesAtSuccessor.GetByIndex(i).Count();
						if (count > maxOccurences)
						{
							maxOccurences = count;
						}
					}

					vint uniqueAtSelf = -1;
					for (vint i = 0; i < postfixesAtSelf.Count(); i++)
					{
						vint count = postfixesAtSelf.GetByIndex(i).Count();
						if (count == maxOccurences)
						{
							if (uniqueAtSelf == -1)
							{
								uniqueAtSelf = i;
							}
							else
							{
								uniqueAtSelf = -2;
								break;
							}
						}
					}

					vint uniqueAtSuccessor = -1;
					for (vint i = 0; i < postfixesAtSuccessor.Count(); i++)
					{
						vint count = postfixesAtSuccessor.GetByIndex(i).Count();
						if (count == maxOccurences)
						{
							if (uniqueAtSuccessor == -1)
							{
								uniqueAtSuccessor = i;
							}
							else
							{
								uniqueAtSuccessor = -2;
								break;
							}
						}
					}

					InsRef lastPostfix;
					if (uniqueAtSelf >= 0)
					{
						// if all bottom traces are the same, their first successors are also the same
						lastPostfix = postfixesAtSelf.GetByIndex(uniqueAtSelf)[0];
					}
					else if (uniqueAtSuccessor >= 0)
					{
						lastPostfix = postfixesAtSuccessor.GetByIndex(uniqueAtSuccessor)[0];
						foundEndPostfix = true;
					}

					if (lastPostfix.trace == nullref)
					{
						succeeded = false;
					}
					else
					{
						// TODO: check if last is in the same thread and is or after the merge trace
						last = GetTrace(lastPostfix.trace);
						ta->lastTrace = last;
						ta->postfix = lastPostfix.ins;
						succeeded = true;
					}
				}
				if (!succeeded) return false;

				// ensure the statistics result is compatible
				if (first && !foundBeginSame && !foundBeginPrefix) foundBeginSame = true;
				if (last && !foundEndSame && !foundEndPostfix) foundEndSame = true;
				if (foundBeginSame == foundBeginPrefix) return false;
				if (foundEndSame == foundEndPostfix) return false;

				// fix prefix if necessary
				if (foundBeginPrefix)
				{
					auto first = GetTrace(GetTrace(ta->firstTrace)->predecessors.first);
					auto traceExec = GetTraceExec(first->traceExecRef);
					ta->firstTrace = first;
					ta->prefix += traceExec->insLists.countAll;
				}

				// fix postfix if necessary
				if (foundEndPostfix)
				{
					// last will be a merge trace
					// so ta->postfix doesn't need to change
					auto last = GetTrace(GetTrace(ta->lastTrace)->successors.first);
					auto traceExec = GetTraceExec(last->traceExecRef);
					ta->lastTrace = last;
				}

				// ensure firstTrace and lastTrace are in the same branch
				auto firstForward = GetTrace(GetTraceExec(GetTrace(ta->firstTrace)->traceExecRef)->branchData.forwardTrace);
				auto lastForward = GetTrace(GetTraceExec(GetTrace(ta->lastTrace)->traceExecRef)->branchData.forwardTrace);
				auto currentForward = lastForward;
				while (true)
				{
					if (currentForward->traceExecRef < firstForward->traceExecRef)
					{
						return false;
					}
					if (currentForward == firstForward)
					{
						return true;
					}

					auto currentExec = GetTraceExec(currentForward->traceExecRef);
					auto nextForwardRef = currentExec->branchData.commonForwardBranch;
					if (nextForwardRef == nullptr)
					{
						nextForwardRef = currentExec->branchData.forwardTrace;
					}

					auto nextForward = GetTrace(currentExec->branchData.forwardTrace);
					if (currentForward != nextForward)
					{
						currentForward = nextForward;
					}
					else if (currentForward->predecessorCount > 0)
					{
						currentForward = GetTrace(GetTraceExec(GetTrace(currentForward->predecessors.first)->traceExecRef)->branchData.forwardTrace);
					}
					else
					{
						break;
					}
				}
				return false;
			}

			bool TraceManager::CheckMergeTrace(TraceAmbiguity* ta, Trace* trace, TraceExec* traceExec, collections::List<Ref<InsExec_StackRefLink>>& visitingIds)
			{
				// when a merge trace is the surviving trace
				// objects in the top object stack are the result of ambiguity
				if (trace->successorCount == 0)
				{
					auto ieOSTop = GetInsExec_StackArrayRefLink(traceExec->context.objectStack);
					return CheckAmbiguityResolution(ta, visitingIds, [=](auto&& callback)
					{
						return callback(ieOSTop->ids);
					});
				}

				// otherwise
				// objects in the top create stack are the result of ambiguity
				// even when there is only one object in the stack

				// but in some cases
				// objects in the top object stack are the result of ambiguity
				// when these objects are the only difference in branches
				// here we need to test if the condition satisfied

				{
					// [CONDITION]
					// the first predecessor must has a StackEnd instruction
					// count the number of instructions after StackEnd
					// these instructions are the postfix
					vint32_t postfix = -1;
					auto firstTrace = GetTrace(trace->predecessors.first);
					auto firstTraceExec = GetTraceExec(firstTrace->traceExecRef);
					for (vint32_t i = firstTraceExec->insLists.countAll - 1; i >= 0; i--)
					{
						auto&& ins = ReadInstruction(i, firstTraceExec->insLists);
						if (ins.type == AstInsType::StackEnd)
						{
							postfix = firstTraceExec->insLists.countAll - i - 1;
							break;
						}
					}
					if (postfix == -1)
					{
						goto CHECK_OBJECTS_IN_TOP_CREATE_STACK;
					}

					// [CONDITION]
					// all predecessor must have a StackEnd instruction
					// posftix of all predecessors must be the same
					{
						auto predecessorId = trace->predecessors.last;
						while (predecessorId != firstTrace)
						{
							auto predecessor = GetTrace(predecessorId);
							predecessorId = predecessor->predecessors.siblingPrev;
							if (!ComparePostfix(firstTraceExec, GetTraceExec(predecessor->traceExecRef), postfix + 1))
							{
								goto CHECK_OBJECTS_IN_TOP_CREATE_STACK;
							}
						}
					}

					// check if all StackEnd ended objects are the result of ambiguity
					if (postfix == 0)
					{
						// if StackEnd is the last instruction of predecessors
						// then their objRefs has been written to the top object stack
						auto ieOSTop = GetInsExec_StackArrayRefLink(traceExec->context.objectStack);
						auto succeeded = CheckAmbiguityResolution(ta, visitingIds, [=](auto&& callback)
						{
							return callback(ieOSTop->ids);
						});
						if (succeeded) return true;
					}
					else
					{
						// otherwise find all objRefs of StackEnd
						auto succeeded = CheckAmbiguityResolution(ta, visitingIds, [=, this, &visitingIds](auto&& callback)
						{
							auto predecessorId = trace->predecessors.first;
							while (predecessorId != nullref)
							{
								auto predecessor = GetTrace(predecessorId);
								predecessorId = predecessor->predecessors.siblingNext;

								// search for the object it ends
								auto predecessorTraceExec = GetTraceExec(predecessor->traceExecRef);
								auto indexEO = predecessorTraceExec->insLists.countAll - postfix - 1;
								auto insExecEO = GetInsExec(predecessorTraceExec->insExecRefs.start + indexEO);
								if (!callback(insExecEO->operatingStacks)) return false;
							}
							return true;
						});
						if (succeeded) return true;
					}
				}
			CHECK_OBJECTS_IN_TOP_CREATE_STACK:
				auto ieCSTop = GetInsExec_StackArrayRefLink(traceExec->context.createStack);
				return CheckAmbiguityResolution(ta, visitingIds, [=](auto&& callback)
				{
					return callback(ieCSTop->ids);
				});
			}

/***********************************************************************
LinkAmbiguityCriticalTrace
***********************************************************************/

			void TraceManager::LinkAmbiguityCriticalTrace(Ref<Trace> traceId)
			{
				auto trace = GetTrace(traceId);
				auto forward = GetTrace(GetTraceExec(trace->traceExecRef)->branchData.forwardTrace);
				if (trace == forward) return;

				auto nextAct = &GetTraceExec(forward->traceExecRef)->nextAmbiguityCriticalTrace;
				while (*nextAct != nullref)
				{
					if (*nextAct == traceId) return;
					if (*nextAct > traceId) break;
					nextAct = &GetTraceExec(GetTrace(*nextAct)->traceExecRef)->nextAmbiguityCriticalTrace;
				}

				auto traceExec = GetTraceExec(trace->traceExecRef);
				traceExec->nextAmbiguityCriticalTrace = *nextAct;
				*nextAct = traceId;
			}

/***********************************************************************
CheckTraceAmbiguity
***********************************************************************/

			void TraceManager::CheckTraceAmbiguity(TraceAmbiguity* ta)
			{
#define TRACE_MAMAGER_PHRASE L"ResolveAmbiguity/CheckMergeTraces/CheckTraceAmbiguity"
				auto teFirst = GetTraceExec(GetTrace(ta->firstTrace)->traceExecRef);

				if (teFirst->ambiguityBegins == nullref)
				{
					LinkAmbiguityCriticalTrace(ta->firstTrace);
				}

				// search in all ambiguityBegins and try to find one has the same lastTrace
				TraceAmbiguityLink* taLinkToOverride = nullptr;
				auto taLinkRef = teFirst->ambiguityBegins;
				while (taLinkRef != nullref)
				{
					auto taLink = GetTraceAmbiguityLink(taLinkRef);
					taLinkRef = taLink->previous;

					auto ta2 = GetTraceAmbiguity(taLink->ambiguity);
					if (ta->lastTrace == ta2->lastTrace)
					{
						// if there is any, try to override this TraceAmbiguity
						taLinkToOverride = taLink;
						break;
					}
				}

				if (taLinkToOverride)
				{
					// if there is a TraceAmbiguity to override
					// ensure they are equivalent
					auto ta2 = GetTraceAmbiguity(taLinkToOverride->ambiguity);
					if (ta2->prefix != ta->prefix || ta2->postfix != ta->postfix)
					{
						throw TraceManager(*this, TRACE_MAMAGER_PHRASE, L"Incompatible TraceAmbiguity has been assigned at the same place");
					}
					// override ambiguityBegins
					taLinkToOverride->ambiguity = ta;

					// override TraceAmbiguity
					ta->overridedAmbiguity = ta2;
				}
				else
				{
					// otherwise, append itself to the list
					auto taLink = GetTraceAmbiguityLink(traceAmbiguityLinks.Allocate());
					taLink->ambiguity = ta;
					taLink->previous = teFirst->ambiguityBegins;
					teFirst->ambiguityBegins = taLink;
				}
#undef TRACE_MAMAGER_PHRASE
			}

/***********************************************************************
CategorizeTraceAmbiguities
***********************************************************************/

			void TraceManager::MarkAmbiguityCoveredForward(Trace* currentTrace, TraceAmbiguity* ta, Trace* firstTrace, TraceExec* firstTraceExec)
			{
#define TRACE_MAMAGER_PHRASE L"ResolveAmbiguity/CheckMergeTraces/MarkAmbiguityCoveredForward"
				while (true)
				{
					auto forward = GetTrace(GetTraceExec(currentTrace->traceExecRef)->branchData.forwardTrace);
					if (forward->traceExecRef <= firstTraceExec)
					{
						throw TraceException(*this, forward, nullptr, TRACE_MAMAGER_PHRASE, L"Unexpected ambiguity resolving structure found.");
					}

					auto forwardExec = GetTraceExec(forward->traceExecRef);
					if (forward->predecessors.first != forward->predecessors.last)
					{
						if (forwardExec->ambiguityDetected != nullref && forwardExec->ambiguityDetected != ta)
						{
							currentTrace = GetTrace(GetTraceAmbiguity(forwardExec->ambiguityDetected)->firstTrace);
						}
						else
						{
							auto predecessorId = forward->predecessors.first;
							while (predecessorId != nullref)
							{
								auto predecessor = GetTrace(predecessorId);
								predecessorId = predecessor->predecessors.siblingNext;
								MarkAmbiguityCoveredForward(predecessor, ta, firstTrace, firstTraceExec);
							}
							return;
						}
					}
					else if (forward->predecessors.first == firstTrace)
					{
						auto forwardExec = GetTraceExec(forward->traceExecRef);
						CHECK_ERROR(forwardExec->ambiguityCoveredInForward == nullref || forwardExec->ambiguityCoveredInForward == ta, L"Unexpected ambiguity resolving structure found.");
						forwardExec->ambiguityCoveredInForward = ta;
						return;
					}
					else
					{
						currentTrace = GetTrace(forward->predecessors.first);
					}
				}
#undef TRACE_MAMAGER_PHRASE
			}

			void TraceManager::CategorizeTraceAmbiguities(Trace* trace, TraceExec* traceExec)
			{
				// find all ambiguityBegins whose first ambiguity instruction is in successors
				auto taLinkRef = traceExec->ambiguityBegins;
				while (taLinkRef != nullref)
				{
					auto taLink = GetTraceAmbiguityLink(taLinkRef);
					taLinkRef = taLink->previous;

					auto ta = GetTraceAmbiguity(taLink->ambiguity);
					if (ta->prefix >= traceExec->insLists.countAll)
					{
						// mark ambiguityCoveredInForward
						MarkAmbiguityCoveredForward(GetTrace(ta->lastTrace), ta, trace, traceExec);
					}
				}
			}

/***********************************************************************
CheckMergeTraces
***********************************************************************/

			void TraceManager::CheckMergeTraces()
			{
#define TRACE_MAMAGER_PHRASE L"ResolveAmbiguity/CheckMergeTraces"
				// mark all branch trace critical
				{
					auto traceId = firstBranchTrace;
					while (traceId != nullref)
					{
						LinkAmbiguityCriticalTrace(traceId);
						traceId = GetTraceExec(GetTrace(traceId)->traceExecRef)->nextBranchTrace;
					}
				}

				// mark all predecessor of merge trace critical
				{
					auto traceId = firstMergeTrace;
					while (traceId != nullref)
					{
						auto trace = GetTrace(traceId);
						auto predecessorId = trace->predecessors.first;
						while (predecessorId != nullref)
						{
							LinkAmbiguityCriticalTrace(predecessorId);
							predecessorId = GetTrace(predecessorId)->predecessors.siblingNext;
						}
						traceId = GetTraceExec(trace->traceExecRef)->nextMergeTrace;
					}
				}

				// iterating TraceMergeExec
				List<Ref<InsExec_StackRefLink>> visitingIds;
				auto traceId = firstMergeTrace;
				while (traceId != nullref)
				{
					auto trace = GetTrace(traceId);
					auto traceExec = GetTraceExec(trace->traceExecRef);
					traceId = traceExec->nextMergeTrace;

					auto ta = GetTraceAmbiguity(traceAmbiguities.Allocate());
					bool succeeded = CheckMergeTrace(ta, trace, traceExec, visitingIds);
					if (!succeeded)
					{
						throw TraceException(*this, trace, nullptr, TRACE_MAMAGER_PHRASE, L"Failed to find ambiguous objects in a merge trace.");
					}
					traceExec->ambiguityDetected = ta;

					// check if existing TraceAmbiguity in firstTrace are compatible
					CheckTraceAmbiguity(ta);
				}

				// find all branch trace with ambiguityBegins
				{
					auto traceId = firstBranchTrace;
					while (traceId != nullref)
					{
						auto trace = GetTrace(traceId);
						auto traceExec = GetTraceExec(trace->traceExecRef);
						traceId = traceExec->nextBranchTrace;

						if (traceExec->ambiguityBegins != nullref)
						{
							CategorizeTraceAmbiguities(trace, traceExec);
						}
					}
				}
#undef TRACE_MAMAGER_PHRASE
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}