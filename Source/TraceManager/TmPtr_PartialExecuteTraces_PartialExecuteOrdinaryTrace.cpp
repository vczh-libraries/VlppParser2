#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
#define NEW_MERGE_STACK_MAGIC_COUNTER (void)(MergeStack_MagicCounter++)

/***********************************************************************
PartialExecuteOrdinaryTrace
***********************************************************************/

			InsExec_Stack* TraceManager::NewStack()
			{
				auto ieStack = GetInsExec_Stack(insExec_Stacks.Allocate());
				ieStack->previous = firstStack;
				firstStack = ieStack;
				return ieStack;
			}

			void TraceManager::PushInsRefLink(Ref<InsExec_InsRefLink>& link, InsRef insRef)
			{
				auto newLink = GetInsExec_InsRefLink(insExec_InsRefLinks.Allocate());
				newLink->previous = link;
				newLink->insRef = insRef;
				link = newLink;
			}

			void TraceManager::PushStackRefLink(Ref<InsExec_StackRefLink>& link, Ref<InsExec_Stack> id)
			{
				auto newLink = GetInsExec_StackRefLink(insExec_StackRefLinks.Allocate());
				newLink->previous = link;
				newLink->id = id;
				link = newLink;
			}

			void TraceManager::PushStackArrayRefLink(Ref<InsExec_StackArrayRefLink>& arrayLink, Ref<InsExec_Stack> id)
			{
				Ref<InsExec_StackRefLink> link;
				PushStackRefLink(link, id);
				PushStackArrayRefLink(arrayLink, link);
			}

			void TraceManager::PushStackArrayRefLink(Ref<InsExec_StackArrayRefLink>& arrayLink, Ref<InsExec_StackRefLink> link)
			{
				auto newArrayLink = GetInsExec_StackArrayRefLink(insExec_StackArrayRefLinks.Allocate());
				newArrayLink->previous = arrayLink;
				newArrayLink->ids = link;

				if (arrayLink == nullref)
				{
					newArrayLink->currentDepth = 0;
				}
				else
				{
					newArrayLink->currentDepth = GetInsExec_StackArrayRefLink(arrayLink)->currentDepth + 1;
				}
				arrayLink = newArrayLink;
			}

			Ref<InsExec_InsRefLink> TraceManager::JoinInsRefLink(Ref<InsExec_InsRefLink> first, Ref<InsExec_InsRefLink> second)
			{
				if (first == nullref) return second;
				if (second == nullref) return first;

				Ref<InsExec_InsRefLink> newInsRef;

				while (first != nullref)
				{
					auto insRef = GetInsExec_InsRefLink(first);
					first = insRef->previous;
					PushInsRefLink(newInsRef, insRef->insRef);
				}

				while (second != nullref)
				{
					auto insRef = GetInsExec_InsRefLink(second);
					second = insRef->previous;
					PushInsRefLink(newInsRef, insRef->insRef);
				}

				return newInsRef;
			}

			Ref<InsExec_StackRefLink> TraceManager::JoinStackRefLink(Ref<InsExec_StackRefLink> first, Ref<InsExec_StackRefLink> second)
			{
				if (first == nullref) return second;
				if (second == nullref) return first;

				Ref<InsExec_StackRefLink> newStack;

				while (first != nullref)
				{
					auto stack = GetInsExec_StackRefLink(first);
					first = stack->previous;
					PushStackRefLink(newStack, stack->id);
				}

				while (second != nullref)
				{
					auto stack = GetInsExec_StackRefLink(second);
					second = stack->previous;
					PushStackRefLink(newStack, stack->id);
				}

				return newStack;
			}

			void TraceManager::PartialExecuteOrdinaryTrace(Trace* trace)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::PartialExecuteOrdinaryTrace(Trace*)#"
				InsExec_Context context;
				if (trace->predecessors.first != nullref)
				{
					auto predecessor = GetTrace(trace->predecessors.first);
					auto traceExec = GetTraceExec(predecessor->traceExecRef);
					context = traceExec->context;
				}

				auto ForEachStack = [this](Ref<InsExec_StackArrayRefLink> targetStack, auto&& callback)
				{
						auto topStackArray = GetInsExec_StackArrayRefLink(targetStack);
						auto topStackLinkRef = topStackArray->ids;
						while (topStackLinkRef != nullref)
						{
							auto topStackLink = GetInsExec_StackRefLink(topStackLinkRef);
							topStackLinkRef = topStackLink->previous;
							auto topStack = GetInsExec_Stack(topStackLink->id);
							callback(topStack);
						}
				};

				auto traceExec = GetTraceExec(trace->traceExecRef);
				for (vint32_t insRef = 0; insRef < traceExec->insLists.countAll; insRef++)
				{
					auto&& ins = ReadInstruction(insRef, traceExec->insLists);
					auto insExec = GetInsExec(traceExec->insExecRefs.start + insRef);
					insExec->contextBeforeExecution = context;

					switch (ins.type)
					{
					case AstInsType::CreateObject:
						{
							CHECK_ERROR(context.createStack != nullref, ERROR_MESSAGE_PREFIX L"[CreateObject] context.createStack is empty.");
							ForEachStack(context.createStack, [=](InsExec_Stack* topStack)
							{
								PushInsRefLink(topStack->createObjectInsRefs, { trace, insRef });
								PushStackRefLink(insExec->operatingStacks, topStack);
							});
						}
						break;
					case AstInsType::StackBegin:
						{
							auto newTopStack = NewStack();
							PushStackArrayRefLink(context.createStack, newTopStack);
							newTopStack->beginInsRef = { trace, insRef };
							PushStackRefLink(insExec->operatingStacks, newTopStack);

							auto newStackTop = GetInsExec_StackArrayRefLink(context.createStack);
							if (context.objectStack == nullref)
							{
								newStackTop->objectStackDepthForCreateStack = 0;
							}
							else
							{
								newStackTop->objectStackDepthForCreateStack = GetInsExec_StackArrayRefLink(context.objectStack)->currentDepth;
							}
						}
						break;
					case AstInsType::StackEnd:
						{
							CHECK_ERROR(context.createStack != nullref, ERROR_MESSAGE_PREFIX L"[StackEnd] context.createStack is empty.");
							bool endWithCreate = false;
							bool endWithReuse = false;
							ForEachStack(context.createStack, [&](InsExec_Stack* topStack)
							{
								bool executedCreateObject = false;
								if (topStack->createObjectInsRefs != nullref)
								{
									auto lastCreateObjectInsRefLink = GetInsExec_InsRefLink(topStack->createObjectInsRefs);
									executedCreateObject = lastCreateObjectInsRefLink->insRef.trace == trace;
								}

								if(executedCreateObject)
								{
									endWithCreate = true;
									PushInsRefLink(topStack->endWithCreateInsRefs, { trace,  insRef });
								}
								else
								{
									endWithReuse = true;
									PushInsRefLink(topStack->endWithReuseInsRefs, { trace, insRef });
								}

								PushStackRefLink(insExec->operatingStacks, topStack);
							});
							CHECK_ERROR(endWithCreate ^ endWithReuse, ERROR_MESSAGE_PREFIX L"[StackEnd] Connected CreateObject and StackEnd should always be in the same trace.");

							if (endWithReuse)
							{
								CHECK_ERROR(context.objectStack != nullref, ERROR_MESSAGE_PREFIX L"[StackEnd] context.objectStack is empty.");
								auto topObjects = GetInsExec_StackArrayRefLink(context.objectStack);
								ForEachStack(context.createStack, [&](InsExec_Stack* topStack)
								{
									topStack->useFromStacks = JoinStackRefLink(topStack->useFromStacks, topObjects->ids);
								});
							}

							auto topStacks = GetInsExec_StackArrayRefLink(context.createStack);
							context.createStack = topStacks->previous;
							PushStackArrayRefLink(context.objectStack, topStacks->ids);
						}
						break;
					case AstInsType::StackSlot:
						{
							CHECK_ERROR(context.createStack != nullref, ERROR_MESSAGE_PREFIX L"[StackSlot] context.createStack is empty.");
							CHECK_ERROR(context.objectStack != nullref, ERROR_MESSAGE_PREFIX L"[StackSlot] context.objectStack is empty.");
							auto topObjects = GetInsExec_StackArrayRefLink(context.objectStack);
							context.objectStack = topObjects->previous;
							ForEachStack(context.createStack, [&](InsExec_Stack* topStack)
							{
								topStack->fieldStacks = JoinStackRefLink(topStack->fieldStacks, topObjects->ids);
							});
						}
						break;
					case AstInsType::Field:
					case AstInsType::FieldIfUnassigned:
					case AstInsType::Token:
					case AstInsType::EnumItem:
						break;
					case AstInsType::ResolveAmbiguity:
						CHECK_FAIL(ERROR_MESSAGE_PREFIX L"ResolveAmbiguity should not appear in traces.");
					default:;
						CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognizabled instruction.");
					}
				}
				traceExec->context = context;
#undef ERROR_MESSAGE_PREFIX
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}