#include "TraceManager.h"
#include "TraceManager_Common.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;

#define NEW_MERGE_STACK_MAGIC_COUNTER (void)(MergeStack_MagicCounter++)

/***********************************************************************
PartialExecuteOrdinaryTrace
***********************************************************************/

			InsExec_Object* TraceManager::NewObject()
			{
				return GetInsExec_Object(insExec_Objects.Allocate());
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

			void TraceManager::PushInsRefLink(Ref<InsExec_InsRefLink>& link, Ref<Trace> trace, vint32_t ins)
			{
				auto newLink = GetInsExec_InsRefLink(insExec_InsRefLinks.Allocate());
				newLink->previous = link;
				newLink->trace = trace;
				newLink->ins = ins;
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
					PushInsRefLink(newStack, stack->trace, stack->ins);
				}

				while (second != nullref)
				{
					auto stack = GetInsExec_InsRefLink(second);
					second = stack->previous;
					PushInsRefLink(newStack, stack->trace, stack->ins);
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
				for (vint32_t insRef = 0; insRef < traceExec->insLists.c3; insRef++)
				{
					auto&& ins = ReadInstruction(insRef, traceExec->insLists);
					auto insExec = GetInsExec(traceExec->insExecRefs.start + insRef);
					insExec->contextBeforeExecution = context;

					switch (ins.type)
					{
					case AstInsType::BeginObject:
						{
							// new object
							auto ieObject = NewObject();
							ieObject->bo_bolr_Trace = trace;
							ieObject->bo_bolr_Ins = insRef;

							// new create stack
							auto ieCSTop = PushCreateStack(context);
							PushInsRefLink(ieCSTop->createInsRefs, trace, insRef);
							ieCSTop->stackBase = GetStackTop(context);
							PushObjRefLink(ieCSTop->objectIds, ieObject);

							// InsExec::createdObjectId
							insExec->createdObjectId = ieObject;
						}
						break;
					case AstInsType::BeginObjectLeftRecursive:
						{
							CHECK_ERROR(GetStackTop(context) - GetStackBase(context) >= 1, ERROR_MESSAGE_PREFIX L"Pushed values not enough.");

							// observe the object stack top
							auto ieOSTop = GetInsExec_ObjectStack(context.objectStack);

							// new object
							auto ieObject = NewObject();
							ieObject->lrObjectIds = ieOSTop->objectIds;
							ieObject->bo_bolr_Trace = trace;
							ieObject->bo_bolr_Ins = insRef;

							// new create stack, the top object is not frozen
							auto ieCSTop = PushCreateStack(context);
							PushInsRefLink(ieCSTop->createInsRefs, trace, insRef);
							ieCSTop->stackBase = ieOSTop->pushedCount - 1;
							PushObjRefLink(ieCSTop->objectIds, ieObject);

							// InsExec::createdObjectId
							insExec->createdObjectId = ieObject;
						}
						break;
					case AstInsType::DelayFieldAssignment:
						{
							// new create stack
							auto ieCSTop = PushCreateStack(context);
							PushInsRefLink(ieCSTop->createInsRefs, trace, insRef);
							ieCSTop->stackBase = GetStackTop(context);
						}
						break;
					case AstInsType::ReopenObject:
						{
							CHECK_ERROR(GetStackTop(context) - GetStackBase(context) >= 1, ERROR_MESSAGE_PREFIX L"Pushed values not enough.");
							CHECK_ERROR(context.createStack != nullref, ERROR_MESSAGE_PREFIX L"There is no created object.");

							// pop an object
							auto ieOSTop = GetInsExec_ObjectStack(context.objectStack);
							context.objectStack = ieOSTop->previous;

							// reopen an object
							// ReopenObject in different branches could write to the same InsExec_CreateStack
							// this happens when ambiguity happens in the !Rule syntax
							// but the same InsExec_CreateStack means the clause of !Rule does not have ambiguity
							// so ambiguity should also be resolved here
							// and such ReopenObject will be the last instruction in a trace
							// this means it is impossible to continue with InsExec_CreateStack polluted by sibling traces
							// therefore adding multiple objects to the same InsExec_CreateStack in multiple branches is fine
							// the successor trace will be a merge trace taking all of the information
							auto ieCSTop = GetInsExec_CreateStack(context.createStack);
							NEW_MERGE_STACK_MAGIC_COUNTER;
							{
								auto ref = ieCSTop->objectIds;
								while (ref != nullref)
								{
									auto link = GetInsExec_ObjRefLink(ref);
									auto ieObject = GetInsExec_Object(link->id);
									ieObject->mergeCounter = MergeStack_MagicCounter;
									ref = link->previous;
								}
							}
							{
								auto ref = ieOSTop->objectIds;
								while (ref != nullref)
								{
									auto link = GetInsExec_ObjRefLink(ref);
									auto ieObject = GetInsExec_Object(link->id);
									if (ieObject->mergeCounter != MergeStack_MagicCounter)
									{
										ieObject->mergeCounter = MergeStack_MagicCounter;
										PushObjRefLink(ieCSTop->objectIds, link->id);
									}
									ref = link->previous;
								}
							}

							auto insRefLinkId = ieCSTop->createInsRefs;
							while(insRefLinkId != nullref)
							{
								auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
								insRefLinkId = insRefLink->previous;

								// check if the top create stack is from DFA
								auto traceCSTop = GetTrace(insRefLink->trace);
								auto traceExecCSTop = GetTraceExec(traceCSTop->traceExecRef);
								CHECK_ERROR(ReadInstruction(insRefLink->ins, traceExecCSTop->insLists).type == AstInsType::DelayFieldAssignment, ERROR_MESSAGE_PREFIX L"DelayFieldAssignment is not submitted before ReopenObject.");

								auto insExecDfa = GetInsExec(traceExecCSTop->insExecRefs.start + insRefLink->ins);
								auto ref = ieOSTop->objectIds;
								while (ref != nullref)
								{
									auto link = GetInsExec_ObjRefLink(ref);
									auto ieObject = GetInsExec_Object(link->id);
									// InsExec_Object::dfaInsRefs
									PushInsRefLink(ieObject->dfaInsRefs, insRefLink->trace, insRefLink->ins);
									// InsExec::objRefs
									PushObjRefLink(insExecDfa->objRefs, ieObject);
									ref = link->previous;
								}
							}
						}
						break;
					case AstInsType::EndObject:
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

								auto traceCSTop = GetTrace(insRefLink->trace);
								auto traceExecCSTop = GetTraceExec(traceCSTop->traceExecRef);
								auto insExecCreate = GetInsExec(traceExecCSTop->insExecRefs.start + insRefLink->ins);
								PushInsRefLink(insExecCreate->eoInsRefs, trace, insRef);
							}
						}
						break;
					case AstInsType::DiscardValue:
					case AstInsType::Field:
					case AstInsType::FieldIfUnassigned:
						{
							CHECK_ERROR(GetStackTop(context) - GetStackBase(context) >= 1, ERROR_MESSAGE_PREFIX L"Pushed values not enough.");

							auto ieObjTop = GetInsExec_ObjectStack(context.objectStack);
							context.objectStack = ieObjTop->previous;
						}
						break;
					case AstInsType::LriStore:
						{
							CHECK_ERROR(GetStackTop(context) - GetStackBase(context) >= 1, ERROR_MESSAGE_PREFIX L"Pushed values not enough.");
							CHECK_ERROR(context.lriStoredObjects == nullref, ERROR_MESSAGE_PREFIX L"LriFetch is not executed before the next LriStore.");

							auto ieObjTop = GetInsExec_ObjectStack(context.objectStack);
							context.objectStack = ieObjTop->previous;
							context.lriStoredObjects = ieObjTop->objectIds;
						}
						break;
					case AstInsType::LriFetch:
						{
							CHECK_ERROR(context.lriStoredObjects != nullref, ERROR_MESSAGE_PREFIX L"LriStore is not executed before the next LriFetch.");
							PushObjectStackMultiple(context, context.lriStoredObjects);
							context.lriStoredObjects = nullref;
						}
						break;
					case AstInsType::Token:
					case AstInsType::EnumItem:
						{
							PushObjectStackSingle(context, Ref<InsExec_Object>(-2));
						}
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

/***********************************************************************
EnsureInsExecContextCompatible
***********************************************************************/

			void TraceManager::EnsureInsExecContextCompatible(Trace* baselineTrace, Trace* commingTrace)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::EnsureInsExecContextCompatible(Trace*, Trace*)#"
				auto&& contextComming = GetTraceExec(baselineTrace->traceExecRef)->context;
				auto&& contextBaseline = GetTraceExec(commingTrace->traceExecRef)->context;
				auto error = []()
				{
					CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Execution results of traces to merge are different.");
				};

				// check if the two lriStored be both empty or non-empty
				if ((contextBaseline.lriStoredObjects == nullref) != (contextComming.lriStoredObjects == nullref)) error();

				// check if the two objectStack have the same depth
				if ((contextBaseline.objectStack == nullref) != (contextComming.objectStack == nullref)) error();
				if (contextBaseline.objectStack != nullref)
				{
					auto stackBaseline = GetInsExec_ObjectStack(contextBaseline.objectStack);
					auto stackComming = GetInsExec_ObjectStack(contextComming.objectStack);
					if (stackBaseline->pushedCount != stackComming->pushedCount) error();
				}

				// check if the two createStack have the same depth
				// check each corresponding createStack have the same stackBase
				auto stack1 = contextBaseline.createStack;
				auto stack2 = contextComming.createStack;
				while (stack1 != stack2)
				{
					if (stack1 == nullref || stack2 == nullref) error();

					auto stackObj1 = GetInsExec_CreateStack(stack1);
					auto stackObj2 = GetInsExec_CreateStack(stack2);

					if (stackObj1->stackBase != stackObj2->stackBase) error();

					stack1 = stackObj1->previous;
					stack2 = stackObj2->previous;
				}
#undef ERROR_MESSAGE_PREFIX
			}

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

/***********************************************************************
PartialExecuteTraces
***********************************************************************/

			void TraceManager::PartialExecuteTraces()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::PartialExecuteTraces()#"
				IterateSurvivedTraces(
					[this](Trace* trace, Trace* predecessor, vint32_t visitCount, vint32_t predecessorCount)
					{
						if (predecessorCount <= 1)
						{
							PartialExecuteOrdinaryTrace(trace);
						}
						else
						{
							if (visitCount > 1)
							{
								EnsureInsExecContextCompatible(predecessor, GetTrace(trace->predecessors.first));
							}

							if (visitCount == predecessorCount)
							{
								MergeInsExecContext(trace);
							}
						}
					}
				);
#undef ERROR_MESSAGE_PREFIX
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}