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
IterateStackWithDependency
***********************************************************************/

			template<typename TCallback>
			void TraceManager::IterateStackWithDependency(Ref<InsExec_StackRefLink>(InsExec_Stack::* dependencies), TCallback&& callback)
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

					// list all untouched dependencies in order, skipped processed ones
					for (vint i = 0; i < indirectStacks.Count(); i++)
					{
						auto stack = indirectStacks[i];
						auto currentInsRefLink = stack->*dependencies;
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
						callback(stack);
					}
				}
			}

/***********************************************************************
SummarizeIndirectCreateObjectInsRefs
***********************************************************************/

			void TraceManager::SummarizeIndirectCreateObjectInsRefs()
			{
				IterateStackWithDependency(&InsExec_Stack::useFromStacks, [this](InsExec_Stack* stack)
				{
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
				});
			}

/***********************************************************************
SummarizeObjectInstances
***********************************************************************/

			void TraceManager::SummarizeObjectInstances()
			{
				// traverse through all object instances
				auto currentObjectRef = firstObjectInstance;
				while (currentObjectRef != nullref)
				{
					auto currentObject = GetInsExec_ObjectInstance(currentObjectRef);
					currentObjectRef = currentObject->previous;

					// summarize from associatedStacks
					SortedList<InsRef> endInsRefs;
					auto currentStackRefLink = currentObject->associatedStacks;
					while (currentStackRefLink != nullref)
					{
						auto stackRefLink = GetInsExec_StackRefLink(currentStackRefLink);
						currentStackRefLink = stackRefLink->previous;

						// beginInsRef
						auto stack = GetInsExec_Stack(stackRefLink->id);
						UpdateTopTrace(currentObject->beginInsRef, stack->beginInsRef);

						// endInsRefs
						CollectInsRefs(endInsRefs, stack->endWithCreateInsRefs);
						CollectInsRefs(endInsRefs, stack->endWithReuseInsRefs);
					}

					for (auto insRef : endInsRefs)
					{
						PushInsRefLink(currentObject->endInsRefs, insRef);
					}
				}
			}

/***********************************************************************
SummarizeEarilestInsRefs
***********************************************************************/

			void TraceManager::SummarizeEarilestInsRefs()
			{
				IterateStackWithDependency(&InsExec_Stack::fieldStacks, [this](InsExec_Stack* stack)
				{
					// traverse through all indirectCreateObjectInsRefs and get the earliest InsExec_ObjectInstance::beginInsRef
					auto currentInsRefLink = stack->indirectCreateObjectInsRefs;
					while (currentInsRefLink != nullref)
					{
						auto insRefLink = GetInsExec_InsRefLink(currentInsRefLink);
						currentInsRefLink = insRefLink->previous;

						// update earliestInsRef
						auto insTrace = GetTrace(insRefLink->insRef.trace);
						auto insTraceExec = GetTraceExec(insTrace->traceExecRef);
						auto insExec = GetInsExec(insTraceExec->insExecRefs.start + insRefLink->insRef.ins);
						auto insObject = GetInsExec_ObjectInstance(insExec->createdObject);
						UpdateTopTrace(stack->earliestInsRef, insObject->beginInsRef);
					}

					// traverse through all fieldStacks and get their earliestInsRef
					auto currentStackRefLink = stack->fieldStacks;
					while (currentStackRefLink != nullref)
					{
						auto stackRefLink = GetInsExec_StackRefLink(currentStackRefLink);
						currentStackRefLink = stackRefLink->previous;

						// update earliestInsRef
						auto fieldStack = GetInsExec_Stack(stackRefLink->id);
						UpdateTopTrace(stack->earliestInsRef, fieldStack->earliestInsRef);
					}
				});
			}

/***********************************************************************
SummarizeInstructionRange
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

			void TraceManager::CollectInsRefs(collections::SortedList<InsRef>& insRefs, Ref<InsExec_InsRefLink> link)
			{
				auto currentInsRefLink = link;
				while (currentInsRefLink != nullref)
				{
					auto insRefLink = GetInsExec_InsRefLink(currentInsRefLink);
					currentInsRefLink = insRefLink->previous;

					if (!insRefs.Contains(insRefLink->insRef))
					{
						insRefs.Add(insRefLink->insRef);
					}
				}
			}

			void TraceManager::SummarizeInstructionRange()
			{
				SummarizeIndirectCreateObjectInsRefs();
				SummarizeObjectInstances();
				SummarizeEarilestInsRefs();
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}