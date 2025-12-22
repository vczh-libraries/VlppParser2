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
MergeInsExecContext
***********************************************************************/

			template<Ref<InsExec_StackArrayRefLink> (InsExec_Context::* stack), typename TMerge>
			Ref<InsExec_StackArrayRefLink> TraceManager::MergeStack(Trace* mergeTrace, TMerge&& merge)
			{
				Array<InsExec_StackArrayRefLink*> stacks(mergeTrace->predecessorCount);

				// fill the first level of stacks objects
				{
					vint index = 0;
					auto predecessorId = mergeTrace->predecessors.first;
					while (predecessorId != nullref)
					{
						auto predecessor = GetTrace(predecessorId);
						auto traceExec = GetTraceExec(predecessor->traceExecRef);

						auto stackId = traceExec->context.*stack;
						stacks[index++] = stackId == nullref ? nullptr : GetInsExec_StackArrayRefLink(stackId);
						predecessorId = predecessor->predecessors.siblingNext;
					}
				}

				Ref<InsExec_StackArrayRefLink> stackTop;
				auto pStackPrevious = &stackTop;
				while (stacks[0])
				{
					// check if all stack objects are the same
					bool sameStackObject = true;
					// TODO: (enumerable) Linq:Skip
					for (vint index = 1; index < stacks.Count(); index++)
					{
						if (stacks[0] != stacks[index])
						{
							sameStackObject = false;
							break;
						}
					}

					if (sameStackObject)
					{
						// if yes, reuse this stack object
						*pStackPrevious = stacks[0];
						break;
					}

					// otherwise, create a new stack object to merge all
					auto newStack = GetInsExec_StackArrayRefLink(insExec_StackArrayRefLinks.Allocate());
					*pStackPrevious = newStack;
					pStackPrevious = &(newStack->previous);

					{
						// call this macro to create a one-time set for InsExec*
						NEW_MERGE_STACK_MAGIC_COUNTER;
						auto magicPush = MergeStack_MagicCounter;
						// TODO: (enumerable) foreach
						for (vint index = 0; index < stacks.Count(); index++)
						{
							// do not visit the same stack object repeatly
							if (stacks[index]->mergeCounter == magicPush) continue;
							stacks[index]->mergeCounter = magicPush;
							merge(newStack, stacks[index]);

							// do not visit the same object repeatly
							auto currentLinkRef = stacks[index]->ids;
							while (currentLinkRef != nullref)
							{
								auto currentLink = GetInsExec_StackRefLink(currentLinkRef);
								currentLinkRef = currentLink->previous;
								auto currentStack = GetInsExec_Stack(currentLink->id);
								if (currentStack->mergeCounter == magicPush) continue;
								currentStack->mergeCounter = magicPush;
								PushStackRefLink(newStack->ids, currentStack);
							}
						}
					}

					// move to next level of stack objects
					for (vint index = 0; index < stacks.Count(); index++)
					{
						auto stackId = stacks[index]->previous;
						stacks[index] = stackId == nullref ? nullptr : GetInsExec_StackArrayRefLink(stackId);
					}
				}
				return stackTop;
			}

			void TraceManager::MergeInsExecContext(Trace* mergeTrace)
			{
				// merge stacks so that objects created in all branches are accessible
				auto traceExec = GetTraceExec(mergeTrace->traceExecRef);

				traceExec->context.objectStack = MergeStack<
					&InsExec_Context::objectStack
				>(
					mergeTrace,
					[this](InsExec_StackArrayRefLink* newStack, InsExec_StackArrayRefLink* commingStack)
					{
						newStack->currentDepth = commingStack->currentDepth;
					});

				traceExec->context.createStack = MergeStack<
					&InsExec_Context::createStack
				>(
					mergeTrace,
					[this](InsExec_StackArrayRefLink* newStack, InsExec_StackArrayRefLink* commingStack)
					{
						newStack->currentDepth = commingStack->currentDepth;
						newStack->objectStackDepthForCreateStack = commingStack->objectStackDepthForCreateStack;
					});

				NEW_MERGE_STACK_MAGIC_COUNTER;
				auto predecessorId = mergeTrace->predecessors.first;
				while (predecessorId != nullref)
				{
					auto predecessor = GetTrace(predecessorId);
					predecessorId = predecessor->predecessors.siblingNext;
					auto predecessorTraceExec = GetTraceExec(predecessor->traceExecRef);
				}
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}