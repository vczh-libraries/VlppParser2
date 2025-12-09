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
CheckAmbiguityResolution
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
				auto insRefLinkId = ieObject->summarizing.bottomInsRefs;
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
			bool TraceManager::CheckAmbiguityResolution(TraceAmbiguity* ta, collections::List<Ref<InsExec_StackRefLink>>& visitingIds, collections::List<WString>* failureReasons, TCallback&& callback)
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

				vint firstTraceCount = 0;
				vint firstSameTraceCount = 0;
				vint firstSamePredecessorCount = 0;

				bool foundEndSame = false;
				bool foundEndPostfix = false;
				bool succeeded = false;

				// iterate all top objects
				if (failureReasons)
				{
					failureReasons->Add(L"[InsExec_Stack->summarizing.earliestInsRef]");
				}
				succeeded = callback([&](Ref<InsExec_StackRefLink> objRefLink)
				{
					return EnumerateObjects(objRefLink, false, [&](InsExec_Stack* ieObject)
					{
						auto createTrace = GetTrace(ieObject->summarizing.earliestInsRef.trace);
						if (failureReasons)
						{
							failureReasons->Add(L"  Verifying object " +
								itow(ieObject->allocatedIndex) +
								L", its earliestInsRef is " +
								itow(ieObject->summarizing.earliestInsRef.trace.handle) + L"@" + itow(ieObject->summarizing.earliestInsRef.ins) +
								L".");
						}
						if (!first)
						{
							first = createTrace;
							firstTraceExec = GetTraceExec(first->traceExecRef);
							ta->firstTrace = createTrace;
							ta->prefix = ieObject->summarizing.earliestInsRef.ins;
							if (failureReasons)
							{
								failureReasons->Add(L"  This is the first object in the list.");
							}
						}
						else
						{
							firstTraceCount++;

							if (first == createTrace)
							{
								// check if two instruction is the same
								if (ta->prefix != ieObject->summarizing.earliestInsRef.ins)
								{
									if (failureReasons)
									{
										failureReasons->Add(L"  It has a different prefix, stopped.");
									}
									return false;
								}
								firstSameTraceCount++;
							}

							if (first->predecessors.first == createTrace->predecessors.first)
							{
								// check if two instruction shares the same prefix
								if (first->predecessors.first != createTrace->predecessors.first)
								{
								}
								auto createTraceExec = GetTraceExec(createTrace->traceExecRef);
								if (!ComparePrefix(firstTraceExec, createTraceExec, ta->prefix))
								{
									if (failureReasons)
									{
										failureReasons->Add(L"  They has a different postfix, stopped");
									}
									return false;
								}
								firstSamePredecessorCount++;
							}

							if (first != createTrace && first->predecessors.first != createTrace->predecessors.first)
							{
								if (failureReasons)
								{
									failureReasons->Add(L"  The predecessor of the trace where the earliestInsRef of the first object is " +
										itow(first->predecessors.first.handle) +
										L", meanwhile the one for the current object is " +
										itow(createTrace->predecessors.first.handle) +
										L", they are different, stopped.");
								}
								return false;
							}
						}

						return true;
					});
				});
				if (!succeeded) return false;

				// iterate all bottom instructions
				{
					if (failureReasons)
					{
						failureReasons->Add(L"[InsExec_Stack->endWithCreateInsRefs/endWithReuseInsRefs]");
					}
					// endWith(Create|Reuse)InsRefs need to be filtered again
					// because the object from the first branch could be a field in the object from the second branch
					// in this case, that object could have multiple incompatible endWith(Create|Reuse)InsRefs
					// so we try eoTrace and the unique and existing eoTrace->successors.first
					// see which wins
					Group<Trace*, InsRef> postfixesAtSelf, postfixesAtSuccessor;

					NEW_MERGE_STACK_MAGIC_COUNTER;
					callback([&](Ref<InsExec_StackRefLink> objRefLink)
					{
						return EnumerateObjects(objRefLink, true, [&](InsExec_Stack* ieObject)
						{
							if (failureReasons)
							{
								failureReasons->Add(L"  Verifying object " +
									itow(ieObject->allocatedIndex) +
									L" which has StackEnd instructions:");
							}
							PushStackRefLink(ta->bottomCreateObjectStacks, ieObject);

							// check if EO satisfies the condition
							return EnumerateBottomInstructions(ieObject, [&](Trace* eoTrace, vint32_t eoIns)
							{
								if (failureReasons)
								{
									failureReasons->Add(L"    " +
										itow(eoTrace->allocatedIndex) + L"@" + itow(eoIns) +
										L".");
								}
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

					if (failureReasons)
					{
						failureReasons->Add(L"  [postfixesAtSelf]");
						for (vint i = 0; i < postfixesAtSelf.Count(); i++)
						{
							auto key = postfixesAtSelf.Keys()[i];
							WString message = L"  " + itow(key ? key->allocatedIndex : -1) + L" ->";
							for (auto&& value : postfixesAtSelf.GetByIndex(i))
							{
								message += L" " + itow(value.trace.handle) + L"@-" + itow(value.ins);
							}
							failureReasons->Add(message);
						}

						failureReasons->Add(L"  [postfixesAtSuccessor]");
						for (vint i = 0; i < postfixesAtSuccessor.Count(); i++)
						{
							auto key = postfixesAtSuccessor.Keys()[i];
							WString message = L"  " + itow(key ? key->allocatedIndex : -1) + L" ->";
							for (auto&& value : postfixesAtSuccessor.GetByIndex(i))
							{
								message += L" " + itow(value.trace.handle) + L"@-" + itow(value.ins);
							}
							failureReasons->Add(message);
						}
					}

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

					if (failureReasons)
					{
						failureReasons->Add(L"  [unique possible largest group]");
						failureReasons->Add(L"  postfixesAtSelf -> " + itow(uniqueAtSelf));
						failureReasons->Add(L"  postfixesAtSuccessor -> " + itow(uniqueAtSuccessor));
						failureReasons->Add(L"  lastPostfix -> " + itow(lastPostfix.trace.handle) + L"@" + itow(lastPostfix.ins));
					}

					if (lastPostfix.trace == nullref)
					{
						if (failureReasons)
						{
							failureReasons->Add(L"  lastPostfix has an empty trace, stopped.");
						}
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

				if (failureReasons)
				{
					failureReasons->Add(L"[TraceAmbiguity]");
				}

				// ensure the statistics result is compatible
				if (last && !foundEndSame && !foundEndPostfix) foundEndSame = true;
				if (firstTraceCount != firstSameTraceCount && firstTraceCount != firstSamePredecessorCount)
				{
					if (failureReasons)
					{
						failureReasons->Add(L"Some StackBegin instructions share the same trace while some share the same predecessor, stopped.");
					}
					return false;
				}
				if (foundEndSame == foundEndPostfix)
				{
					if (failureReasons)
					{
						failureReasons->Add(L"Some StackEnd instructions share the same trace while some share the same successor, stopped.");
					}
					return false;
				}

				// fix prefix if necessary
				if (firstTraceCount != firstSameTraceCount)
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

				if (failureReasons)
				{
					failureReasons->Add(L"firstTrace: " + itow(ta->firstTrace.handle));
					failureReasons->Add(L"prefix: " + itow(ta->prefix));
					failureReasons->Add(L"lastTrace: " + itow(ta->lastTrace.handle));
					failureReasons->Add(L"postfix: " + itow(ta->postfix));
					failureReasons->Add(L"firstForward: " + itow(firstForward->allocatedIndex));
					failureReasons->Add(L"lastForward (currentForward): " + itow(lastForward->allocatedIndex));
				}

				auto currentForward = lastForward;
				while (true)
				{
					if (currentForward->traceExecRef < firstForward->traceExecRef)
					{
						if (failureReasons)
						{
							failureReasons->Add(L"currentForward is before firstForward, stopped.");
						}
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
						if (failureReasons)
						{
							failureReasons->Add(L"currentForward steps forward to: " +
								itow(currentForward->allocatedIndex) +
								L".");
						}
					}
					else if (currentForward->predecessorCount > 0)
					{
						currentForward = GetTrace(GetTraceExec(GetTrace(currentForward->predecessors.first)->traceExecRef)->branchData.forwardTrace);
						if (failureReasons)
						{
							failureReasons->Add(L"currentForward steps forward passing a branch to: " +
								itow(currentForward->allocatedIndex) +
								L".");
						}
					}
					else
					{
						if (failureReasons)
						{
							failureReasons->Add(L"currentForward reaches the beginning of the trace, stopped.");
						}
						break;
					}
				}

				return false;
			}

/***********************************************************************
CheckSingleMergeTrace
***********************************************************************/

			bool TraceManager::CheckSingleMergeTrace(TraceAmbiguity* ta, Trace* trace, TraceExec* traceExec, collections::List<Ref<InsExec_StackRefLink>>& visitingIds, collections::List<WString>* failureReasons)
			{
				if (failureReasons)
				{
					failureReasons->Add(L"Trying to merge trace " +
						itow(trace->allocatedIndex)
						+ L".");
				}

				// when a merge trace is the last trace
				// objects in the top object stack are the result of ambiguity
				if (trace->successorCount == 0)
				{
					if (failureReasons)
					{
						failureReasons->Add(L"This is the last trace, compare all concurrent objects in the objectStack top.");
					}
					auto ieOSTop = GetInsExec_StackArrayRefLink(traceExec->context.objectStack);
					return CheckAmbiguityResolution(ta, visitingIds, failureReasons, [=](auto&& callback)
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
						if (failureReasons)
						{
							failureReasons->Add(L"The first predecessor " +
								itow(firstTrace->allocatedIndex) +
								L" has no StackEnd.");
						}

						goto CHECK_OBJECTS_IN_TOP_CREATE_STACK;
					}

					if (failureReasons)
					{
						failureReasons->Add(L"The first predecessor " +
							itow(firstTrace->allocatedIndex) +
							L" has StackEnd, its postfix is " +
							itow(postfix) +
							L".");
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
								if (failureReasons)
								{
									failureReasons->Add(L"Another predecessor " +
										itow(predecessor->allocatedIndex) +
										L" has no StackEnd or has a different postfix.");
								}
								goto CHECK_OBJECTS_IN_TOP_CREATE_STACK;
							}
						}
					}

					// check if all StackEnd ended objects are the result of ambiguity
					if (postfix == 0)
					{
						// if StackEnd is the last instruction of predecessors
						// then their objRefs has been written to the top object stack
						if (failureReasons)
						{
							failureReasons->Add(L"The postfix is 0, compare all concurrent objects in the objectStack top.");
						}
						auto ieOSTop = GetInsExec_StackArrayRefLink(traceExec->context.objectStack);
						auto succeeded = CheckAmbiguityResolution(ta, visitingIds, failureReasons, [=](auto&& callback)
						{
							return callback(ieOSTop->ids);
						});
						if (succeeded) return true;
					}
					else
					{
						// otherwise find all objRefs of StackEnd
						if (failureReasons)
						{
							failureReasons->Add(L"The postfix > 0, compare all concurrent objects in all StackEnd instructions.");
						}
						auto succeeded = CheckAmbiguityResolution(ta, visitingIds, failureReasons, [=, this, &visitingIds](auto&& callback)
						{
							auto predecessorId = trace->predecessors.first;
							while (predecessorId != nullref)
							{
								auto predecessor = GetTrace(predecessorId);
								predecessorId = predecessor->predecessors.siblingNext;

								// search for the object it ends
								if (failureReasons)
								{
									failureReasons->Add(L"Verifying predecessor " +
										itow(predecessor->allocatedIndex) +
										L".");
								}
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
				if (failureReasons)
				{
					failureReasons->Add(L"All condition dissatisfied, compare all concurrent objects in the createStack top.");
				}
				auto ieCSTop = GetInsExec_StackArrayRefLink(traceExec->context.createStack);
				return CheckAmbiguityResolution(ta, visitingIds, failureReasons, [=](auto&& callback)
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
						throw TraceException(*this, ta, ta2, TRACE_MAMAGER_PHRASE, L"Incompatible TraceAmbiguity has been assigned at the same place");
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
					bool succeeded = CheckSingleMergeTrace(ta, trace, traceExec, visitingIds, nullptr);
					if (!succeeded)
					{
						List<WString> failureReasons;
						CheckSingleMergeTrace(ta, trace, traceExec, visitingIds, &failureReasons);

						throw TraceException(*this, trace, nullptr, TRACE_MAMAGER_PHRASE, stream::GenerateToStream([&](stream::TextWriter& writer)
						{
							writer.WriteLine(L"Failed to find ambiguous objects in a merge trace.");
							writer.WriteLine(L"[Details]");
							for (auto&& reason : failureReasons)
							{
								writer.WriteLine(reason);
							}
						}));
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