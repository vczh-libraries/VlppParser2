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
				// this function collects byEdge's insBeforeInput, byEdge's insAfterInput, executedReturn's insAfterInput in order
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

				insLists.c1 = (vint32_t)(insLists.edgeInsBeforeInput.count);
				insLists.c2 = (vint32_t)(insLists.c1 + insLists.edgeInsAfterInput.count);
				insLists.c3 = (vint32_t)(insLists.c2 + insLists.returnInsAfterInput.count);
			}

			AstIns& TraceManager::ReadInstruction(vint32_t instruction, TraceInsLists& insLists)
			{
				// access the instruction object from a trace
				// the index is the instruction in a virtual instruction array
				// defined by all InstructionArray in TraceInsLists combined together
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

			bool TraceManager::RunInstruction(vint32_t instruction, TraceInsLists& insLists, vint32_t& objectCount)
			{
				// run an instruction to simulate the number of extra constructing AST objects in the stack
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

			void TraceManager::FindBalancedBeginObject(Trace*& trace, vint32_t& instruction, vint32_t& objectCount)
			{
				// given the current instruction and the current constructing AST objects
				// find the nearlest BeginObject or BeginObjectLeftRecursive before the current instruction
				// that creates the bottom object
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::FindBalancedBeginObject(Trace*&, vint&, vint&)#"
				TraceInsLists insLists;
				ReadInstructionList(trace, insLists);

				while (true)
				{
					if (trace->predecessors.first != trace->predecessors.last)
					{
						// if there are multiple predecessors
						// then this is a ambiguity resolving trace
						FillAmbiguityInfoForMergingTrace(trace);

						// execute all instructions until it reaches the first EndObject instruction
						// and this EndObject instruction is not executed
						for (auto i = instruction; i > trace->ambiguity.insEndObject; i--)
						{
							if (RunInstruction(i, insLists, objectCount))
							{
								instruction = i;
								return;
							}
						}

						// since the BeginObject instruction for this EndObject instruction will be executed after calling FillAmbiguityInfoForMergingTrace
						// must jump to the instruction before that BeginObject instruction
						instruction = trace->ambiguity.insBeginObject - 1;
						trace = GetTrace(trace->ambiguity.traceBeginObject);
						ReadInstructionList(trace, insLists);
					}
					else
					{
						// if there is only one predecessor
						// run all instructions until we find the correct BeginObject or BeginObjectLeftRecursive instruction
						for (auto i = instruction; i >= 0; i--)
						{
							if (RunInstruction(i, insLists, objectCount))
							{
								instruction = i;
								return;
							}
						}

						// if not found, then we continue searching in the predecessor trace
						CHECK_ERROR(trace->predecessors.first != -1, ERROR_MESSAGE_PREFIX L"Encountered unbalanced instructions.");
						auto lastBranch = trace;

						trace = GetTrace(trace->predecessors.first);
						ReadInstructionList(trace, insLists);
						instruction = insLists.c3 - 1;
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void TraceManager::FillAmbiguityInfoForMergingTrace(Trace* trace)
			{
				// assuming that this is a ambiguity resolving trace
				// find the first instruction that accesses the object which is closed by the first EndObject in this trace
				// such instruction must be BeginObject
				// it is possible that the object closed by EndObject is created by a BeginObjectLeftRecursive
				// in this case we need to keep searching
				// until we find the BeginObject which creates the object that is consumed by BeginObjectLeftRecursive
				// by executing from such BeginObject instead of BeginObjectLeftRecursive for all branches
				// we prevent the object created by such BeginObject to be shared in multiple other objects

#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::SearchSingleTraceForBeginObject(Trace*&, vint&, vint&)#"
				// skip if the instruction has been found
				if (trace->ambiguity.traceBeginObject != -1)
				{
					return;
				}

				CHECK_ERROR(trace->predecessors.first != trace->predecessors.last, L"This function is not allowed to run on non-merging traces.");

				// find the first EndObject instruction
				TraceInsLists insLists;
				ReadInstructionList(trace, insLists);

				vint32_t insEndObject = -1;
				for (vint32_t i = 0; i < insLists.c3; i++)
				{
					auto& ins = ReadInstruction(i, insLists);
					if (ins.type == AstInsType::EndObject)
					{
						insEndObject = i;
						break;
					}
				}
				CHECK_ERROR(insEndObject != -1, ERROR_MESSAGE_PREFIX L"Cannot find EndObject instruction in the merging trace.");

				// run all instructions before and including the EndObject instruction
				// since we know EndObject addes 1 to the counter
				// so we don't really need to call RunInstruction on it
				// we could begin the counter from 1
				vint32_t objectCount = 1;
				for (vint32_t i = insEndObject - 1; i >= 0; i--)
				{
					if (RunInstruction(i, insLists, objectCount))
					{
						CHECK_FAIL(ERROR_MESSAGE_PREFIX L"BeginObject for the EndObject in the merging trace is impossible to be in the same trace.");
					}
				}

				vint32_t insBeginObject = -1;
				vint32_t traceBeginObject = -1;

				// call FindBalancedBeginObject on all predecessors
				auto predecessorId = trace->predecessors.first;
				while (predecessorId != -1)
				{
					auto predecessor = GetTrace(predecessorId);
					{
						TraceInsLists branchInsLists;
						ReadInstructionList(predecessor, branchInsLists);

						auto branchTrace = predecessor;
						vint32_t branchInstruction = branchInsLists.c3 - 1;
						vint32_t branchObjectCount = objectCount;
						FindBalancedBeginObject(branchTrace, branchInstruction, branchObjectCount);

						// the instruction found from different predecessors must be the same
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
				}

				// if the object closed by EndObject is created by BeginObjectLeftRecursive
				// we need to find the BeginObject which creates an object that is consumed by BeginObjectLeftRecursive
				{
					trace->ambiguity.insEndObject = insEndObject;

					auto currentTrace = GetTrace(traceBeginObject);
					vint32_t currentIns = insBeginObject;

					ReadInstructionList(currentTrace, insLists);
					auto ins = ReadInstruction(currentIns, insLists);
					trace->ambiguity.ambiguityType = ins.param;

					vint32_t objectCount = 0;
					// the object consumed by BeginObjectLeftRecursive could be created by a former BeginObjectLeftRecursive
					// we need to search until we reach the BeginObject instruction
					while (ins.type == AstInsType::BeginObjectLeftRecursive)
					{
						currentIns--;
						FindBalancedBeginObject(currentTrace, currentIns, objectCount);
						ReadInstructionList(currentTrace, insLists);
						ins = ReadInstruction(currentIns, insLists);
					}

					trace->ambiguity.insBeginObject = currentIns;
					trace->ambiguity.traceBeginObject = currentTrace->allocatedIndex;
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void TraceManager::FillAmbiguityInfoForPredecessorTraces(Trace* trace)
			{
				// fill Trace::ambiguity in any traces that could be reached by the current trace
				while (trace)
				{
					if (trace->predecessors.first != trace->predecessors.last)
					{
						// if an ambiguity resolving trace has been filled
						// then we could stop here
						// because a previous call should have visited from this trace all the way to the root trace
						if (trace->ambiguity.traceBeginObject == -1)
						{
							FillAmbiguityInfoForMergingTrace(trace);
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

				// we starts from all surviving traces
				// and visit all predecessors
				// until we reach the end
				// so that we could skip all failed traces

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

					// ensure that there is only one root trace
					if (visiting->predecessors.first == -1)
					{
						CHECK_ERROR(rootTraceCandidate == nullptr, L"vl::glr::automaton::TraceManager::PrepareTraceRoute()#Impossible to have more than one root trace.");
						rootTraceCandidate = visiting;
					}

					// add the current trace to its predecessors' successors collection
					// so that a succeeded trace only have other succeeded successors in its successor collection
					auto predecessorId = visiting->predecessors.first;
					while (predecessorId != -1)
					{
						auto predecessor = GetTrace(predecessorId);
						AddTraceToCollection(predecessor, visiting, &Trace::successors);
						predecessorId = predecessor->predecessors.siblingNext;
						visited.Add(predecessor);
					}
				}

				// find all ambiguity resolving traces and fill their Trace::ambiguity
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