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
CalculateObjectFirstInstruction
***********************************************************************/

			bool TraceManager::UpdateTopTrace(InsRef& topInsRef, InsRef newInsRef)
			{
				if (
					topInsRef.trace == nullref ||
					topInsRef.trace > newInsRef.trace ||
					(topInsRef.trace == newInsRef.trace && topInsRef.ins > newInsRef.ins)
					)
				{
					topInsRef = newInsRef;
					return true;
				}
				else
				{
					return false;
				}
			}

			void TraceManager::InjectFirstInstruction(InsRef insRef, Ref<InsExec_StackRefLink> injectTargets, vuint64_t magicInjection)
			{
				auto objLinkRef = injectTargets;
				while (objLinkRef != nullref)
				{
					auto objLink = GetInsExec_StackRefLink(objLinkRef);
					objLinkRef = objLink->previous;
					auto ieObject = GetInsExec_Stack(objLink->id);

					if (ieObject->mergeCounter == magicInjection) continue;
					ieObject->mergeCounter = magicInjection;

					// there will be only one top create instruction per object
					// even when object relationship is partial ordered
					// TODO: prove it
					if (UpdateTopTrace(ieObject->topInsRef, insRef))
					{
						InjectFirstInstruction(insRef, ieObject->assignedToObjectIds, magicInjection);
					}
				}
			}

			void TraceManager::CalculateObjectFirstInstruction()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CalculateObjectFirstInstruction()#"
				// check all individual objects
				{
					auto objRef = firstObject;
					while (objRef != nullref)
					{
						auto ieObject = GetInsExec_Object(objRef);
						objRef = ieObject->previous;

						// set the top local trace to its create trace
						UpdateTopTrace(ieObject->topLocalInsRef, ieObject->createInsRef);

						// check all DFA instructions
						auto insRefLinkId = ieObject->dfaInsRefs;
						while (insRefLinkId != nullref)
						{
							auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
							insRefLinkId = insRefLink->previous;

							// there will be only one top local create instruction per object
							// even when object relationship is partial ordered
							// TODO: prove it
							UpdateTopTrace(ieObject->topLocalInsRef, insRefLink->insRef);
						}

						// set the top trace to its top local trace
						UpdateTopTrace(ieObject->topInsRef, ieObject->topLocalInsRef);
					}
				}

				// check all assigned to targets
				{
					auto objRef = firstObject;
					while (objRef != nullref)
					{
						auto ieObject = GetInsExec_Object(objRef);
						objRef = ieObject->previous;

						NEW_MERGE_STACK_MAGIC_COUNTER;
						auto magicInjection = MergeStack_MagicCounter;
						ieObject->mergeCounter = magicInjection;
						InjectFirstInstruction(ieObject->topInsRef, ieObject->assignedToObjectIds, magicInjection);
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

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

/***********************************************************************
SummarizeIndirectCreateObjectInsRefs
***********************************************************************/

			void TraceManager::SummarizeIndirectCreateObjectInsRefs()
			{
				NEW_MERGE_STACK_MAGIC_COUNTER;
				auto stackMagicCounter = MergeStack_MagicCounter;

				// traverse through all stacks
				auto currentStackRef = firstStack;
				while (currentStackRef != nullref)
				{
					auto currentStack = GetInsExec_Stack(currentStackRef);
					currentStackRef = currentStack->previous;
					if (currentStack->mergeCounter == stackMagicCounter) continue;
					currentStack->mergeCounter = stackMagicCounter;

					List<InsExec_Stack*> indirectStacks;
					indirectStacks.Add(currentStack);

					// list all untouched useFromStacks in order, skipped processed ones
					for (vint i = 0; i < indirectStacks.Count(); i++)
					{
						auto stack = indirectStacks[i];
						auto currentInsRefLink = stack->useFromStacks;
						while (currentInsRefLink != nullref)
						{
							auto insRefLink = GetInsExec_StackRefLink(currentInsRefLink);
							currentInsRefLink = insRefLink->previous;

							auto useFromStack = GetInsExec_Stack(insRefLink->id);
							if (useFromStack->mergeCounter == stackMagicCounter) continue;
							useFromStack->mergeCounter = stackMagicCounter;
							indirectStacks.Add(useFromStack);
						}
					}

					// process all listed stacks
					for (vint i = indirectStacks.Count() - 1; i >= 0; i--)
					{
						auto stack = indirectStacks[i];
						Ref<InsExec_InsRefLink> potentialInsRefs;

						// accumulate all createObjectInsRefs from useFromStacks
						auto currentStackRefLink = stack->useFromStacks;
						while (currentStackRefLink != nullref)
						{
							auto stackRefLink = GetInsExec_StackRefLink(currentStackRefLink);
							currentStackRefLink = stackRefLink->previous;

							auto useFromStack = GetInsExec_Stack(stackRefLink->id);
							potentialInsRefs = JoinInsRefLink(potentialInsRefs, useFromStack->indirectCreateObjectInsRefs);
						}
						potentialInsRefs = JoinInsRefLink(potentialInsRefs, stack->createObjectInsRefs);

						// make sure InsExec_Stack::indirectCreateObjectInsRefs maps to InsExec_ObjectInstance::associatedStacks without duplication
						NEW_MERGE_STACK_MAGIC_COUNTER;
						auto objectMagicCounter = MergeStack_MagicCounter;

						auto currentInsRefLink = potentialInsRefs;
						while (currentInsRefLink != nullref)
						{
							auto insRefLink = GetInsExec_InsRefLink(currentInsRefLink);
							currentInsRefLink = insRefLink->previous;

							auto insTrace = GetTrace(insRefLink->insRef.trace);
							auto insTraceExec = GetTraceExec(insTrace->traceExecRef);
							auto insExec = GetInsExec(insTraceExec->insExecRefs.start + insRefLink->insRef.ins);
							auto insObject = GetInsExec_ObjectInstance(insExec->createdObject);
							if (insObject->mergeCounter == objectMagicCounter) continue;
							insObject->mergeCounter = objectMagicCounter;

							PushInsRefLink(stack->indirectCreateObjectInsRefs, insRefLink->insRef);
							PushStackRefLink(insObject->associatedStacks, stack);
						}
					}
				}
			}

/***********************************************************************
SummarizeInstructionRange
***********************************************************************/

			void TraceManager::SummarizeInstructionRange()
			{
				SummarizeIndirectCreateObjectInsRefs();
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}