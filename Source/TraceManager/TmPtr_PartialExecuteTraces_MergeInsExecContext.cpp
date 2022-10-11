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

			void TraceManager::PushInsRefLinkWithCounter(Ref<InsExec_InsRefLink>& link, Ref<InsExec_InsRefLink> comming)
			{
				while (comming != nullref)
				{
					auto commingStack = GetInsExec_InsRefLink(comming);
					comming = commingStack->previous;

					auto insTrace = GetTrace(commingStack->trace);
					auto insTraceExec = GetTraceExec(insTrace->traceExecRef);
					auto insExec = GetInsExec(insTraceExec->insExecRefs.start + commingStack->ins);
					if (insExec->mergeCounter == MergeStack_MagicCounter) continue;

					insExec->mergeCounter = MergeStack_MagicCounter;
					PushInsRefLink(link, commingStack->trace, commingStack->ins);
				}
			}

			void TraceManager::PushObjRefLinkWithCounter(Ref<InsExec_ObjRefLink>& link, Ref<InsExec_ObjRefLink> comming)
			{
				while (comming != nullref)
				{
					auto commingStack = GetInsExec_ObjRefLink(comming);
					comming = commingStack->previous;

					auto ieObject = GetInsExec_Object(commingStack->id);
					if (ieObject->mergeCounter == MergeStack_MagicCounter) continue;

					ieObject->mergeCounter = MergeStack_MagicCounter;
					PushObjRefLink(link, ieObject);
				}
			}

			template<typename T, T* (TraceManager::* get)(Ref<T>), Ref<T> (InsExec_Context::* stack), typename TMerge>
			Ref<T> TraceManager::MergeStack(Trace* mergeTrace, AllocateOnly<T>& allocator, TMerge&& merge)
			{
				Array<T*> stacks(mergeTrace->predecessorCount);

				// fill the first level of stacks objects
				{
					vint index = 0;
					auto predecessorId = mergeTrace->predecessors.first;
					while (predecessorId != nullref)
					{
						auto predecessor = GetTrace(predecessorId);
						auto traceExec = GetTraceExec(predecessor->traceExecRef);

						auto stackId = traceExec->context.*stack;
						stacks[index++] = stackId == nullref ? nullptr : (this->*get)(stackId);
						predecessorId = predecessor->predecessors.siblingNext;
					}
				}

				Ref<T> stackTop;
				Ref<T>* pStackPrevious = &stackTop;
				while (stacks[0])
				{
					// check if all stack objects are the same
					bool sameStackObject = true;
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
					auto newStack = (this->*get)(allocator.Allocate());
					*pStackPrevious = newStack;
					pStackPrevious = &(newStack->previous);

					// call this macro to create a one-time set for InsExec*
					NEW_MERGE_STACK_MAGIC_COUNTER;
					for (vint index = 0; index < stacks.Count(); index++)
					{
						// do not visit the same stack object repeatly
						if (stacks[index]->mergeCounter == MergeStack_MagicCounter) continue;
						stacks[index]->mergeCounter = MergeStack_MagicCounter;
						merge(newStack, stacks[index]);

						// do not visit the same object repeatly
						PushObjRefLinkWithCounter(newStack->objectIds, stacks[index]->objectIds);
					}

					// move to next level of stack objects
					for (vint index = 0; index < stacks.Count(); index++)
					{
						auto stackId = stacks[index]->previous;
						stacks[index] = stackId == nullref ? nullptr : (this->*get)(stackId);
					}
				}
				return stackTop;
			}

			void TraceManager::MergeInsExecContext(Trace* mergeTrace)
			{
				// merge stacks so that objects created in all branches are accessible
				auto traceExec = GetTraceExec(mergeTrace->traceExecRef);

				traceExec->context.objectStack = MergeStack<
					InsExec_ObjectStack,
					&TraceManager::GetInsExec_ObjectStack,
					&InsExec_Context::objectStack
				>(
					mergeTrace,
					insExec_ObjectStacks,
					[this](InsExec_ObjectStack* newStack, InsExec_ObjectStack* commingStack)
					{
						// all commingStack->pushedCount are ensured to be the same
						newStack->pushedCount = commingStack->pushedCount;
					});

				traceExec->context.createStack = MergeStack<
					InsExec_CreateStack,
					&TraceManager::GetInsExec_CreateStack,
					&InsExec_Context::createStack
				>(
					mergeTrace,
					insExec_CreateStacks,
					[this](InsExec_CreateStack* newStack, InsExec_CreateStack* commingStack)
					{
						// all commingStack->stackBase are ensured to be the same
						newStack->stackBase = commingStack->stackBase;
						PushInsRefLinkWithCounter(newStack->createInsRefs, commingStack->createInsRefs);
					});

				NEW_MERGE_STACK_MAGIC_COUNTER;
				auto predecessorId = mergeTrace->predecessors.first;
				while (predecessorId != nullref)
				{
					auto predecessor = GetTrace(predecessorId);
					predecessorId = predecessor->predecessors.siblingNext;
					auto predecessorTraceExec = GetTraceExec(predecessor->traceExecRef);

					// do not visit the same object repeatly
					PushObjRefLinkWithCounter(traceExec->context.lriStoredObjects, predecessorTraceExec->context.lriStoredObjects);
				}
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}