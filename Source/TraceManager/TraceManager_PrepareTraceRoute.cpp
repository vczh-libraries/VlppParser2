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
				vint32_t insExecCount = 0;
				IterateSurvivedTraces([&](Trace* trace, ...)
				{
					if (trace->traceExecRef != -1) return false;
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
					return true;
				});
				insExecs.Resize(insExecCount);
			}

/***********************************************************************
PartialExecuteTraces
***********************************************************************/

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
					auto insExec = GetInsExec(traceExec->insExecRefs.start + insRef);
					AstIns ins = ReadInstruction(insRef, traceExec->insLists);

					switch (ins.type)
					{
					case AstInsType::BeginObject:
						{
							auto ieObject = GetInsExec_Object(insExec_Objects.Allocate());
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

							auto ieObject = GetInsExec_Object(insExec_Objects.Allocate());
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
							auto ieObject = GetInsExec_Object(insExec_Objects.Allocate());
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

							auto ieObject = GetInsExec_Object(insExec_Objects.Allocate());
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
PrepareTraceRoute
***********************************************************************/

			void TraceManager::PrepareTraceRoute()
			{
				CHECK_ERROR(state == TraceManagerState::Finished, L"vl::glr::automaton::TraceManager::PrepareTraceRoute()#Wrong timing to call this function.");
				state = TraceManagerState::PreparedTraceRoute;

				AllocateExecutionData();
				PartialExecuteTraces();
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
							traceExec->forwardTrace = trace->allocatedIndex;
							traceExec->branchDepth = 0;
						}
						else if (predecessorCount == 1)
						{
							// if a predecessor is a merge trace
							// jump to its forwardTrace
							// until an ordinary trace is found
							while (predecessor->state == -1)
							{
								predecessor = GetTrace(GetTraceExec(predecessor->traceExecRef)->forwardTrace);
							}

							// copy its data
							auto predecessorTraceExec = GetTraceExec(predecessor->traceExecRef);
							traceExec->forwardTrace = predecessorTraceExec->forwardTrace;
							traceExec->branchDepth = predecessorTraceExec->branchDepth;
						}
						else
						{
							auto expected = GetTraceExec(predecessor->traceExecRef);
							if (predecessor->state != -1)
							{
								// if a merge state's predecessor is also a merge state
								// use its data
								// because they are equivalent
								// otherwise, use the data from its forwardTrace
								expected = GetTraceExec(GetTrace(expected->forwardTrace)->traceExecRef);
							}

							if (visitCount == 1)
							{
								// for the first time visiting a merge trace, copy the data
								traceExec->forwardTrace = expected->forwardTrace;
								traceExec->branchDepth = expected->branchDepth;
							}
							else if (traceExec->forwardTrace != expected->forwardTrace)
							{
								// otherwise, use the data from the latest common shared node
								auto stepForward = [this](TraceExec* traceExec)
								{
									return traceExec;
								};

								auto closer = traceExec->branchDepth <= expected->branchDepth ? traceExec : expected;
								auto further= traceExec->branchDepth > expected->branchDepth ? traceExec : expected;

								while (closer->branchDepth < further->branchDepth)
								{
									closer = stepForward(closer);
								}
								CHECK_ERROR(closer->branchDepth == further->branchDepth, ERROR_MESSAGE_PREFIX L"Internal error: branchDepth corrupted.");

								while (closer->forwardTrace != expected->forwardTrace)
								{
									closer = stepForward(closer);
									expected = stepForward(expected);
								}
								traceExec->forwardTrace = expected->forwardTrace;
								traceExec->branchDepth = expected->branchDepth;
							}
						}
					}
				);
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
ResolveAmbiguity
***********************************************************************/

			void TraceManager::ResolveAmbiguity()
			{
				CHECK_ERROR(state == TraceManagerState::PreparedTraceRoute, L"vl::glr::automaton::TraceManager::ResolveAmbiguity()#Wrong timing to call this function.");
				state = TraceManagerState::ResolvedAmbiguity;

				BuildAmbiguityStructures();
			}
		}
	}
}