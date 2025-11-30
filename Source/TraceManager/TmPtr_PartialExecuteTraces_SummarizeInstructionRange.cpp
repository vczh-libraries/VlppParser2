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
SummarizeObjectInstances
***********************************************************************/

			void TraceManager::SummarizeObjectInstances()
			{
			}

/***********************************************************************
SummarizeEarilestInsRefs
***********************************************************************/

			void TraceManager::SummarizeEarilestInsRefs()
			{
			}

/***********************************************************************
SummarizeInstructionRange
***********************************************************************/

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