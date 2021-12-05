#include "SyntaxBase.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;

/***********************************************************************
TraceManager::PrepareTraceRoute
***********************************************************************/

			void TraceManager::ReadInstructionList(Trace* trace, TraceInsLists& insLists)
			{
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
				if (trace->executedReturn != -1)
				{
					auto& returnDesc = executable.returns[trace->executedReturn];
					insLists.returnInsAfterInput = returnDesc.insAfterInput;
				}
				else
				{
					insLists.returnInsAfterInput = {};
				}

				insLists.c1 = insLists.edgeInsBeforeInput.count;
				insLists.c2 = insLists.c1 + insLists.edgeInsAfterInput.count;
				insLists.c3 = insLists.c2 + insLists.returnInsAfterInput.count;
			}

			AstIns& TraceManager::ReadInstruction(vint instruction, TraceInsLists& insLists)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::ReadInstruction(vint, TraceInsLists&)#"
				CHECK_ERROR(0 <= instruction && instruction <= insLists.c3, ERROR_MESSAGE_PREFIX L"Instruction index out of range.");

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

				return executable.instructions[insRef];
#undef ERROR_MESSAGE_PREFIX
			}

			bool TraceManager::RunInstruction(vint instruction, TraceInsLists& insLists, vint& objectCount)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::SearchSingleTraceForBeginObject(Trace*&, vint&, vint&)#"
				auto& ins = ReadInstruction(instruction, insLists);
				switch (ins.type)
				{
				case AstInsType::EndObject:
					objectCount++;
					break;
				case AstInsType::ReopenObject:
				case AstInsType::BeginObject:
				case AstInsType::BeginObjectLeftRecursive:
					CHECK_ERROR(objectCount > 0, ERROR_MESSAGE_PREFIX L"Encountered unbalanced instructions.");
					objectCount--;
					break;
				}

				return objectCount == 0 && (ins.type == AstInsType::BeginObject || ins.type == AstInsType::BeginObjectLeftRecursive);
#undef ERROR_MESSAGE_PREFIX
			}

			void TraceManager::FindBalancedBeginObject(Trace*& trace, vint& instruction, vint& objectCount)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::FindBalancedBeginObject(Trace*&, vint&, vint&)#"
				TraceInsLists insLists;
				ReadInstructionList(trace, insLists);

				while (true)
				{
					if (trace->predecessors.first != trace->predecessors.last)
					{
						FillAmbiguityInfoForMergingTrace(trace);
						for (vint i = instruction; i > trace->ambiguity.insEndObject; i--)
						{
							if (RunInstruction(i, insLists, objectCount))
							{
								instruction = i;
								return;
							}
						}

						instruction = trace->ambiguity.insBeginObject - 1;
						trace = GetTrace(trace->ambiguity.traceBeginObject);
						ReadInstructionList(trace, insLists);
					}
					else
					{
						for (vint i = instruction; i >= 0; i--)
						{
							if (RunInstruction(i, insLists, objectCount))
							{
								instruction = i;
								return;
							}
						}

						CHECK_ERROR(trace->predecessors.first != -1, ERROR_MESSAGE_PREFIX L"Encountered unbalanced instructions.");
						auto lastBranch = trace;

						trace = GetTrace(trace->predecessors.first);
						ReadInstructionList(trace, insLists);
						instruction = insLists.c3 - 1;

						if (trace->successors.first != trace->successors.last)
						{
							lastBranch->runtimeRouting.expectedVisitCount++;
						}
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void TraceManager::FillAmbiguityInfoForMergingTrace(Trace* trace)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::SearchSingleTraceForBeginObject(Trace*&, vint&, vint&)#"
				if (trace->ambiguity.traceBeginObject != -1)
				{
					return;
				}

				CHECK_ERROR(trace->predecessors.first != trace->predecessors.last, L"This function is not allowed to run on non-merging traces.");
				TraceInsLists insLists;
				ReadInstructionList(trace, insLists);

				vint insEndObject = -1;
				for (vint i = 0; i < insLists.c3; i++)
				{
					auto& ins = ReadInstruction(i, insLists);
					if (ins.type == AstInsType::EndObject)
					{
						insEndObject = i;
						break;
					}
				}
				CHECK_ERROR(insEndObject != -1, ERROR_MESSAGE_PREFIX L"Cannot find EndObject instruction in the merging trace.");

				vint objectCount = 1;
				for (vint i = insEndObject - 1; i >= 0; i--)
				{
					if (RunInstruction(i, insLists, objectCount))
					{
						CHECK_FAIL(ERROR_MESSAGE_PREFIX L"BeginObject for the EndObject in the merging trace is impossible to be in the same trace.");
					}
				}

				vint insBeginObject = -1;
				vint traceBeginObject = -1;

				vint predecessorId = trace->predecessors.first;
				while (predecessorId != -1)
				{
					auto predecessor = GetTrace(predecessorId);
					{
						TraceInsLists branchInsLists;
						ReadInstructionList(predecessor, branchInsLists);

						auto branchTrace = predecessor;
						vint branchInstruction = branchInsLists.c3 - 1;
						vint branchObjectCount = objectCount;
						FindBalancedBeginObject(branchTrace, branchInstruction, branchObjectCount);

						if (traceBeginObject == -1)
						{
							traceBeginObject = branchTrace->allocatedIndex;
							insBeginObject = branchInstruction;
						}
						else
						{
							CHECK_ERROR(traceBeginObject == branchTrace->allocatedIndex && insBeginObject == branchInstruction, ERROR_MESSAGE_PREFIX L"BeginObject searched from different branches are not the same.");
						}
					}
					predecessorId = predecessor->predecessors.siblingNext;
					trace->runtimeRouting.expectedVisitCount++;
				}

				trace->ambiguity.insEndObject = insEndObject;
				trace->ambiguity.insBeginObject = insBeginObject;
				trace->ambiguity.traceBeginObject = traceBeginObject;
#undef ERROR_MESSAGE_PREFIX
			}

			void TraceManager::FillAmbiguityInfoForPredecessorTraces(Trace* trace)
			{
				while (trace)
				{
					if (trace->predecessors.first != trace->predecessors.last)
					{
						if (trace->ambiguity.traceBeginObject == -1)
						{
							FillAmbiguityInfoForMergingTrace(trace);
							if (maxTraceVisitCount < trace->runtimeRouting.expectedVisitCount)
							{
								maxTraceVisitCount = trace->runtimeRouting.expectedVisitCount;
							}
							trace = GetTrace(trace->ambiguity.traceBeginObject);
						}
						else
						{
							break;
						}
					}
					else
					{
						if (trace->predecessors.first == -1) break;
						trace = GetTrace(trace->predecessors.first);
					}
				}
			}

			Trace* TraceManager::PrepareTraceRoute()
			{
				if (state == TraceManagerState::PreparedTraceRoute) return rootTrace;
				CHECK_ERROR(state == TraceManagerState::Finished, L"vl::glr::automaton::TraceManager::PrepareTraceRoute()#Wrong timing to call this function.");
				state = TraceManagerState::PreparedTraceRoute;

				Trace* rootTraceCandidate = nullptr;
				SortedList<Trace*> available;
				List<Trace*> visited;

				for (vint i = 0; i < concurrentCount; i++)
				{
					auto trace = concurrentTraces->Get(i);
					visited.Add(trace);
				}

				for (vint i = 0; i < visited.Count(); i++)
				{
					auto visiting = visited[i];
					if (available.Contains(visiting)) continue;
					available.Add(visiting);

					if (visiting->predecessors.first == -1)
					{
						CHECK_ERROR(rootTraceCandidate == nullptr, L"vl::glr::automaton::TraceManager::PrepareTraceRoute()#Impossible to have more than one root trace.");
						rootTraceCandidate = visiting;
					}

					auto predecessorId = visiting->predecessors.first;
					while (predecessorId != -1)
					{
						auto predecessor = GetTrace(predecessorId);
						AddTraceToCollection(predecessor, visiting, &Trace::successors);
						predecessorId = predecessor->predecessors.siblingNext;
						visited.Add(predecessor);
					}
				}

				for (vint i = 0; i < concurrentCount; i++)
				{
					auto trace = concurrentTraces->Get(i);
					FillAmbiguityInfoForPredecessorTraces(trace);
				}

				rootTrace = rootTraceCandidate;
				return rootTrace;
			}
		}
	}
}