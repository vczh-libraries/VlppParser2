#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;

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
IterateSurvivedTraces
***********************************************************************/

			template<typename TCallback>
			void TraceManager::IterateSurvivedTraces(TCallback&& callback)
			{
				List<Trace*> traces;
				traces.Add(initialTrace);

				while (traces.Count() > 0)
				{
					auto current = traces[traces.Count() - 1];
					traces.RemoveAt(traces.Count() - 1);

					if (!callback(current)) continue;

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
AllocateExecutionData
***********************************************************************/

			void TraceManager::AllocateExecutionData()
			{
				vint32_t insExecCount = 0;
				IterateSurvivedTraces([&](Trace* trace)
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

			void TraceManager::PartialExecuteTraces()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::PartialExecuteTraces()#"
				Trace* lastTrace = nullptr;
				IterateSurvivedTraces([&](Trace* trace)
				{
					if (trace->predecessors.first == trace->predecessors.last)
					{
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
									ieCSTop->dfa_bo_bolr_Trace = trace->allocatedIndex;
									ieCSTop->dfa_bo_bolr_Ins = insRef;

									insExec->objectId = ieObject->pushedObjectId;
								}
								break;
							case AstInsType::BeginObjectLeftRecursive:
								{
									CHECK_ERROR(GetStackTop(context) - GetStackBase(context) >= 1, ERROR_MESSAGE_PREFIX L"Pushed values not enough.");

									auto ieOSTop = GetInsExec_ObjectStack(context.objectStack);
									CHECK_ERROR(ieOSTop->objectId >= 0 || ieOSTop->objectId <= -3, ERROR_MESSAGE_PREFIX L"The poped value is not an object.");
									auto ieObjTop = GetInsExec_Object(ieOSTop->objectId >= 0 ? ieOSTop->objectId : -(ieOSTop->objectId + 3));

									auto ieObject = GetInsExec_Object(insExec_Objects.Allocate());
									ieObject->pushedObjectId = ieObject->allocatedIndex;
									ieObject->dfa_bo_bolr_ra_Trace = trace->allocatedIndex;
									ieObject->dfa_bo_bolr_ra_Ins = insRef;

									auto ieCSTop = PushCreateStack(context);
									ieCSTop->objectId = ieObject->pushedObjectId;
									ieCSTop->stackBase = ieOSTop->pushedCount - 1;
									ieCSTop->dfa_bo_bolr_Trace = trace->allocatedIndex;
									ieCSTop->dfa_bo_bolr_Ins = insRef;

									insExec->objectId = ieObject->pushedObjectId;
									insExec->associatedTrace = ieObjTop->dfa_bo_bolr_ra_Trace;
									insExec->associatedIns = ieObjTop->dfa_bo_bolr_ra_Ins;
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
									ieCS->dfa_bo_bolr_Trace = trace->allocatedIndex;
									ieCS->dfa_bo_bolr_Ins = insRef;

									insExec->objectId = ieObject->pushedObjectId;
								}
								break;
							case AstInsType::ReopenObject:
								{
									CHECK_ERROR(GetStackTop(context) - GetStackBase(context) >= 1, ERROR_MESSAGE_PREFIX L"Pushed values not enough.");
									CHECK_ERROR(context.createStack != -1, ERROR_MESSAGE_PREFIX L"There is no created object.");

									auto ieCSTop = GetInsExec_CreateStack(context.createStack);
									CHECK_ERROR(ieCSTop->objectId <= -3, ERROR_MESSAGE_PREFIX L"DelayFieldAssignment is not submitted before ReopenObject.");

									auto ieObjTop = GetInsExec_ObjectStack(context.objectStack);
									context.objectStack = ieObjTop->previous;

									insExec->objectId = ieCSTop->objectId;
									insExec->associatedTrace = ieCSTop->dfa_bo_bolr_Trace;
									insExec->associatedIns = ieCSTop->dfa_bo_bolr_Ins;
								}
								break;
							case AstInsType::EndObject:
								{
									CHECK_ERROR(context.createStack != -1, ERROR_MESSAGE_PREFIX L"There is no created object.");

									auto ieCSTop = GetInsExec_CreateStack(context.createStack);
									context.createStack = ieCSTop->previous;
									PushObjectStack(context, ieCSTop->objectId);

									insExec->objectId = ieCSTop->objectId;
									insExec->associatedTrace = ieCSTop->dfa_bo_bolr_Ins;
									insExec->associatedIns = ieCSTop->dfa_bo_bolr_Trace;
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

						lastTrace = trace;
						return true;
					}
					else
					{
						auto firstPredecessor = GetTrace(trace->predecessors.first);
						if (trace->predecessors.first == lastTrace->allocatedIndex)
						{
							GetTraceExec(trace->traceExecRef)->context = GetTraceExec(firstPredecessor->traceExecRef)->context;
							lastTrace = trace;
							return true;
						}
						else
						{
							auto contextBaseline = GetTraceExec(trace->traceExecRef)->context;
							auto contextComming = GetTraceExec(firstPredecessor->traceExecRef)->context;
							CHECK_ERROR(
								contextBaseline.objectStack == contextComming.objectStack &&
								contextBaseline.createStack == contextComming.createStack &&
								contextBaseline.lriStored == contextComming.lriStored,
								ERROR_MESSAGE_PREFIX L"Execution results of traces to merge are different."
								);
							lastTrace = nullptr;
							return false;
						}
					}
				});
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
PrepareTraceRoute
***********************************************************************/

			void TraceManager::PrepareTraceRoute()
			{
				if (state == TraceManagerState::PreparedTraceRoute) return;
				CHECK_ERROR(state == TraceManagerState::Finished, L"vl::glr::automaton::TraceManager::PrepareTraceRoute()#Wrong timing to call this function.");
				state = TraceManagerState::PreparedTraceRoute;

				AllocateExecutionData();
				PartialExecuteTraces();
			}
		}
	}
}