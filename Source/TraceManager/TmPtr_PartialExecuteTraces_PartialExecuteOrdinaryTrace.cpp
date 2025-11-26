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

			InsExec_Object* TraceManager::NewObject()
			{
				auto ieObject = GetInsExec_Object(insExec_Objects.Allocate());
				ieObject->previous = firstObject;
				firstObject = ieObject;
				return ieObject;
			}

			vint32_t TraceManager::GetStackBase(InsExec_Context& context)
			{
				if (context.createStack == nullref)
				{
					return 0;
				}
				else
				{
					return GetInsExec_CreateStack(context.createStack)->stackBase;
				}
			}

			vint32_t TraceManager::GetStackTop(InsExec_Context& context)
			{
				if (context.objectStack == nullref)
				{
					return 0;
				}
				else
				{
					return GetInsExec_ObjectStack(context.objectStack)->pushedCount;
				}
			}

			void TraceManager::PushInsRefLink(Ref<InsExec_InsRefLink>& link, InsRef insRef)
			{
				auto newLink = GetInsExec_InsRefLink(insExec_InsRefLinks.Allocate());
				newLink->previous = link;
				newLink->insRef = insRef;
				link = newLink;
			}

			void TraceManager::PushObjRefLink(Ref<InsExec_ObjRefLink>& link, Ref<InsExec_Object> id)
			{
				auto newLink = GetInsExec_ObjRefLink(insExec_ObjRefLinks.Allocate());
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

			Ref<InsExec_ObjRefLink> TraceManager::JoinObjRefLink(Ref<InsExec_ObjRefLink> first, Ref<InsExec_ObjRefLink> second)
			{
				if (first == nullref) return second;
				if (second == nullref) return first;

				Ref<InsExec_ObjRefLink> newStack;

				while (first != nullref)
				{
					auto stack = GetInsExec_ObjRefLink(first);
					first = stack->previous;
					PushObjRefLink(newStack, stack->id);
				}

				while (second != nullref)
				{
					auto stack = GetInsExec_ObjRefLink(second);
					second = stack->previous;
					PushObjRefLink(newStack, stack->id);
				}

				return newStack;
			}

			void TraceManager::PushAssignedToObjectIdsSingleWithMagic(Ref<InsExec_ObjRefLink> fieldObjectIds, Ref<InsExec_Object> assignedToTarget)
			{
				NEW_MERGE_STACK_MAGIC_COUNTER;
				auto magicFieldObject = MergeStack_MagicCounter;

				auto linkRef = fieldObjectIds;
				while (linkRef != nullref)
				{
					auto link = GetInsExec_ObjRefLink(linkRef);
					linkRef = link->previous;

					if (link->id.handle == InsExec_Object::TokenOrEnumItemObjectId)
					{
						continue;
					}
					auto ieFieldObject = GetInsExec_Object(link->id);
					if (ieFieldObject->mergeCounter == magicFieldObject) continue;
					ieFieldObject->mergeCounter = magicFieldObject;
					PushObjRefLink(ieFieldObject->assignedToObjectIds, assignedToTarget);
				}
			}

			void TraceManager::PushAssignedToObjectIdsMultipleWithMagic(Ref<InsExec_ObjRefLink> fieldObjectIds, Ref<InsExec_ObjRefLink> assignedToTarget)
			{
				NEW_MERGE_STACK_MAGIC_COUNTER;
				auto magicElement = MergeStack_MagicCounter;

				auto linkRef = assignedToTarget;
				while (linkRef != nullref)
				{
					auto link = GetInsExec_ObjRefLink(linkRef);
					linkRef = link->previous;

					auto ieAssignedToObject = GetInsExec_Object(link->id);
					if (ieAssignedToObject->mergeCounter == magicElement) return;
					ieAssignedToObject->mergeCounter = magicElement;

					PushAssignedToObjectIdsSingleWithMagic(fieldObjectIds, link->id);
				}
			}

			InsExec_ObjectStack* TraceManager::PushObjectStackSingle(InsExec_Context& context, Ref<InsExec_Object> objectId)
			{
				auto ie = GetInsExec_ObjectStack(insExec_ObjectStacks.Allocate());
				ie->previous = context.objectStack;
				PushObjRefLink(ie->objectIds, objectId);
				ie->pushedCount = GetStackTop(context) + 1;
				context.objectStack = ie;
				return ie;
			}

			InsExec_ObjectStack* TraceManager::PushObjectStackMultiple(InsExec_Context& context, Ref<InsExec_ObjRefLink> linkId)
			{
				auto ie = GetInsExec_ObjectStack(insExec_ObjectStacks.Allocate());
				ie->previous = context.objectStack;
				ie->objectIds = JoinObjRefLink(ie->objectIds, linkId);
				ie->pushedCount = GetStackTop(context) + 1;
				context.objectStack = ie;
				return ie;
			}

			InsExec_CreateStack* TraceManager::PushCreateStack(InsExec_Context& context)
			{
				auto ie = GetInsExec_CreateStack(insExec_CreateStacks.Allocate());
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