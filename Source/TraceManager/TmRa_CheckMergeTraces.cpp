#include "TraceManager.h"

#if defined VCZH_MSVC && defined _DEBUG
#define VCZH_DO_DEBUG_CHECK
#endif

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
			bool TraceManager::SearchForObjects(Ref<InsExec_ObjRefLink> objRefLinkStartSet, bool withCounter, TCallback&& callback)
			{
				// check every object in the link
				auto linkId = objRefLinkStartSet;
				while (linkId != nullref)
				{
					auto objRefLink = GetInsExec_ObjRefLink(linkId);
					linkId = objRefLink->previous;
					auto ieObject = GetInsExec_Object(objRefLink->id);

					if (withCounter)
					{
						// skip if it has been searched
						if (ieObject->mergeCounter == MergeStack_MagicCounter) goto CHECK_NEXT_OBJECT;
						ieObject->mergeCounter = MergeStack_MagicCounter;
					}

					if (!callback(ieObject)) return false;
				CHECK_NEXT_OBJECT:;
				}
				return true;
			}

			template<typename TCallback>
			bool TraceManager::SearchForAllLevelObjectsWithCounter(InsExec_Object* startObject, collections::List<Ref<InsExec_ObjRefLink>>& visitingIds, TCallback&& callback)
			{
#define PUSH_ID(ID)													\
					do{												\
						if (availableIds == visitingIds.Count())	\
							visitingIds.Add(ID);					\
						else										\
							visitingIds[availableIds] = ID;			\
						availableIds++;								\
					} while (false)

				vint availableIds = 0;
				auto processObject = [&](InsExec_Object* ieObject)
				{
					// skip if it has been searched
					if (ieObject->mergeCounter == MergeStack_MagicCounter) return true;
					ieObject->mergeCounter = MergeStack_MagicCounter;
					if (!callback(ieObject)) return false;

					// keep searching until ieObject->lrObjectIds is empty
					while (ieObject->lrObjectIds != nullref)
					{
						auto lrObjRefLink = GetInsExec_ObjRefLink(ieObject->lrObjectIds);
						if (lrObjRefLink->previous == nullref)
						{
							// if ieObject->lrObjectIds has only one object
							// continue in place
							ieObject = GetInsExec_Object(lrObjRefLink->id);

							// skip if it has been searched
							if (ieObject->mergeCounter == MergeStack_MagicCounter) return true;
							ieObject->mergeCounter = MergeStack_MagicCounter;
							if (!callback(ieObject)) return false;
						}
						else
						{
							// otherwise
							// the link is pushed and search it later
							PUSH_ID(ieObject->lrObjectIds);
							break;
						}
					}

					return true;
				};

				// start with startObject
				if (!processObject(startObject)) return false;
				for (vint linkIdIndex = 0; linkIdIndex < availableIds; linkIdIndex++)
				{
					// for any new object link, check every object in it
					auto linkId = visitingIds[linkIdIndex];
					while (linkId != nullref)
					{
						auto objRefLink = GetInsExec_ObjRefLink(linkId);
						linkId = objRefLink->previous;
						auto ieObject = GetInsExec_Object(objRefLink->id);
						if (!processObject(ieObject)) return false;
					}
				}
				return true;
#undef PUSH_ID
			}

#ifdef VCZH_DO_DEBUG_CHECK
			void TraceManager::EnsureSameForwardTrace(Ref<Trace> currentTraceId, Ref<Trace> forwardTraceId)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::EnsureSameForwardTrace(vint32_t, vint32_t)#"
				auto currentTrace = GetTrace(currentTraceId);
				auto currentTraceExec = GetTraceExec(currentTrace->traceExecRef);
				while (currentTraceExec->branchData.forwardTrace > forwardTraceId)
				{
					currentTrace = StepForward(currentTrace);
					currentTraceExec = GetTraceExec(currentTrace->traceExecRef);
				}
				CHECK_ERROR(currentTraceExec->branchData.forwardTrace == forwardTraceId, ERROR_MESSAGE_PREFIX L"Internal error: assumption is broken.");
#undef ERROR_MESSAGE_PREFIX
			}
#endif

			template<typename TCallback>
			bool TraceManager::SearchForTopCreateInstructions(InsExec_Object* ieObject, TCallback&& callback)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::SearchForTopCreateInstructions(InsExec_Object*, TCallback&&)#"
				// find the first instruction in all create instructions
				// its trace should be a common ancestor of all traces of all create instructions
				auto trace = ieObject->bo_bolr_Trace;
				vint32_t ins = ieObject->bo_bolr_Ins;

				auto insRefLinkId = ieObject->dfaInsRefs;
				while (insRefLinkId != nullref)
				{
					auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
					insRefLinkId = insRefLink->previous;
					if (insRefLink->trace < trace || (insRefLink->trace == trace && insRefLink->ins < ins))
					{
						trace = insRefLink->trace;
						ins = insRefLink->ins;
					}
				}

#ifdef VCZH_DO_DEBUG_CHECK
				// ensure they actually have the same ancestor trace
				auto forwardTraceId = GetTraceExec(GetTrace(trace)->traceExecRef)->branchData.forwardTrace;
				EnsureSameForwardTrace(ieObject->bo_bolr_Trace, forwardTraceId);
				insRefLinkId = ieObject->dfaInsRefs;
				while (insRefLinkId != nullref)
				{
					auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
					EnsureSameForwardTrace(GetInsExec_InsRefLink(insRefLinkId)->trace, forwardTraceId);
					insRefLinkId = insRefLink->previous;
				}
#endif
				// there will be only one top create instruction per object
				return callback(trace, ins);
#undef ERROR_MESSAGE_PREFIX
			}

			template<typename TCallback>
			bool TraceManager::SearchForTopCreateInstructionsInAllLevelsWithCounter(InsExec_Object* startObject, collections::List<Ref<InsExec_ObjRefLink>>& visitingIds, TCallback&& callback)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::SearchForTopCreateInstructionsInAllLevelsWithCounter(InsExec_Object*, List<vint32_t>&, TCallback&&)#"
#ifdef VCZH_DO_DEBUG_CHECK
				Ref<InsExec_InsRefLink> insForEachObject;
#endif
				Ref<Trace> trace;
				vint32_t ins = -1;

				bool succeeded = SearchForAllLevelObjectsWithCounter(startObject, visitingIds, [&](InsExec_Object* ieObject)
				{
					return SearchForTopCreateInstructions(ieObject, [&](Ref<Trace> createTraceId, vint32_t createIns)
					{
#ifdef VCZH_DO_DEBUG_CHECK
						PushInsRefLink(insForEachObject, createTraceId, createIns);
#endif
						if (trace == nullref || createTraceId < trace || (createTraceId == trace && createIns < ins))
						{
							trace = createTraceId;
							ins = createIns;
						}
						return true;
					});
				});
				if (trace == nullref) return true;
				if (!succeeded) return false;
#ifdef VCZH_DO_DEBUG_CHECK
				// ensure they actually have the same ancestor trace
				auto forwardTraceId = GetTraceExec(GetTrace(trace)->traceExecRef)->branchData.forwardTrace;
				auto insRefLinkId = insForEachObject;
				while (insRefLinkId != nullref)
				{
					auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
					EnsureSameForwardTrace(insRefLink->trace, forwardTraceId);
					insRefLinkId = insRefLink->previous;
				}
#endif
				// there will be only one top create instruction per object
				// even when InsExec_Object::lrObjectIds are considered
				return callback(trace, ins);
#undef ERROR_MESSAGE_PREFIX
			}

			template<typename TCallback>
			bool TraceManager::SearchForEndObjectInstructions(Trace* createTrace, vint32_t createIns, TCallback&& callback)
			{
				// all EndObject ending a BO/BOLR/DFA are considered
				// there is no "bottom EndObject"
				// each EndObject should be in different branches
				auto traceExec = GetTraceExec(createTrace->traceExecRef);
				auto insExec = GetInsExec(traceExec->insExecRefs.start + createIns);
				auto insRefLinkId = insExec->eoInsRefs;
				while (insRefLinkId != nullref)
				{
					auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
					insRefLinkId = insRefLink->previous;
					if (!callback(GetTrace(insRefLink->trace), insRefLink->ins)) return false;
				}
				return true;
			}

			bool TraceManager::ComparePrefix(TraceExec* baselineTraceExec, TraceExec* commingTraceExec, vint32_t prefix)
			{
				if (commingTraceExec->insLists.c3 < prefix) return false;
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
				if (commingTraceExec->insLists.c3 < postfix) return false;
				for (vint32_t i = 0; i < postfix; i++)
				{
					auto&& insBaseline = ReadInstruction(baselineTraceExec->insLists.c3 - i - 1, baselineTraceExec->insLists);
					auto&& insComming = ReadInstruction(baselineTraceExec->insLists.c3 - i - 1, baselineTraceExec->insLists);
					if (insBaseline != insComming) return false;
				}

				return true;
			}

			template<typename TCallback>
			bool TraceManager::CheckAmbiguityResolution(TraceAmbiguity* ta, collections::List<Ref<InsExec_ObjRefLink>>& visitingIds, TCallback&& callback)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CheckAmbiguityResolution(TraceAmbiguity&, List<vint32_t>&, TCallback&&)#"
				// following conditions need to be satisfies if multiple objects could be the result of ambiguity
				//
				// BO/DFA that create objects must be
				//   the same instruction in the same trace
				//   in different trace
				//     these traces share the same predecessor
				//     prefix in these traces are the same
				//
				// EO that ed objects must be
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
				succeeded = callback([&](Ref<InsExec_ObjRefLink> objRefLink)
				{
					return SearchForObjects(objRefLink, false, [&](InsExec_Object* ieObject)
					{
						// check if BO/DFA satisfies the condition
						NEW_MERGE_STACK_MAGIC_COUNTER;
						return SearchForTopCreateInstructionsInAllLevelsWithCounter(ieObject, visitingIds, [&](Ref<Trace> createTraceId, vint32_t createIns)
						{
							auto createTrace = GetTrace(createTraceId);
#ifdef VCZH_DO_DEBUG_CHECK
							{
								auto traceExec = GetTraceExec(createTrace->traceExecRef);
								auto&& ins = ReadInstruction(createIns, traceExec->insLists);
								CHECK_ERROR(ins.type == AstInsType::BeginObject || ins.type == AstInsType::DelayFieldAssignment, ERROR_MESSAGE_PREFIX L"The found instruction is not a BeginObject or DelayFieldAssignment instruction.");
							}
#endif

							if (!first)
							{
								first = createTrace;
								firstTraceExec = GetTraceExec(first->traceExecRef);
								ta->firstTrace = createTrace;
								ta->prefix = createIns;
							}
							else if (first == createTrace)
							{
								// check if two instruction is the same
								if (ta->prefix != createIns) return false;
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
				});
				if (!succeeded) return false;

				// iterate all bottom objects
				NEW_MERGE_STACK_MAGIC_COUNTER;
				succeeded = callback([&](Ref<InsExec_ObjRefLink> objRefLink)
				{
					return SearchForObjects(objRefLink, true, [&](InsExec_Object* ieObject)
					{
						PushObjRefLink(ta->bottomObjectIds, ieObject);

						// check if BO/DFA satisfies the condition
						return SearchForTopCreateInstructions(ieObject, [&](Ref<Trace> createTraceId, vint32_t createIns)
						{
							auto createTrace = GetTrace(createTraceId);
#ifdef VCZH_DO_DEBUG_CHECK
							{
								auto traceExec = GetTraceExec(createTrace->traceExecRef);
								auto&& ins = ReadInstruction(createIns, traceExec->insLists);
								CHECK_ERROR(ins.type == AstInsType::BeginObject || ins.type == AstInsType::DelayFieldAssignment, ERROR_MESSAGE_PREFIX L"The found instruction is not a BeginObject or DelayFieldAssignment instruction.");
							}
#endif

							// check if EO satisfies the condition
							return SearchForEndObjectInstructions(createTrace, createIns, [&](Trace* eoTrace, vint32_t eoIns)
							{
#ifdef VCZH_DO_DEBUG_CHECK
								{
									auto traceExec = GetTraceExec(eoTrace->traceExecRef);
									auto&& ins = ReadInstruction(eoIns, traceExec->insLists);
									CHECK_ERROR(ins.type == AstInsType::EndObject, ERROR_MESSAGE_PREFIX L"The found instruction is not a EndObject instruction.");
								}
#endif

								if (!last)
								{
									last = eoTrace;
									lastTraceExec = GetTraceExec(last->traceExecRef);
									ta->lastTrace = eoTrace;
									ta->postfix = lastTraceExec->insLists.c3 - eoIns - 1;
								}
								else if (last == eoTrace)
								{
									// check if two instruction is the same
									auto eoTraceExec = GetTraceExec(eoTrace->traceExecRef);
									if (ta->postfix != eoTraceExec->insLists.c3 - eoIns - 1) return false;
									foundEndSame = true;
								}
								else
								{
									// check if two instruction shares the same postfix
									if (last->successors.first != eoTrace->successors.first) return false;
									auto eoTraceExec = GetTraceExec(eoTrace->traceExecRef);
									if (!ComparePostfix(lastTraceExec, eoTraceExec, ta->postfix + 1)) return false;
									foundEndPostfix = true;
								}
								return true;
							});
						});
					});
				});
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
					ta->prefix += traceExec->insLists.c3;
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

				return true;
#undef ERROR_MESSAGE_PREFIX
			}

			bool TraceManager::CheckMergeTrace(TraceAmbiguity* ta, Trace* trace, TraceExec* traceExec, collections::List<Ref<InsExec_ObjRefLink>>& visitingIds)
			{
				// when a merge trace is the surviving trace
				// objects in the top object stack are the result of ambiguity
				if (trace->successorCount == 0)
				{
					auto ieOSTop = GetInsExec_ObjectStack(traceExec->context.objectStack);
					return CheckAmbiguityResolution(ta, visitingIds, [=](auto&& callback)
					{
						return callback(ieOSTop->objectIds);
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
					// the first predecessor must has a EndObject instruction
					// count the number of instructions after EndObject
					// these instructions are the postfix
					vint32_t postfix = -1;
					auto firstTrace = GetTrace(trace->predecessors.first);
					auto firstTraceExec = GetTraceExec(firstTrace->traceExecRef);
					for (vint32_t i = firstTraceExec->insLists.c3 - 1; i >= 0; i--)
					{
						auto&& ins = ReadInstruction(i, firstTraceExec->insLists);
						if (ins.type == AstInsType::EndObject)
						{
							postfix = firstTraceExec->insLists.c3 - i - 1;
							break;
						}
					}
					if (postfix == -1)
					{
						goto CHECK_OBJECTS_IN_TOP_CREATE_STACK;
					}

					// [CONDITION]
					// all predecessor must have a EndObject instruction
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

					// check if all EndObject ended objects are the result of ambiguity
					if (postfix == 0)
					{
						// if EndObject is the last instruction of predecessors
						// then their objRefs has been written to the top object stack
						auto ieOSTop = GetInsExec_ObjectStack(traceExec->context.objectStack);
						auto succeeded = CheckAmbiguityResolution(ta, visitingIds, [=](auto&& callback)
						{
							return callback(ieOSTop->objectIds);
						});
						if (succeeded) return true;
					}
					else
					{
						// otherwise find all objRefs of EndObject
						auto succeeded = CheckAmbiguityResolution(ta, visitingIds, [=, &visitingIds](auto&& callback)
						{
							auto predecessorId = trace->predecessors.first;
							while (predecessorId != nullref)
							{
								auto predecessor = GetTrace(predecessorId);
								predecessorId = predecessor->predecessors.siblingNext;

								// search for the object it ends
								auto predecessorTraceExec = GetTraceExec(predecessor->traceExecRef);
								auto indexEO = predecessorTraceExec->insLists.c3 - postfix - 1;
								auto insExecEO = GetInsExec(predecessorTraceExec->insExecRefs.start + indexEO);
								if (!callback(insExecEO->objRefs)) return false;
							}
							return true;
						});
						if (succeeded) return true;
					}

				}
			CHECK_OBJECTS_IN_TOP_CREATE_STACK:
				auto ieCSTop = GetInsExec_CreateStack(traceExec->context.createStack);
				return CheckAmbiguityResolution(ta, visitingIds, [=](auto&& callback)
				{
					return callback(ieCSTop->objectIds);
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
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CheckTraceAmbiguity(TraceAmbiguity*)#"
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
#ifdef VCZH_DO_DEBUG_CHECK
					CHECK_ERROR(ta2->prefix == ta->prefix, ERROR_MESSAGE_PREFIX L"Incompatible TraceAmbiguity has been assigned at the same place.");
					CHECK_ERROR(ta2->postfix == ta->postfix, ERROR_MESSAGE_PREFIX L"Incompatible TraceAmbiguity has been assigned at the same place.");
#endif
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
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
DebugCheckTraceAmbiguityInSameTrace
***********************************************************************/

#ifdef VCZH_DO_DEBUG_CHECK
			void TraceManager::DebugCheckTraceAmbiguitiesInSameTrace(Trace* trace, TraceExec* traceExec)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::DebugCheckTraceAmbiguityInSameTrace(Trace*, TraceExec*)#"

				// if there are multiple ambiguityBegins
				// first ambiguity instructions must all be in successors
				vint faiInBranch = 0;
				vint faiInSuccessor = 0;
				auto taLinkRef = traceExec->ambiguityBegins;
				while (taLinkRef != nullref)
				{
					auto taLink = GetTraceAmbiguityLink(taLinkRef);
					taLinkRef = taLink->previous;

					auto ta = GetTraceAmbiguity(taLink->ambiguity);
					if (ta->prefix >= traceExec->insLists.c3)
					{
						faiInSuccessor++;
					}
					else
					{
						faiInBranch++;
					}
				}
				CHECK_ERROR((faiInBranch == 1 && faiInSuccessor == 0) || faiInBranch == 0, ERROR_MESSAGE_PREFIX L"Incompatible TraceAmbiguity has been assigned at the same place.");
#undef ERROR_MESSAGE_PREFIX
			}
#endif

/***********************************************************************
CategorizeTraceAmbiguities
***********************************************************************/

			void TraceManager::MarkAmbiguityCoveredForward(Trace* currentTrace, TraceAmbiguity* ta, Trace* firstTrace, TraceExec* firstTraceExec)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::MarkAmbiguityCoveredForward(Trace*, TraceAmbiguity*, Trace*, TraceExec*)#"
				while (true)
				{
					auto forward = GetTrace(GetTraceExec(currentTrace->traceExecRef)->branchData.forwardTrace);
					CHECK_ERROR(forward->traceExecRef > firstTraceExec, ERROR_MESSAGE_PREFIX L"Unexpected ambiguity resolving structure found.");

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
#undef ERROR_MESSAGE_PREFIX
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
					if (ta->prefix >= traceExec->insLists.c3)
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
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CheckMergeTraces()#"
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
				List<Ref<InsExec_ObjRefLink>> visitingIds;
				auto traceId = firstMergeTrace;
				while (traceId != nullref)
				{
					auto trace = GetTrace(traceId);
					auto traceExec = GetTraceExec(trace->traceExecRef);
					traceId = traceExec->nextMergeTrace;

					auto ta = GetTraceAmbiguity(traceAmbiguities.Allocate());
					bool succeeded = CheckMergeTrace(ta, trace, traceExec, visitingIds);
					CHECK_ERROR(succeeded, ERROR_MESSAGE_PREFIX L"Failed to find ambiguous objects in a merge trace.");
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
#ifdef VCZH_DO_DEBUG_CHECK
							DebugCheckTraceAmbiguitiesInSameTrace(trace, traceExec);
#endif
							CategorizeTraceAmbiguities(trace, traceExec);
						}
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}

#if defined VCZH_MSVC && defined _DEBUG
#undef VCZH_DO_DEBUG_CHECK
#endif