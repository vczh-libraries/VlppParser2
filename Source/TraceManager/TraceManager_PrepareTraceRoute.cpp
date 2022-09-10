#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;

			vuint64_t MergeStack_MagicCounter = 0;
#define NEW_MERGE_STACK_MAGIC_COUNTER (void)(MergeStack_MagicCounter++)

/***********************************************************************
IterateSurvivedTraces
***********************************************************************/

			template<typename TCallback>
			void TraceManager::IterateSurvivedTraces(TCallback&& callback)
			{
				Trace* lastTrace = nullptr;
				List<Trace*> traces;
				traces.Add(initialTrace);

				while (traces.Count() > 0)
				{
					auto current = traces[traces.Count() - 1];
					traces.RemoveAt(traces.Count() - 1);

					if (current->iterateCounter == current->predecessorCount)
					{
						current->iterateCounter = 0;
					}

					current->iterateCounter++;
					callback(
						current,
						(
							current->predecessorCount == 0 ? nullptr :
							current->predecessorCount == 1 ? GetTrace(current->predecessors.first) :
							lastTrace
						),
						current->iterateCounter,
						current->predecessorCount
						);
					lastTrace = current;
					if (current->iterateCounter < current->predecessorCount) continue;

					vint32_t successorId = current->successors.last;
					while (successorId != -1)
					{
						auto successor = GetTrace(successorId);
						successorId = successor->successors.siblingPrev;
						traces.Add(successor);
					}
				}
			}

/***********************************************************************
ReadInstructionList
***********************************************************************/

			void TraceManager::ReadInstructionList(Trace* trace, TraceInsLists& insLists)
			{
				// this function collects the following instructions in order:
				//   1) byEdge.insBeforeInput
				//   2) byEdge.insAfterInput
				//   3) executedReturnStack.returnIndex.insAfterInput in order
				if (trace->byEdge != -1)
				{
					auto& edgeDesc = executable.edges[trace->byEdge];
					insLists.edgeInsBeforeInput = edgeDesc.insBeforeInput;
					insLists.edgeInsAfterInput = edgeDesc.insAfterInput;
				}
				else
				{
					insLists.edgeInsBeforeInput = {};
					insLists.edgeInsAfterInput = {};
				}
				if (trace->executedReturnStack != -1)
				{
					auto returnStack = GetReturnStack(trace->executedReturnStack);
					auto& returnDesc = executable.returns[returnStack->returnIndex];
					insLists.returnInsAfterInput = returnDesc.insAfterInput;
				}
				else
				{
					insLists.returnInsAfterInput = {};
				}

				insLists.c1 = (vint32_t)(insLists.edgeInsBeforeInput.count);
				insLists.c2 = (vint32_t)(insLists.c1 + insLists.edgeInsAfterInput.count);
				insLists.c3 = (vint32_t)(insLists.c2 + insLists.returnInsAfterInput.count);
			}

/***********************************************************************
ReadInstruction
***********************************************************************/

			AstIns& TraceManager::ReadInstruction(vint32_t instruction, TraceInsLists& insLists)
			{
				// access the instruction object from a trace
				// the index is the instruction in a virtual instruction array
				// defined by all InstructionArray in TraceInsLists combined together
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::ReadInstruction(vint, TraceInsLists&)#"
				CHECK_ERROR(0 <= instruction && instruction < insLists.c3, ERROR_MESSAGE_PREFIX L"Instruction index out of range.");

				vint insRef = -1;
				if (instruction < insLists.c1)
				{
					insRef = insLists.edgeInsBeforeInput.start + instruction;
				}
				else if (instruction < insLists.c2)
				{
					insRef = insLists.edgeInsAfterInput.start + (instruction - insLists.c1);
				}
				else if (instruction < insLists.c3)
				{
					insRef = insLists.returnInsAfterInput.start + (instruction - insLists.c2);
				}
				else
				{
					CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Instruction index out of range.");
				}

				return executable.astInstructions[insRef];
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
AllocateExecutionData
***********************************************************************/

			void TraceManager::AllocateExecutionData()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::AllocateExecutionData()#"
				vint32_t insExecCount = 0;
				IterateSurvivedTraces([&](Trace* trace, Trace* predecessor, vint32_t visitCount, vint32_t predecessorCount)
				{
					// ensure traceExecRef reflects the partial order of the execution order of traces
					if (predecessorCount > 1 && visitCount != predecessorCount) return;

					CHECK_ERROR(trace->traceExecRef == -1, ERROR_MESSAGE_PREFIX L"Internal error: IterateSurvivedTraces unexpectedly revisit a trace.");
					trace->traceExecRef = traceExecs.Allocate();

					auto traceExec = GetTraceExec(trace->traceExecRef);
					traceExec->traceId = trace->allocatedIndex;
					ReadInstructionList(trace, traceExec->insLists);
					if (traceExec->insLists.c3 > 0)
					{
						traceExec->insExecRefs.start = insExecCount;
						traceExec->insExecRefs.count = traceExec->insLists.c3;
						insExecCount += traceExec->insLists.c3;
					}

					if (trace->predecessors.first != trace->predecessors.last)
					{
						if (firstMergeTrace == -1)
						{
							firstMergeTrace = trace->allocatedIndex;
							lastMergeTrace = trace->allocatedIndex;
						}
						else
						{
							auto lastTraceExec = GetTraceExec(GetTrace(lastMergeTrace)->traceExecRef);
							lastTraceExec->nextMergeTrace = trace->allocatedIndex;
							traceExec->previousMergeTrace = lastMergeTrace;
							lastMergeTrace = trace->allocatedIndex;
						}
					}
				});
				insExecs.Resize(insExecCount);
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
PartialExecuteOrdinaryTrace
***********************************************************************/

			InsExec_Object* TraceManager::NewObject()
			{
				return GetInsExec_Object(insExec_Objects.Allocate());
			}

			vint32_t TraceManager::GetStackBase(InsExec_Context& context)
			{
				if (context.createStack == -1)
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
				if (context.objectStack == -1)
				{
					return 0;
				}
				else
				{
					return GetInsExec_ObjectStack(context.objectStack)->pushedCount;
				}
			}

			void TraceManager::PushInsRefLink(vint32_t& link, vint32_t trace, vint32_t ins)
			{
				auto newLink = GetInsExec_InsRefLink(insExec_InsRefLinks.Allocate());
				newLink->previous = link;
				newLink->trace = trace;
				newLink->ins = ins;
				link = newLink->allocatedIndex;
			}

			void TraceManager::PushObjRefLink(vint32_t& link, vint32_t id)
			{
				auto newLink = GetInsExec_ObjRefLink(insExec_ObjRefLinks.Allocate());
				newLink->previous = link;
				newLink->id = id;
				link = newLink->allocatedIndex;
			}

			vint32_t TraceManager::JoinInsRefLink(vint32_t first, vint32_t second)
			{
				if (first == -1) return second;
				if (second == -1) return first;

				vint32_t newStack = -1;

				while (first != -1)
				{
					auto stack = GetInsExec_InsRefLink(first);
					first = stack->previous;
					PushInsRefLink(newStack, stack->trace, stack->ins);
				}

				while (second != -1)
				{
					auto stack = GetInsExec_InsRefLink(second);
					second = stack->previous;
					PushInsRefLink(newStack, stack->trace, stack->ins);
				}

				return newStack;
			}

			vint32_t TraceManager::JoinObjRefLink(vint32_t first, vint32_t second)
			{
				if (first == -1) return second;
				if (second == -1) return first;

				vint32_t newStack = -1;

				while (first != -1)
				{
					auto stack = GetInsExec_ObjRefLink(first);
					first = stack->previous;
					PushObjRefLink(newStack, stack->id);
				}

				while (second != -1)
				{
					auto stack = GetInsExec_ObjRefLink(second);
					second = stack->previous;
					PushObjRefLink(newStack, stack->id);
				}

				return newStack;
			}

			InsExec_ObjectStack* TraceManager::PushObjectStackSingle(InsExec_Context& context, vint32_t objectId)
			{
				auto ie = GetInsExec_ObjectStack(insExec_ObjectStacks.Allocate());
				ie->previous = context.objectStack;
				PushObjRefLink(ie->objectIds, objectId);
				ie->pushedCount = GetStackTop(context) + 1;
				context.objectStack = ie->allocatedIndex;
				return ie;
			}

			InsExec_ObjectStack* TraceManager::PushObjectStackMultiple(InsExec_Context& context, vint32_t linkId)
			{
				auto ie = GetInsExec_ObjectStack(insExec_ObjectStacks.Allocate());
				ie->previous = context.objectStack;
				ie->objectIds = JoinObjRefLink(ie->objectIds, linkId);
				ie->pushedCount = GetStackTop(context) + 1;
				context.objectStack = ie->allocatedIndex;
				return ie;
			}

			InsExec_CreateStack* TraceManager::PushCreateStack(InsExec_Context& context)
			{
				auto ie = GetInsExec_CreateStack(insExec_CreateStacks.Allocate());
				ie->previous = context.createStack;
				context.createStack = ie->allocatedIndex;
				return ie;
			}

			void TraceManager::PartialExecuteOrdinaryTrace(Trace* trace)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::PartialExecuteOrdinaryTrace(Trace*)#"
				InsExec_Context context;
				if (trace->predecessors.first != -1)
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
							ieObject->bo_bolr_Trace = trace->allocatedIndex;
							ieObject->bo_bolr_Ins = insRef;

							// new create stack
							auto ieCSTop = PushCreateStack(context);
							PushInsRefLink(ieCSTop->createInsRefs, trace->allocatedIndex, insRef);
							ieCSTop->stackBase = GetStackTop(context);
							PushObjRefLink(ieCSTop->objectIds, ieObject->allocatedIndex);

							// InsExec::createdObjectId
							insExec->createdObjectId = ieObject->allocatedIndex;
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
							ieObject->bo_bolr_Trace = trace->allocatedIndex;
							ieObject->bo_bolr_Ins = insRef;

							// new create stack, the top object is not frozen
							auto ieCSTop = PushCreateStack(context);
							PushInsRefLink(ieCSTop->createInsRefs, trace->allocatedIndex, insRef);
							ieCSTop->stackBase = ieOSTop->pushedCount - 1;
							PushObjRefLink(ieCSTop->objectIds, ieObject->allocatedIndex);

							// InsExec::createdObjectId
							insExec->createdObjectId = ieObject->allocatedIndex;
						}
						break;
					case AstInsType::DelayFieldAssignment:
						{
							// new create stack
							auto ieCSTop = PushCreateStack(context);
							PushInsRefLink(ieCSTop->createInsRefs, trace->allocatedIndex, insRef);
							ieCSTop->stackBase = GetStackTop(context);
						}
						break;
					case AstInsType::ReopenObject:
						{
							CHECK_ERROR(GetStackTop(context) - GetStackBase(context) >= 1, ERROR_MESSAGE_PREFIX L"Pushed values not enough.");
							CHECK_ERROR(context.createStack != -1, ERROR_MESSAGE_PREFIX L"There is no created object.");

							// pop an object
							auto ieOSTop = GetInsExec_ObjectStack(context.objectStack);
							context.objectStack = ieOSTop->previous;

							// reopen an object
							auto ieCSTop = GetInsExec_CreateStack(context.createStack);
							{
								auto ref = ieOSTop->objectIds;
								while (ref != -1)
								{
									auto link = GetInsExec_ObjRefLink(ref);
									PushObjRefLink(ieCSTop->objectIds, link->id);
									ref = link->previous;
								}
							}

							vint32_t insRefLinkId = ieCSTop->createInsRefs;
							while(insRefLinkId!=-1)
							{
								auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
								insRefLinkId = insRefLink->previous;

								// check if the top create stack is from DFA
								auto traceCSTop = GetTrace(insRefLink->trace);
								auto traceExecCSTop = GetTraceExec(traceCSTop->traceExecRef);
								CHECK_ERROR(ReadInstruction(insRefLink->ins, traceExecCSTop->insLists).type == AstInsType::DelayFieldAssignment, ERROR_MESSAGE_PREFIX L"DelayFieldAssignment is not submitted before ReopenObject.");

								auto insExecDfa = GetInsExec(traceExecCSTop->insExecRefs.start + insRefLink->ins);
								auto ref = ieOSTop->objectIds;
								while (ref != -1)
								{
									auto link = GetInsExec_ObjRefLink(ref);
									auto ieObject = GetInsExec_Object(link->id);
									// InsExec_Object::dfaInsRefs
									PushInsRefLink(ieObject->dfaInsRefs, insRefLink->trace, insRefLink->ins);
									// InsExec::objRefs
									PushObjRefLink(insExecDfa->objRefs, ieObject->allocatedIndex);
									ref = link->previous;
								}
							}
						}
						break;
					case AstInsType::EndObject:
						{
							CHECK_ERROR(context.createStack != -1, ERROR_MESSAGE_PREFIX L"There is no created object.");

							// pop a create stack
							auto ieCSTop = GetInsExec_CreateStack(context.createStack);
							context.createStack = ieCSTop->previous;

							// push an object
							CHECK_ERROR(ieCSTop->objectIds != -1, ERROR_MESSAGE_PREFIX L"An object has not been associated to the create stack yet.");
							PushObjectStackMultiple(context, ieCSTop->objectIds);

							// InsExec::objRefs
							insExec->objRefs = ieCSTop->objectIds;

							// InsExec::eoInsRefs
							vint32_t insRefLinkId = ieCSTop->createInsRefs;
							while (insRefLinkId != -1)
							{
								auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
								insRefLinkId = insRefLink->previous;

								auto traceCSTop = GetTrace(insRefLink->trace);
								auto traceExecCSTop = GetTraceExec(traceCSTop->traceExecRef);
								auto insExecCreate = GetInsExec(traceExecCSTop->insExecRefs.start + insRefLink->ins);
								PushInsRefLink(insExecCreate->eoInsRefs, trace->allocatedIndex, insRef);
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
							CHECK_ERROR(context.lriStoredObjects == -1, ERROR_MESSAGE_PREFIX L"LriFetch is not executed before the next LriStore.");

							auto ieObjTop = GetInsExec_ObjectStack(context.objectStack);
							context.objectStack = ieObjTop->previous;
							context.lriStoredObjects = ieObjTop->objectIds;
						}
						break;
					case AstInsType::LriFetch:
						{
							CHECK_ERROR(context.lriStoredObjects != -1, ERROR_MESSAGE_PREFIX L"LriStore is not executed before the next LriFetch.");
							PushObjectStackMultiple(context, context.lriStoredObjects);
							context.lriStoredObjects = -1;
						}
						break;
					case AstInsType::Token:
					case AstInsType::EnumItem:
						{
							PushObjectStackSingle(context, -2);
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
				if ((contextBaseline.lriStoredObjects != -1) != (contextComming.lriStoredObjects != -1)) error();

				// check if the two objectStack have the same depth
				if ((contextBaseline.objectStack == -1) != (contextComming.objectStack == -1)) error();
				if (contextBaseline.objectStack != -1)
				{
					auto stackBaseline = GetInsExec_ObjectStack(contextBaseline.objectStack);
					auto stackComming = GetInsExec_ObjectStack(contextComming.objectStack);
					if (stackBaseline->pushedCount != stackComming->pushedCount) error();
				}

				// check if the two createStack have the same depth
				// check each corresponding createStack have the same stackBase
				vint32_t stack1 = contextBaseline.createStack;
				vint32_t stack2 = contextComming.createStack;
				while (stack1 != stack2)
				{
					if (stack1 == -1 || stack2 == -1) error();

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

			void TraceManager::PushInsRefLinkWithCounter(vint32_t& link, vint32_t comming)
			{
				while (comming != -1)
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

			void TraceManager::PushObjRefLinkWithCounter(vint32_t& link, vint32_t comming)
			{
				while (comming != -1)
				{
					auto commingStack = GetInsExec_ObjRefLink(comming);
					comming = commingStack->previous;

					auto ieObject = GetInsExec_Object(commingStack->id);
					if (ieObject->mergeCounter == MergeStack_MagicCounter) continue;

					ieObject->mergeCounter = MergeStack_MagicCounter;
					PushObjRefLink(link, ieObject->allocatedIndex);
				}
			}

			template<typename T, T* (TraceManager::* get)(vint32_t), vint32_t(InsExec_Context::* stack), typename TMerge>
			vint32_t TraceManager::MergeStack(Trace* mergeTrace, AllocateOnly<T>& allocator, TMerge&& merge)
			{
				Array<T*> stacks(mergeTrace->predecessorCount);

				// fill the first level of stacks objects
				{
					vint index = 0;
					vint32_t predecessorId = mergeTrace->predecessors.first;
					while (predecessorId != -1)
					{
						auto predecessor = GetTrace(predecessorId);
						auto traceExec = GetTraceExec(predecessor->traceExecRef);

						vint32_t stackId = traceExec->context.*stack;
						stacks[index++] = stackId == -1 ? nullptr : (this->*get)(stackId);
						predecessorId = predecessor->predecessors.siblingNext;
					}
				}

				vint32_t stackTop = -1;
				vint32_t* pStackPrevious = &stackTop;
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
						*pStackPrevious = stacks[0]->allocatedIndex;
						break;
					}

					// otherwise, create a new stack object to merge all
					auto newStack = (this->*get)(allocator.Allocate());
					*pStackPrevious = newStack->allocatedIndex;
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
						vint32_t stackId = stacks[index]->previous;
						stacks[index] = stackId == -1 ? nullptr : (this->*get)(stackId);
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
				vint32_t predecessorId = mergeTrace->predecessors.first;
				while (predecessorId != -1)
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

/***********************************************************************
BuildAmbiguityStructures
***********************************************************************/

			void TraceManager::BuildAmbiguityStructures()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::BuildAmbiguityStructures()#"
				IterateSurvivedTraces(
					[this](Trace* trace, Trace* predecessor, vint32_t visitCount, vint32_t predecessorCount)
					{
						auto traceExec = GetTraceExec(trace->traceExecRef);
						if (predecessorCount == 0)
						{
							// initialize branch information for initialTrace
							traceExec->branchData = { trace->allocatedIndex,0 };
						}
						else if (predecessorCount == 1)
						{
							if (predecessor->successors.first!=predecessor->successors.last)
							{
								// if the current trace is a branch head
								traceExec->branchData.forwardTrace = trace->allocatedIndex;
								traceExec->branchData.branchDepth = GetTraceExec(predecessor->traceExecRef)->branchData.branchDepth + 1;
							}
							else
							{
								// if a predecessor is a merge trace
								// jump to its forwardTrace
								// until an ordinary trace is found
								while (predecessor->state == -1)
								{
									predecessor = GetTrace(GetTraceExec(predecessor->traceExecRef)->branchData.forwardTrace);
								}

								// copy its data
								auto predecessorTraceExec = GetTraceExec(predecessor->traceExecRef);
								traceExec->branchData = predecessorTraceExec->branchData;
							}
						}
						else
						{
							auto stepForward = [this](TraceBranchData branchData) -> TraceBranchData
							{
								auto branchTrace = GetTrace(branchData.forwardTrace);
								auto branchHeadTrace = GetTrace(GetTraceExec(branchTrace->traceExecRef)->branchData.forwardTrace);
								return { branchHeadTrace->predecessors.first,branchData.branchDepth - 1 };
							};

							CHECK_ERROR(predecessor->state != -1, ERROR_MESSAGE_PREFIX L"Predecessor trace of a merge trace cannot be a merge trace.");
							auto commingTraceExec = GetTraceExec(predecessor->traceExecRef);
							auto comming = commingTraceExec->branchData;

							if (visitCount == 1)
							{
								// for the first time visiting a merge trace, copy the data
								traceExec->branchData = comming;
							}
							else if (traceExec->branchData.forwardTrace != comming.forwardTrace)
							{
								// otherwise, use the data from the latest common shared node

								// closer and further are two TraceBranchData of this merge state
								auto closer = traceExec->branchData;
								auto further = comming;
								if (closer.branchDepth < further.branchDepth)
								{
									auto t = closer;
									closer = further;
									further = t;
								}

								// step closer forward until it has the same depth as further
								while (closer.branchDepth != further.branchDepth)
								{
									closer = stepForward(closer);
								}

								// step closer and further forward until they become the same
								while (closer.forwardTrace != further.forwardTrace)
								{
									closer = stepForward(closer);
									further = stepForward(further);
								}

								// the latest common shared node is found
								traceExec->branchData = further;
							}
						}
					}
				);
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
DebugCheckTraceExecData
***********************************************************************/

#if defined VCZH_MSVC && defined _DEBUG
			void TraceManager::DebugCheckTraceExecData()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::DebugCheckTraceExecData()#"
				IterateSurvivedTraces(
					[this](Trace* trace, Trace* predecessor, vint32_t visitCount, vint32_t predecessorCount)
					{
						if (predecessorCount <= 1)
						{
							auto traceExec = GetTraceExec(trace->traceExecRef);
							for (vint32_t insRef = 0; insRef < traceExec->insExecRefs.count; insRef++)
							{
								auto&& ins = ReadInstruction(insRef, traceExec->insLists);
								auto insExec = GetInsExec(traceExec->insExecRefs.start + insRef);

								// ensure BO/BOLR/DFA are closed
								switch (ins.type)
								{
								case AstInsType::BeginObject:
								case AstInsType::BeginObjectLeftRecursive:
								case AstInsType::DelayFieldAssignment:
									CHECK_ERROR(insExec->eoInsRefs != -1, ERROR_MESSAGE_PREFIX L"Internal error: BO/BOLA/DFA not closed.");
									break;
								}

								// ensure DFA are associated with objects closed
								switch (ins.type)
								{
								case AstInsType::DelayFieldAssignment:
									CHECK_ERROR(insExec->objRefs != -1, ERROR_MESSAGE_PREFIX L"Internal error: DFA not associated.");
									break;
								}
							}
						}
					}
				);
#undef ERROR_MESSAGE_PREFIX
			}
#endif

/***********************************************************************
PrepareTraceRoute
***********************************************************************/

			void TraceManager::PrepareTraceRoute()
			{
				CHECK_ERROR(state == TraceManagerState::Finished, L"vl::glr::automaton::TraceManager::PrepareTraceRoute()#Wrong timing to call this function.");
				state = TraceManagerState::PreparedTraceRoute;

				AllocateExecutionData();
				PartialExecuteTraces();
				BuildAmbiguityStructures();
#if defined VCZH_MSVC && defined _DEBUG
				DebugCheckTraceExecData();
#endif
			}

/***********************************************************************
CheckMergeTraces
***********************************************************************/

			template<typename TCallback>
			bool TraceManager::SearchForObjectsWithCounter(vint32_t objRefLinkStartSet, TCallback&& callback)
			{
				// check every object in the link
				vint32_t linkId = objRefLinkStartSet;
				while (linkId != -1)
				{
					auto objRefLink = GetInsExec_ObjRefLink(linkId);
					linkId = objRefLink->previous;
					auto ieObject = GetInsExec_Object(objRefLink->id);

					// skip if it has been searched
					if (ieObject->mergeCounter == MergeStack_MagicCounter) goto CHECK_NEXT_OBJECT;
					ieObject->mergeCounter = MergeStack_MagicCounter;

					if (!callback(ieObject)) return false;
				CHECK_NEXT_OBJECT:;
				}
				return true;
			}

			template<typename TCallback>
			bool TraceManager::SearchForTopObjectsWithCounter(vint32_t objRefLinkStartSet, collections::List<vint32_t>& visitingIds, TCallback&& callback)
			{
#define PUSH_ID(ID)													\
					do{												\
						if (availableIds == visitingIds.Count())	\
							visitingIds.Add(ID);					\
						else										\
							visitingIds[availableIds] = ID;			\
						availableIds++;								\
					} while (false)

				// start with objRefLinkStartSet
				vint availableIds = 0;
				PUSH_ID(objRefLinkStartSet);
				for (vint linkIdIndex = 0; linkIdIndex < availableIds; linkIdIndex++)
				{
					// for any new object link, check every object in it
					vint32_t linkId = visitingIds[linkIdIndex];
					while (linkId != -1)
					{
						auto objRefLink = GetInsExec_ObjRefLink(linkId);
						linkId = objRefLink->previous;
						auto ieObject = GetInsExec_Object(objRefLink->id);

						// keep searching until ieObject->lrObjectIds is empty
						while (ieObject->lrObjectIds != -1)
						{
							// skip if it has been searched
							if (ieObject->mergeCounter == MergeStack_MagicCounter) goto CHECK_NEXT_OBJECT;
							ieObject->mergeCounter = MergeStack_MagicCounter;

							auto lrObjRefLink = GetInsExec_ObjRefLink(ieObject->lrObjectIds);
							if (lrObjRefLink->previous == -1)
							{
								// if ieObject->lrObjectIds has only one object
								// continue in place
								ieObject = GetInsExec_Object(lrObjRefLink->id);
							}
							else
							{
								// otherwise
								// the link is pushed and search it later
								PUSH_ID(ieObject->lrObjectIds);
								break;
							}
						}

						// a top object has been found
						if (ieObject->lrObjectIds == -1)
						{
							// skip if it has been searched
							if (ieObject->mergeCounter == MergeStack_MagicCounter) goto CHECK_NEXT_OBJECT;
							ieObject->mergeCounter = MergeStack_MagicCounter;

							if (!callback(ieObject)) return false;
						}
					CHECK_NEXT_OBJECT:;
					}
				}
				return true;
#undef PUSH_ID
			}

			template<typename TCallback>
			bool TraceManager::SearchForTopCreateInstructions(InsExec_Object* ieObject, TCallback&& callback)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::SearchForTopCreateInstructions(InsExec_Object*, TCallback&&)#"
				// find the first instruction in all create instructions
				// its trace should be a common ancestor of all traces of all create instructions
				vint32_t trace = ieObject->bo_bolr_Trace;
				vint32_t ins = ieObject->bo_bolr_Ins;

				auto insRefLinkId = ieObject->dfaInsRefs;
				while (insRefLinkId != -1)
				{
					auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
					insRefLinkId = insRefLink->previous;
					if (insRefLink->trace < trace || (insRefLink->trace == trace && insRefLink->ins < ins))
					{
						trace = insRefLink->trace;
						ins = insRefLink->ins;
					}
				}

#if defined VCZH_MSVC && defined _DEBUG
				// ensure they actually have the same ancestor trace
				vint32_t forwardTraceId = GetTraceExec(GetTrace(trace)->traceExecRef)->branchData.forwardTrace;
				auto boTrace = GetTrace(ieObject->bo_bolr_Trace);
				insRefLinkId = ieObject->dfaInsRefs;
				while (true)
				{
					auto currentTrace = boTrace;
					if (insRefLinkId != -1)
					{
						currentTrace = GetTrace(GetInsExec_InsRefLink(insRefLinkId)->trace);
					}

					auto currentTraceExec = GetTraceExec(currentTrace->traceExecRef);
					while (currentTraceExec->branchData.forwardTrace > forwardTraceId)
					{
						if (currentTrace->allocatedIndex == currentTraceExec->branchData.forwardTrace)
						{
							currentTrace = GetTrace(currentTrace->predecessors.first);
						}
						else
						{
							currentTrace = GetTrace(currentTraceExec->branchData.forwardTrace);
						}
						currentTraceExec = GetTraceExec(currentTrace->traceExecRef);
					}
					CHECK_ERROR(currentTraceExec->branchData.forwardTrace == forwardTraceId, ERROR_MESSAGE_PREFIX L"Internal error: assumption is broken.");

					if (insRefLinkId == -1)
					{
						break;
					}
					else
					{
						auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
						insRefLinkId = insRefLink->previous;
					}
				}
#endif

				return callback(GetTrace(trace), ins);
#undef ERROR_MESSAGE_PREFIX
			}

			template<typename TCallback>
			bool TraceManager::SearchForEndObjectInstructions(Trace* createTrace, vint32_t createIns, TCallback&& callback)
			{
				auto traceExec = GetTraceExec(createTrace->traceExecRef);
				auto insExec = GetInsExec(traceExec->insExecRefs.start + createIns);
				vint32_t insRefLinkId = insExec->eoInsRefs;
				while (insRefLinkId != -1)
				{
					auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
					insRefLinkId = insRefLink->previous;
					if (!callback(GetTrace(insRefLink->trace), insRefLink->ins)) return false;
				}
				return true;
			}

			bool TraceManager::ComparePrefix(TraceExec* baselineTraceExec, TraceExec* commingTraceExec, vint32_t prefix)
			{
				if (commingTraceExec->insLists.c3 < prefix) return false;
				for (vint32_t i = 0; i < prefix; i++)
				{
					auto&& insBaseline = ReadInstruction(i, baselineTraceExec->insLists);
					auto&& insComming = ReadInstruction(i, baselineTraceExec->insLists);
					if (insBaseline != insComming) return false;
				}

				return true;
			}

			bool TraceManager::ComparePostfix(TraceExec* baselineTraceExec, TraceExec* commingTraceExec, vint32_t postfix)
			{
				if (commingTraceExec->insLists.c3 < postfix) return false;
				for (vint32_t i = 0; i < postfix; i++)
				{
					auto&& insBaseline = ReadInstruction(baselineTraceExec->insLists.c3 - i - 1, baselineTraceExec->insLists);
					auto&& insComming = ReadInstruction(baselineTraceExec->insLists.c3 - i - 1, baselineTraceExec->insLists);
					if (insBaseline != insComming) return false;
				}

				return true;
			}

			template<typename TCallback>
			bool TraceManager::CheckAmbiguityResolution(TraceAmbiguity* ta, collections::List<vint32_t>& visitingIds, TCallback&& callback)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CheckAmbiguityResolution(TraceAmbiguity&, List<vint32_t>&, TCallback&&)#"
				// following conditions need to be satisfies if multiple objects could be the result of ambiguity
				//
				// BO/DFA that create objects must be
				//   the same instruction in the same trace
				//   in different trace
				//     these traces share the same predecessor
				//     prefix in these traces are the same
				//
				// EO that ed objects must be
				//   the same instruction in the same trace
				//   in different trace
				//     these traces share the same successor
				//     postfix in these traces are the same

				// initialize TraceAmbiguity
				Trace* first = nullptr;
				Trace* last = nullptr;
				TraceExec* firstTraceExec = nullptr;
				TraceExec* lastTraceExec = nullptr;
				bool foundBeginSame = false;
				bool foundBeginPrefix = false;
				bool foundEndSame = false;
				bool foundEndPostfix = false;
				bool succeeded = false;

				// iterate all top objects
				NEW_MERGE_STACK_MAGIC_COUNTER;
				succeeded = callback([&](vint32_t objRefLink)
				{
					return SearchForTopObjectsWithCounter(objRefLink, visitingIds, [&](InsExec_Object* ieObject)
					{
						PushObjRefLink(ta->topObjectIds, ieObject->allocatedIndex);

						// check if BO/DFA satisfies the condition
						return SearchForTopCreateInstructions(ieObject, [&](Trace* createTrace, vint32_t createIns)
						{
#if defined VCZH_MSVC && defined _DEBUG
							{
								auto traceExec = GetTraceExec(createTrace->traceExecRef);
								auto&& ins = ReadInstruction(createIns, traceExec->insLists);
								CHECK_ERROR(ins.type == AstInsType::BeginObject || ins.type == AstInsType::DelayFieldAssignment, ERROR_MESSAGE_PREFIX L"The found instruction is not a BeginObject or DelayFieldAssignment instruction.");
							}
#endif

							if (!first)
							{
								first = createTrace;
								firstTraceExec = GetTraceExec(first->traceExecRef);
								ta->firstTrace = createTrace->allocatedIndex;
								ta->prefix = createIns;
							}
							else if (first == createTrace)
							{
								// check if two instruction is the same
								if (ta->prefix != createIns) return false;
								foundBeginSame = true;
							}
							else
							{
								// check if two instruction shares the same prefix
								if (first->predecessors.first != createTrace->predecessors.first) return false;
								auto createTraceExec = GetTraceExec(createTrace->traceExecRef);
								if (!ComparePrefix(firstTraceExec, createTraceExec, ta->prefix)) return false;
								foundBeginPrefix = true;
							}

							return true;
						});
					});
				});
				if (!succeeded) return false;

				// iterate all bottom objects
				NEW_MERGE_STACK_MAGIC_COUNTER;
				succeeded = callback([&](vint32_t objRefLink)
				{
					return SearchForObjectsWithCounter(objRefLink, [&](InsExec_Object* ieObject)
					{
						PushObjRefLink(ta->bottomObjectIds, ieObject->allocatedIndex);

						// check if BO/DFA satisfies the condition
						return SearchForTopCreateInstructions(ieObject, [&](Trace* createTrace, vint32_t createIns)
						{
#if defined VCZH_MSVC && defined _DEBUG
							{
								auto traceExec = GetTraceExec(createTrace->traceExecRef);
								auto&& ins = ReadInstruction(createIns, traceExec->insLists);
								CHECK_ERROR(ins.type == AstInsType::BeginObject || ins.type == AstInsType::DelayFieldAssignment, ERROR_MESSAGE_PREFIX L"The found instruction is not a BeginObject or DelayFieldAssignment instruction.");
							}
#endif

							// check if EO satisfies the condition
							return SearchForEndObjectInstructions(createTrace, createIns, [&](Trace* eoTrace, vint32_t eoIns)
							{
#if defined VCZH_MSVC && defined _DEBUG
								{
									auto traceExec = GetTraceExec(eoTrace->traceExecRef);
									auto&& ins = ReadInstruction(eoIns, traceExec->insLists);
									CHECK_ERROR(ins.type == AstInsType::EndObject, ERROR_MESSAGE_PREFIX L"The found instruction is not a EndObject instruction.");
								}
#endif

								if (!last)
								{
									last = eoTrace;
									lastTraceExec = GetTraceExec(last->traceExecRef);
									ta->lastTrace = eoTrace->allocatedIndex;
									ta->postfix = lastTraceExec->insLists.c3 - eoIns - 1;
								}
								else if (last == eoTrace)
								{
									// check if two instruction is the same
									auto eoTraceExec = GetTraceExec(eoTrace->traceExecRef);
									if (ta->postfix != eoTraceExec->insLists.c3 - eoIns - 1) return false;
									foundEndSame = true;
								}
								else
								{
									// check if two instruction shares the same postfix
									if (last->successors.first != eoTrace->successors.first) return false;
									auto eoTraceExec = GetTraceExec(eoTrace->traceExecRef);
									if (!ComparePostfix(lastTraceExec, eoTraceExec, ta->postfix + 1)) return false;
									foundEndPostfix = true;
								}
								return true;
							});
						});
					});
				});
				if (!succeeded) return false;

				// ensure the statistics result is compatible
				if (first && !foundBeginSame && !foundBeginPrefix) foundBeginSame = true;
				if (last && !foundEndSame && !foundEndPostfix) foundEndSame = true;
				if (foundBeginSame == foundBeginPrefix) return false;
				if (foundEndSame == foundEndPostfix) return false;

				// fix prefix if necessary
				if (foundBeginPrefix)
				{
					auto first = GetTrace(GetTrace(ta->firstTrace)->predecessors.first);
					auto traceExec = GetTraceExec(first->traceExecRef);
					ta->firstTrace = first->allocatedIndex;
					ta->prefix += traceExec->insLists.c3;
				}

				// fix postfix if necessary
				if (foundEndPostfix)
				{
					// last will be a merge trace
					// so ta->postfix doesn't need to change
					auto last = GetTrace(GetTrace(ta->lastTrace)->successors.first);
					auto traceExec = GetTraceExec(last->traceExecRef);
					ta->lastTrace = last->allocatedIndex;
				}

				return true;
#undef ERROR_MESSAGE_PREFIX
			}

			bool TraceManager::CheckMergeTrace(TraceAmbiguity* ta, Trace* trace, TraceExec* traceExec, collections::List<vint32_t>& visitingIds)
			{
				// when a merge trace is the surviving trace
				// objects in the top object stack are the result of ambiguity
				if (trace->successorCount == 0)
				{
					auto ieOSTop = GetInsExec_ObjectStack(traceExec->context.objectStack);
					return CheckAmbiguityResolution(ta, visitingIds, [=](auto&& callback)
					{
						return callback(ieOSTop->objectIds);
					});
				}

				// otherwise
				// objects in the top create stack are the result of ambiguity
				// even when there is only one object in the stack

				// but in some cases
				// objects in the top object stack are the result of ambiguity
				// when these objects are the only difference in branches
				// here we need to test if the condition satisfied

				{
					// [CONDITION]
					// the top create stack should be either empty or only contains one object
					if (traceExec->context.createStack != -1)
					{
						auto ieCSTop = GetInsExec_CreateStack(traceExec->context.createStack);
						auto objRefLink = GetInsExec_ObjRefLink(ieCSTop->objectIds);
						if (objRefLink->previous != -1)
						{
							goto CHECK_OBJECTS_IN_TOP_CREATE_STACK;
						}
					}

					// [CONDITION]
					// the first predecessor must has a EndObject instruction
					// count the number of instructions after EndObject
					// these instructions are the postfix
					vint32_t postfix = -1;
					auto firstTrace = GetTrace(trace->predecessors.first);
					auto firstTraceExec = GetTraceExec(firstTrace->traceExecRef);
					for (vint32_t i = firstTraceExec->insLists.c3 - 1; i >= 0; i--)
					{
						auto&& ins = ReadInstruction(i, firstTraceExec->insLists);
						if (ins.type == AstInsType::EndObject)
						{
							postfix = firstTraceExec->insLists.c3 - i - 1;
							break;
						}
					}
					if (postfix == -1) goto CHECK_OBJECTS_IN_TOP_CREATE_STACK;

					// [CONDITION]
					// all predecessor must have a EndObject instruction
					// posftix of all predecessors must be the same
					{
						vint32_t predecessorId = trace->predecessors.last;
						while (predecessorId != firstTrace->allocatedIndex)
						{
							auto predecessor = GetTrace(predecessorId);
							predecessorId = predecessor->predecessors.siblingPrev;
							if (!ComparePostfix(firstTraceExec, GetTraceExec(predecessor->traceExecRef), postfix + 1)) goto CHECK_OBJECTS_IN_TOP_CREATE_STACK;
						}
					}

					// check if all EndObject ended objects are the result of ambiguity
					if (postfix == 0)
					{
						// if EndObject is the last instruction of predecessors
						// then their objRefs has been written to the top object stack
						auto ieOSTop = GetInsExec_ObjectStack(traceExec->context.objectStack);
						auto succeeded = CheckAmbiguityResolution(ta, visitingIds, [=](auto&& callback)
						{
							return callback(ieOSTop->objectIds);
						});
						if (succeeded) return true;
					}
					else
					{
						// otherwise find all objRefs of EndObject
						auto succeeded = CheckAmbiguityResolution(ta, visitingIds, [=, &visitingIds](auto&& callback)
						{
							vint32_t predecessorId = trace->predecessors.first;
							while (predecessorId != -1)
							{
								auto predecessor = GetTrace(predecessorId);
								predecessorId = predecessor->predecessors.siblingNext;

								// search for the object it ends
								auto predecessorTraceExec = GetTraceExec(predecessor->traceExecRef);
								auto indexEO = predecessorTraceExec->insLists.c3 - postfix - 1;
								auto insExecEO = GetInsExec(predecessorTraceExec->insExecRefs.start + indexEO);
								if (!callback(insExecEO->objRefs)) return false;
							}
							return true;
						});
						if (succeeded) return true;
					}

				}
			CHECK_OBJECTS_IN_TOP_CREATE_STACK:
				auto ieCSTop = GetInsExec_CreateStack(traceExec->context.createStack);
				return CheckAmbiguityResolution(ta, visitingIds, [=](auto&& callback)
				{
					return callback(ieCSTop->objectIds);
				});
			}

			void TraceManager::CheckMergeTraces()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CheckMergeTraces()#"
				// iterating TraceMergeExec
				List<vint32_t> visitingIds;
				vint32_t traceId = firstMergeTrace;
				while (traceId != -1)
				{
					auto trace = GetTrace(traceId);
					auto traceExec = GetTraceExec(trace->traceExecRef);
					traceId = traceExec->nextMergeTrace;

					auto ta = GetTraceAmbiguity(traceAmbiguities.Allocate());
					bool succeeded = CheckMergeTrace(ta, trace, traceExec, visitingIds);
					CHECK_ERROR(succeeded, ERROR_MESSAGE_PREFIX L"Failed to find ambiguous objects in a merge trace.");
					traceExec->ambiguity = ta->allocatedIndex;
				}
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
ResolveAmbiguity
***********************************************************************/

			void TraceManager::ResolveAmbiguity()
			{
				CHECK_ERROR(state == TraceManagerState::PreparedTraceRoute, L"vl::glr::automaton::TraceManager::ResolveAmbiguity()#Wrong timing to call this function.");
				state = TraceManagerState::ResolvedAmbiguity;

				CheckMergeTraces();
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}