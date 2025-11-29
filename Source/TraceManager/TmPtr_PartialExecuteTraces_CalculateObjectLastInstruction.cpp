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
CalculateObjectLastInstruction
***********************************************************************/

			bool TraceManager::IsInTheSameBranch(Trace* forward, Trace* targetForwardAtFront)
			{
				while (true)
				{
					// if two forwards are the same
					if (forward == targetForwardAtFront)
					{
						// then they are in the same branch
						return true;
					}
					else if (forward->traceExecRef > targetForwardAtFront->traceExecRef)
					{
						// otherwise
						auto forwardExec = GetTraceExec(forward->traceExecRef);
						if (forwardExec->branchData.commonForwardBranch != nullref)
						{
							// if commonForwardBranch exists, this is a merge trace
							auto commonForward = GetTrace(forwardExec->branchData.commonForwardBranch);
							if (commonForward->traceExecRef < targetForwardAtFront->traceExecRef)
							{
								// is the merge trace is in front of the targetForwardAtFront
								// check each branch
								auto predecessorId = forward->predecessors.first;
								while (predecessorId != nullref)
								{
									auto predecessor = GetTrace(predecessorId);
									predecessorId = predecessor->predecessors.siblingNext;

									auto predecessorExec = GetTraceExec(predecessor->traceExecRef);
									if (IsInTheSameBranch(GetTrace(predecessorExec->branchData.forwardTrace), targetForwardAtFront))
									{
										return true;
									}
								}

								// targetForwardAtFront could be among them, but could not be in front of them
								return false;
							}
						}

						// if commonForwardBranch doesn't contribute, look forward again
						auto nextForward = GetTrace(forwardExec->branchData.forwardTrace);
						if (nextForward == forward)
						{
							if (forward->predecessors.first == nullptr)
							{
								break;
							}
							else
							{
								forward = GetTrace(GetTraceExec(GetTrace(forward->predecessors.first)->traceExecRef)->branchData.forwardTrace);
							}
						}
						else
						{
							forward = nextForward;
						}
					}
					else
					{
						break;
					}
				}
				return false;
			}

			void TraceManager::CalculateObjectLastInstruction()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CalculateObjectLastInstruction()#"
				// check all individual objects
				{
					auto objRef = firstObject;
					while (objRef != nullref)
					{
						auto ieObject = GetInsExec_Object(objRef);
						objRef = ieObject->previous;

						// all StackEnd ending a StackBegin are considered
						// there is no "bottom StackEnd"
						// each StackEnd should be in different branches
						auto topLocalTrace = GetTrace(ieObject->topLocalInsRef.trace);
						auto topLocalTraceExec = GetTraceExec(topLocalTrace->traceExecRef);
						auto insExec = GetInsExec(topLocalTraceExec->insExecRefs.start + ieObject->topLocalInsRef.ins);
						auto insRefLinkId = insExec->eoInsRefs;

						// get the branch where StackBegin stays
						auto createTrace = GetTrace(ieObject->createInsRef.trace);
						auto createTraceExec = GetTraceExec(createTrace->traceExecRef);
						auto createTraceForward = GetTrace(createTraceExec->branchData.forwardTrace);

						NEW_MERGE_STACK_MAGIC_COUNTER;
						auto magicInsRef = MergeStack_MagicCounter;

						while (insRefLinkId != nullref)
						{
							auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
							insRefLinkId = insRefLink->previous;

							auto bottomInsRef = insRefLink->insRef;
							auto bottomTrace = GetTrace(bottomInsRef.trace);
							auto bottomTraceExec = GetTraceExec(bottomTrace->traceExecRef);
							auto bottomInsExec = GetInsExec(bottomTraceExec->insExecRefs.start + bottomInsRef.ins);
							if (bottomInsExec->mergeCounter != magicInsRef)
							{
								bottomInsExec->mergeCounter = magicInsRef;

								// filter out any result that does not happen after ieObject->createTrace
								// topLocalTrace could be a DFA created object, and multiple objects could share the same DFA object
								// in some cases its eoInsRefs could pointing to EndObject of completely unrelated objects
								// TODO: make it accurate

								if (IsInTheSameBranch(GetTrace(bottomTraceExec->branchData.forwardTrace), createTraceForward))
								{
									PushInsRefLink(ieObject->bottomInsRefs, bottomInsRef);
								}
							}
						}

						CHECK_ERROR(ieObject->bottomInsRefs != nullref, ERROR_MESSAGE_PREFIX L"Cannot found bottom instructions for an object.");
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}