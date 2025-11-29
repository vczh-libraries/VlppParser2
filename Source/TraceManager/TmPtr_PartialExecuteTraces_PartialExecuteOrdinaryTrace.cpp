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

			Ref<InsExec_InsRefLink> TraceManager::JoinInsRefLink(Ref<InsExec_InsRefLink> first, Ref<InsExec_InsRefLink> second)
			{
				if (first == nullref) return second;
				if (second == nullref) return first;

				Ref<InsExec_InsRefLink> newStack;

				while (first != nullref)
				{
					auto stack = GetInsExec_InsRefLink(first);
					first = stack->previous;
					PushInsRefLink(newStack, stack->insRef);
				}

				while (second != nullref)
				{
					auto stack = GetInsExec_InsRefLink(second);
					second = stack->previous;
					PushInsRefLink(newStack, stack->insRef);
				}

				return newStack;
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

			void TraceManager::PushOwnerStack_WithNewMagicCounter(Ref<InsExec_StackRefLink> fieldStacks, Ref<InsExec_Stack> ownerStack)
			{
				NEW_MERGE_STACK_MAGIC_COUNTER;
				auto magicFieldObject = MergeStack_MagicCounter;

				auto linkRef = fieldStacks;
				while (linkRef != nullref)
				{
					auto link = GetInsExec_StackRefLink(linkRef);
					linkRef = link->previous;

					auto ieFieldObject = GetInsExec_Stack(link->id);
					if (ieFieldObject->mergeCounter == magicFieldObject) continue;

					ieFieldObject->mergeCounter = magicFieldObject;
					PushStackRefLink(ieFieldObject->ownerStacks, ownerStack);
				}
			}

			void TraceManager::PushOwnerStackMultiple_WithNewMagicCounter(Ref<InsExec_StackRefLink> fieldStacks, Ref<InsExec_StackRefLink> ownerStacks)
			{
				NEW_MERGE_STACK_MAGIC_COUNTER;
				auto magicElement = MergeStack_MagicCounter;

				auto linkRef = ownerStacks;
				while (linkRef != nullref)
				{
					auto link = GetInsExec_StackRefLink(linkRef);
					linkRef = link->previous;

					auto ieAssignedToObject = GetInsExec_Stack(link->id);
					if (ieAssignedToObject->mergeCounter == magicElement) return;
					ieAssignedToObject->mergeCounter = magicElement;

					PushOwnerStack_WithNewMagicCounter(ieAssignedToObject->ownerStacks, link->id);
				}
			}

			InsExec_StackArrayRefLink* TraceManager::PushObjectStack(InsExec_Context& context, Ref<InsExec_StackRefLink> linkId)
			{
				auto ie = GetInsExec_StackArrayRefLink(insExec_StackArrayRefLinks.Allocate());
				ie->previous = context.objectStack;
				ie->ids = JoinStackRefLink(ie->ids, linkId);
				context.objectStack = ie;
				return ie;
			}

			InsExec_StackRefLink* TraceManager::PushCreateStack(InsExec_Context& context)
			{
				auto ie = GetInsExec_StackRefLink(insExec_StackRefLinks.Allocate());
				ie->previous = context.createStack;
				context.createStack = ie;
				return ie;
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
							// new object
							auto ieObject = NewObject();
							ieObject->createInsRef = { trace,insRef };

							// associate to current create stack
							auto ieCSTop = GetInsExec_CreateStack(context.createStack);
							PushObjRefLink(ieCSTop->objectIds, ieObject);

							// InsExec::createdObjectId
							insExec->createdObjectId = ieObject;
						}
						break;
					case AstInsType::StackBegin:
						{
							// new create stack
							auto ieCSTop = PushCreateStack(context);
							PushInsRefLink(ieCSTop->createInsRefs, { trace, insRef });
							ieCSTop->stackBase = GetStackTop(context);
						}
						break;
					case AstInsType::StackEnd:
						{
							CHECK_ERROR(context.createStack != nullref, ERROR_MESSAGE_PREFIX L"There is no created object.");

							// pop a create stack
							auto ieCSTop = GetInsExec_CreateStack(context.createStack);
							context.createStack = ieCSTop->previous;

							// push an object
							CHECK_ERROR(ieCSTop->objectIds != nullref, ERROR_MESSAGE_PREFIX L"An object has not been associated to the create stack yet.");
							PushObjectStackMultiple(context, ieCSTop->objectIds);

							// InsExec::objRefs
							insExec->objRefs = ieCSTop->objectIds;

							// InsExec::eoInsRefs
							auto insRefLinkId = ieCSTop->createInsRefs;
							while (insRefLinkId != nullref)
							{
								auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
								insRefLinkId = insRefLink->previous;

								auto traceCSTop = GetTrace(insRefLink->insRef.trace);
								auto traceExecCSTop = GetTraceExec(traceCSTop->traceExecRef);
								auto insExecCreate = GetInsExec(traceExecCSTop->insExecRefs.start + insRefLink->insRef.ins);
								PushInsRefLink(insExecCreate->eoInsRefs, { trace, insRef });
							}
						}
						break;
					case AstInsType::StackSlot:
						{
							CHECK_ERROR(GetStackTop(context) - GetStackBase(context) >= 1, ERROR_MESSAGE_PREFIX L"Pushed values not enough.");

							auto ieObjTop = GetInsExec_ObjectStack(context.objectStack);
							context.objectStack = ieObjTop->previous;

							// InsExec_Object::assignedToObjectIds
							if (context.createStack != nullref)
							{
								auto ieCSTop = GetInsExec_CreateStack(context.createStack);
								if (ieCSTop->objectIds == nullref)
								{
									ieCSTop->reverseAssignedToObjectIds = JoinObjRefLink(ieCSTop->reverseAssignedToObjectIds, ieObjTop->objectIds);
								}
								else
								{
									PushAssignedToObjectIdsMultipleWithMagic(ieObjTop->objectIds, ieCSTop->objectIds);
								}
							}
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