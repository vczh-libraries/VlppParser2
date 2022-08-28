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
							AstIns ins;
							ReadInstruction(insRef, traceExec->insLists);

							switch (ins.type)
							{
							case AstInsType::BeginObject:
								break;
							case AstInsType::BeginObjectLeftRecursive:
								break;
							case AstInsType::DelayFieldAssignment:
								break;
							case AstInsType::ReopenObject:
								break;
							case AstInsType::EndObject:
								break;
							case AstInsType::DiscardValue:
								break;
							case AstInsType::LriStore:
								break;
							case AstInsType::LriFetch:
								break;
							case AstInsType::Field:
								break;
							case AstInsType::FieldIfUnassigned:
								break;
							case AstInsType::ResolveAmbiguity:
								break;
							default:;
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
			}
		}
	}
}