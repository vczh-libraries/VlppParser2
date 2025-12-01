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
SummarizeEarilestLocalInsRefs
***********************************************************************/

			void TraceManager::SummarizeEarilestLocalInsRefs()
			{
				IterateStackWithDependency(&InsExec_Stack::useFromStacks, [this](InsExec_Stack* stack)
				{
					auto currentStackRefLink = stack->useFromStacks;
					while (currentStackRefLink != nullref)
					{
						auto stackRefLink = GetInsExec_StackRefLink(currentStackRefLink);
						currentStackRefLink = stackRefLink->previous;

						auto useFromStack = GetInsExec_Stack(stackRefLink->id);
						UpdateTopTrace(stack->summarizing.earliestLocalInsRef, useFromStack->summarizing.earliestLocalInsRef);
					}
					UpdateTopTrace(stack->summarizing.earliestLocalInsRef, stack->beginInsRef);
				});
			}

/***********************************************************************
SummarizeEarilestInsRefs
***********************************************************************/

			void TraceManager::SummarizeEarilestInsRefs()
			{
				IterateStackWithDependency(&InsExec_Stack::useFromStacks, [this](InsExec_Stack* stack)
				{
					auto currentStackRefLink = stack->fieldStacks;
					while (currentStackRefLink != nullref)
					{
						auto stackRefLink = GetInsExec_StackRefLink(currentStackRefLink);
						currentStackRefLink = stackRefLink->previous;

						auto fieldStack = GetInsExec_Stack(stackRefLink->id);
						UpdateTopTrace(stack->summarizing.earliestInsRef, fieldStack->summarizing.earliestInsRef);
					}
					UpdateTopTrace(stack->summarizing.earliestInsRef, stack->summarizing.earliestLocalInsRef);
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
				SummarizeEarilestLocalInsRefs();
				SummarizeEarilestInsRefs();
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}