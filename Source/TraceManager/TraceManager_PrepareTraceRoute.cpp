#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;

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

			template<typename T, T* (TraceManager::* get)(vint32_t)>
			void TraceManager::BuildDoubleLink(T* node, vint32_t& top, vint32_t& bottom)
			{
				if (top == -1)
				{
					top = node->allocatedIndex;
					bottom = node->allocatedIndex;
				}
				else
				{
					(this->*get)(top)->next = node->allocatedIndex;
					node->previous = top;
					top = node->allocatedIndex;
				}
			}

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

					if (trace->successors.first != trace->successors.last)
					{
						auto branchExec = GetTraceBranchExec(branchExecs.Allocate());
						branchExec->traceId = trace->allocatedIndex;
						traceExec->branchExec = branchExec->allocatedIndex;

						BuildDoubleLink<TraceBranchExec, &TraceManager::GetTraceBranchExec>(
							branchExec,
							topBranchExec,
							bottomBranchExec);
					}

					if (trace->predecessors.first != trace->predecessors.last)
					{
						auto mergeExec = GetTraceMergeExec(mergeExecs.Allocate());
						mergeExec->traceId = trace->allocatedIndex;
						traceExec->mergeExec = mergeExec->allocatedIndex;

						BuildDoubleLink<TraceMergeExec, &TraceManager::GetTraceMergeExec>(
							mergeExec,
							topMergeExec,
							bottomMergeExec);
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

			vuint64_t MergeStack_MagicCounter = 0;

#define NEW_MERGE_STACK_MAGIC_COUNTER (void)(MergeStack_MagicCounter++)

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

#undef NEW_MERGE_STACK_MAGIC_COUNTER

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

			void TraceManager::CheckMergeTrace(Trace* trace, TraceExec* traceExec, TraceMergeExec* tme)
			{
				// when a merge trace is the surviving trace
				// objects in the top object stack are the result of ambiguity
				if (trace->successorCount == 0)
				{
					tme->objectIdsToMerge = GetInsExec_ObjectStack(traceExec->context.objectStack)->objectIds;
					return;
				}

				// otherwise
				// objects in the top create stack are the result of ambiguity
				// even when there is only one object in the stack
				tme->objectIdsToMerge = GetInsExec_CreateStack(traceExec->context.createStack)->objectIds;

				// but in some cases
				// objects in the top object stack are the result of ambiguity
				// when these objects are the only difference in branches
				// here we need to test if the condition satisfied

				// [CONDITION]
				// the top create stack should be either empty or only contains one object
				if (traceExec->context.createStack != -1)
				{
					auto ieCSTop = GetInsExec_CreateStack(traceExec->context.createStack);
					auto objRefLink = GetInsExec_ObjRefLink(ieCSTop->objectIds);
					if (objRefLink->previous != -1)
					{
						return;
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
				if (postfix == -1) return;

				// [CONDITION]
				// all predecessor must have a EndObject instruction
				// posftix of all predecessors must be the same
				vint32_t predecessorId = trace->predecessors.last;
				while (predecessorId != firstTrace->allocatedIndex)
				{
					auto predecessor = GetTrace(predecessorId);
					predecessorId = predecessor->predecessors.siblingPrev;
					auto predecessorTraceExec = GetTraceExec(predecessor->traceExecRef);

					if (predecessorTraceExec->insLists.c3 <= postfix) return;
					auto&& insEO = ReadInstruction(predecessorTraceExec->insLists.c3 - postfix - 1, predecessorTraceExec->insLists);
					if (insEO.type != AstInsType::EndObject) return;

					for (vint32_t i = 0; i < postfix; i++)
					{
						auto&& insFirst = ReadInstruction(firstTraceExec->insLists.c3 - i - 1, firstTraceExec->insLists);
						auto&& insPredecessor = ReadInstruction(predecessorTraceExec->insLists.c3 - i - 1, predecessorTraceExec->insLists);
						if (insFirst != insPredecessor) return;
					}
				}

				// [CONDITION]
				// all EndObject must be the last EndObject of the objects it ends
				// the first create instructions of the objects must be
				//   the same instruction in the same trace
				//   in different trace
				//     these traces share the same predecessor
				//     prefix (instructions before the create instruction) in these traces are the same
			}

			void TraceManager::CheckMergeTraces()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CheckMergeTraces()#"
				// iterating TraceMergeExec
				vint32_t tmeId = bottomMergeExec;
				while (tmeId != -1)
				{
					auto tme = GetTraceMergeExec(tmeId);
					auto trace = GetTrace(tme->traceId);
					auto traceExec = GetTraceExec(trace->traceExecRef);
					tmeId = tme->next;
					CheckMergeTrace(trace, traceExec, tme);
					CHECK_ERROR(tme->objectIdsToMerge != -1, ERROR_MESSAGE_PREFIX L"Failed to find ambiguous objects in a merge trace.");
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
		}
	}
}