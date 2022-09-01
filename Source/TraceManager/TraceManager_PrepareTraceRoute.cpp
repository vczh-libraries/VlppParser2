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

			void TraceManager::AllocateExecutionData()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::AllocateExecutionData()#"
				vint32_t insExecCount = 0;
				IterateSurvivedTraces([&](Trace* trace, Trace* predecessor, vint32_t visitCount, vint32_t predecessorCount)
				{
					if (visitCount != 1) return;
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
						traceExec->branchExec = branchExec->allocatedIndex;

						branchExec->previous = topBranchExec;
						topBranchExec = branchExec->allocatedIndex;
					}

					if (trace->predecessors.first != trace->predecessors.last)
					{
						auto mergeExec = GetTraceMergeExec(mergeExecs.Allocate());
						traceExec->mergeExec = mergeExec->allocatedIndex;

						mergeExec->previous = topMergeExec;
						topMergeExec = mergeExec->allocatedIndex;
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
				auto ieObject = GetInsExec_Object(insExec_Objects.Allocate());
				ieObject->previous = topObject;
				topObject = ieObject->allocatedIndex;
				return ieObject;
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

			InsExec_ObjectStack* TraceManager::PushObjectStack(InsExec_Context& context, vint32_t objectId)
			{
				auto ie = GetInsExec_ObjectStack(insExec_ObjectStacks.Allocate());
				ie->previous = context.objectStack;
				ie->objectId = objectId;
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
					auto ins = ReadInstruction(insRef, traceExec->insLists);
					auto insExec = GetInsExec(traceExec->insExecRefs.start + insRef);
					insExec->topCreatedObjectBefore = context.createStack == -1 ? -1 : GetInsExec_CreateStack(context.createStack)->objectId;

					switch (ins.type)
					{
					case AstInsType::BeginObject:
						{
							auto ieObject = NewObject();
							ieObject->pushedObjectId = ieObject->allocatedIndex;
							ieObject->dfa_bo_bolr_ra_Trace = trace->allocatedIndex;
							ieObject->dfa_bo_bolr_ra_Ins = insRef;

							auto ieCSTop = PushCreateStack(context);
							ieCSTop->objectId = ieObject->pushedObjectId;
							ieCSTop->stackBase = GetStackTop(context);

							insExec->objectId = ieObject->pushedObjectId;
						}
						break;
					case AstInsType::BeginObjectLeftRecursive:
						{
							CHECK_ERROR(GetStackTop(context) - GetStackBase(context) >= 1, ERROR_MESSAGE_PREFIX L"Pushed values not enough.");

							auto ieOSTop = GetInsExec_ObjectStack(context.objectStack);

							auto ieObject = NewObject();
							ieObject->pushedObjectId = ieObject->allocatedIndex;
							ieObject->lrObjectId = ieOSTop->objectId;
							ieObject->dfa_bo_bolr_ra_Trace = trace->allocatedIndex;
							ieObject->dfa_bo_bolr_ra_Ins = insRef;

							auto ieCSTop = PushCreateStack(context);
							ieCSTop->objectId = ieObject->pushedObjectId;
							ieCSTop->stackBase = ieOSTop->pushedCount - 1;

							insExec->objectId = ieObject->pushedObjectId;
						}
						break;
					case AstInsType::DelayFieldAssignment:
						{
							auto ieObject = NewObject();
							ieObject->pushedObjectId = -ieObject->allocatedIndex - 3;
							ieObject->dfa_bo_bolr_ra_Trace = trace->allocatedIndex;
							ieObject->dfa_bo_bolr_ra_Ins = insRef;

							auto ieCS = PushCreateStack(context);
							ieCS->objectId = ieObject->pushedObjectId;
							ieCS->stackBase = GetStackTop(context);

							insExec->objectId = ieObject->pushedObjectId;
						}
						break;
					case AstInsType::ReopenObject:
						{
							CHECK_ERROR(GetStackTop(context) - GetStackBase(context) >= 1, ERROR_MESSAGE_PREFIX L"Pushed values not enough.");
							CHECK_ERROR(context.createStack != -1, ERROR_MESSAGE_PREFIX L"There is no created object.");

							auto ieCSTop = GetInsExec_CreateStack(context.createStack);
							CHECK_ERROR(ieCSTop->objectId <= -3, ERROR_MESSAGE_PREFIX L"DelayFieldAssignment is not submitted before ReopenObject.");

							auto ieOSTop = GetInsExec_ObjectStack(context.objectStack);
							context.objectStack = ieOSTop->previous;

							auto ieObjTop = GetInsExec_Object(ieOSTop->objectId);
							CHECK_ERROR(ieObjTop->dfaObjectId == -1 || ieObjTop->dfaObjectId == ieCSTop->objectId, ERROR_MESSAGE_PREFIX L"Internal error: InsExec_Object::dfaObjectId has been assigned.");
							ieObjTop->dfaObjectId = ieCSTop->objectId;

							insExec->objectId = ieCSTop->objectId;
						}
						break;
					case AstInsType::EndObject:
						{
							CHECK_ERROR(context.createStack != -1, ERROR_MESSAGE_PREFIX L"There is no created object.");

							auto ieCSTop = GetInsExec_CreateStack(context.createStack);
							context.createStack = ieCSTop->previous;
							PushObjectStack(context, ieCSTop->objectId);

							auto ieObject = GetInsExec_Object(ieCSTop->objectId);
							if (++ieObject->eo_Counter == 1)
							{
								ieObject->eo_Trace = trace->allocatedIndex;
								ieObject->eo_Ins = insRef;
							}
							else
							{
								ieObject->eo_Trace = -1;
								ieObject->eo_Ins = -1;
							}

							insExec->objectId = ieCSTop->objectId;
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
							CHECK_ERROR(context.lriStored == -1, ERROR_MESSAGE_PREFIX L"LriFetch is not executed before the next LriStore.");

							auto ieObjTop = GetInsExec_ObjectStack(context.objectStack);
							context.objectStack = ieObjTop->previous;
							context.lriStored = ieObjTop->objectId;
						}
						break;
					case AstInsType::LriFetch:
						{
							CHECK_ERROR(context.lriStored != -1, ERROR_MESSAGE_PREFIX L"LriStore is not executed before the next LriFetch.");
							PushObjectStack(context, context.lriStored);
							context.lriStored = -1;
						}
						break;
					case AstInsType::ResolveAmbiguity:
						{
							CHECK_ERROR(GetStackTop(context) - GetStackBase(context) >= (vint32_t)ins.count, ERROR_MESSAGE_PREFIX L"Pushed values not enough create an ambiguity node.");
							for (vint i = 0; i < ins.count; i++)
							{
								auto ieObjTop = GetInsExec_ObjectStack(context.objectStack);
								context.objectStack = ieObjTop->previous;
							}

							auto ieObject = NewObject();
							ieObject->pushedObjectId = ieObject->allocatedIndex;
							ieObject->dfa_bo_bolr_ra_Trace = trace->allocatedIndex;
							ieObject->dfa_bo_bolr_ra_Ins = insRef;
							PushObjectStack(context, ieObject->pushedObjectId);

							insExec->objectId = ieObject->pushedObjectId;
						}
						break;
					case AstInsType::Token:
					case AstInsType::EnumItem:
						{
							PushObjectStack(context, -3);
						}
						break;
					default:;
						CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unrecognizabled instruction.");
					}
				}
				traceExec->context = context;
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
PartialExecuteTraces
***********************************************************************/

			template<typename T, T* (TraceManager::* GetData)(vint32_t index), typename TCallback, typename TError>
			void TraceManager::CompareObjectOrCreateStack(vint32_t stack1, vint32_t stack2, TCallback&& callback, TError&& error)
			{
				while (stack1 != stack2)
				{
					if (stack1 == -1 || stack2 == -1) error();

					auto stackObj1 = (this->*GetData)(stack1);
					auto stackObj2 = (this->*GetData)(stack2);
					callback(stackObj1, stackObj2, error);

					stack1 = stackObj1->previous;
					stack2 = stackObj2->previous;
				}
			}

			void TraceManager::PartialExecuteTraces()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::PartialExecuteTraces()#"
				IterateSurvivedTraces(
					[this](Trace* trace, Trace* predecessor, vint32_t visitCount, vint32_t predecessorCount)
					{
						if (trace->allocatedIndex == 24)
						{
							int a = 0;
						}
						if (predecessorCount <= 1)
						{
							PartialExecuteOrdinaryTrace(trace);
						}
						else
						{
							auto&& contextComming = GetTraceExec(predecessor->traceExecRef)->context;
							auto&& contextBaseline = GetTraceExec(trace->traceExecRef)->context;
							if (visitCount == 1)
							{
								contextBaseline = contextComming;
							}
							else
							{
								auto error = []()
								{
									CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Execution results of traces to merge are different.");
								};

								// check if the two lriStored be both empty or non-empty
								if ((contextBaseline.lriStored != -1) != (contextComming.lriStored != -1)) error();

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
								CompareObjectOrCreateStack<InsExec_CreateStack, &TraceManager::GetInsExec_CreateStack>(
									contextBaseline.createStack,
									contextComming.createStack,
									[this](InsExec_CreateStack* baselineStack, InsExec_CreateStack* commingStack, auto&& error)
									{
										if (baselineStack->stackBase != commingStack->stackBase) error();
									},
									error
									);
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

							auto commingTraceExec = GetTraceExec(predecessor->traceExecRef);
							auto comming = commingTraceExec->branchData;
							if (predecessor->state != -1)
							{
								// if a merge state's predecessor is also a merge state
								// use its data
								// because they are equivalent
								// otherwise, use the data from its forwardTrace
								auto branchHeadTrace = GetTrace(comming.forwardTrace);
								comming = { branchHeadTrace->predecessors.first,comming.branchDepth - 1 };
							}

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
BuildObjectHierarchy
***********************************************************************/

			void TraceManager::BuildObjectHierarchy()
			{
				vint32_t id = topObject;
				while (id != -1)
				{
					auto ieObject = GetInsExec_Object(id);
					id = ieObject->previous;
				}
			}

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
				BuildObjectHierarchy();
			}

/***********************************************************************
DetermineAmbiguityRanges
***********************************************************************/

			void TraceManager::DetermineAmbiguityRanges()
			{
				// reverse iterating TraceBranchExec
				vint32_t id = topBranchExec;
				while (id != -1)
				{
					auto branchExec = GetTraceBranchExec(id);
					id = branchExec->previous;
				}
			}

/***********************************************************************
ResolveAmbiguity
***********************************************************************/

			void TraceManager::ResolveAmbiguity()
			{
				CHECK_ERROR(state == TraceManagerState::PreparedTraceRoute, L"vl::glr::automaton::TraceManager::ResolveAmbiguity()#Wrong timing to call this function.");
				state = TraceManagerState::ResolvedAmbiguity;

				DetermineAmbiguityRanges();
			}
		}
	}
}