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

/***********************************************************************
RunInstruction
***********************************************************************/

			bool TraceManager::RunInstruction(vint32_t instruction, TraceInsLists& insLists, vint32_t& objectCount, vint32_t& reopenCount)
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

				switch (ins.type)
				{
				case AstInsType::ReopenObject:
					reopenCount++;
					break;
				case AstInsType::DelayFieldAssignment:
					reopenCount--;
					break;
				}

				// if we found a ReopenObject
				// we should continue to search until we reach BeginObject or BeginObjectLeftRecursive
				return objectCount == 0 && (ins.type == AstInsType::BeginObject || ins.type == AstInsType::BeginObjectLeftRecursive);
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
FindBalancedBoOrBolr
***********************************************************************/

			void TraceManager::FindBalancedBoOrBolr(SharedBeginObject& balanced, vint32_t& objectCount, vint32_t& reopenCount)
			{
				// given the current instruction and the current constructing AST objects
				// find the nearlest BeginObject or BeginObjectLeftRecursive before the current instruction
				// that creates the bottom object
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::FindBalancedBoOrBolr(Trace*&, vint&, vint&)#"
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
							if (RunInstruction(i, insLists, objectCount, reopenCount))
							{
								instruction = i;
								return;
							}
						}

						// since the BeginObject instruction for this EndObject instruction will be executed after calling FillAmbiguityInfoForMergingTrace
						// must jump to the instruction before that BeginObject instruction

						if (objectCount == 0)
						{
							instruction = trace->ambiguity.insBeginObject;
							trace = GetTrace(trace->ambiguity.traceBeginObject);
							return;
						}
						else
						{
							instruction = trace->ambiguity.insBeginObject - 1;
							trace = GetTrace(trace->ambiguity.traceBeginObject);
							ReadInstructionList(trace, insLists);
						}
					}
					else
					{
						// if there is only one predecessor
						// run all instructions until we find the correct BeginObject or BeginObjectLeftRecursive instruction

						if (trace->ambiguityInsPostfix != -1)
						{
							vint32_t start = insLists.c1 - trace->ambiguityInsPostfix - 1;
							if (instruction > start) instruction = start;
						}

						for (auto i = instruction; i >= 0; i--)
						{
							if (RunInstruction(i, insLists, objectCount, reopenCount))
							{
								instruction = i;
								return;
							}
						}

						// if not found, then we continue searching in the predecessor trace
						CHECK_ERROR(trace->predecessors.first != -1, ERROR_MESSAGE_PREFIX L"Encountered unbalanced instructions.");

						trace = GetTrace(trace->predecessors.first);
						ReadInstructionList(trace, insLists);
						instruction = insLists.c3 - 1;
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
FindBalancedBeginObject
***********************************************************************/

			void TraceManager::FindBalancedBeginObject(Trace* trace, vint32_t objectCount, SharedBeginObject& branch)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::FindBalancedBeginObject(Trace*, vint32_t, Trace*&, vint32_t&, vint32_t&)#"
				// find the first balanced BeginObject or BeginObjectLeftRecursive
				TraceInsLists branchInsLists;
				ReadInstructionList(trace, branchInsLists);

				branchTrace = trace;
				branchInstruction = branchInsLists.c3 - 1;
				vint32_t branchObjectCount = objectCount;
				vint32_t branchReopenCount = 0;
				FindBalancedBoOrBolr(branchTrace, branchInstruction, branchObjectCount, branchReopenCount);

				// no matter if we found BeginObject or BeginObjectLeftRecursive
				// we now know what type of the AST we need to resolve
				ReadInstructionList(branchTrace, branchInsLists);
				auto ins = ReadInstruction(branchInstruction, branchInsLists);
				branchType = ins.param;

				// if we found a BeginObjectLeftRecursive which creates the bottom object in stack
				// then we should continue until we reach the BeginObject
				// because such BeginObject creates objects that eventually become part of BeginObjectLeftRecursive created objects
				// we cannot allow sharing the same child AST object in different parent AST objects.
				while (ins.type == AstInsType::BeginObjectLeftRecursive)
				{
					branchInstruction--;
					FindBalancedBoOrBolr(branchTrace, branchInstruction, branchObjectCount, branchReopenCount);
					ReadInstructionList(branchTrace, branchInsLists);
					ins = ReadInstruction(branchInstruction, branchInsLists);
				}

				// if branchReopenCount > 0
				// it means there must be this amount of DelayFieldAssignment instruction before BeginOpen
				// the first DelayFieldAssignment must be located
				if (branchReopenCount > 0)
				{
					ReadInstructionList(branchTrace, branchInsLists);
					while (true)
					{
						branchInstruction--;
						if (branchInstruction == -1)
						{
							// a merging trace must at least have one EndObject before the balanced BeginObject
							// so it is not possible to see it here
							CHECK_ERROR(branchTrace->predecessors.first == branchTrace->predecessors.last, ERROR_MESSAGE_PREFIX L"Unexpected merging trace when searching for DelayFieldAssignment.");
							CHECK_ERROR(branchTrace->predecessors.first != -1, ERROR_MESSAGE_PREFIX L"Unexpected root trace when searching for DelayFieldAssignment.");
							branchTrace = GetTrace(branchTrace->predecessors.first);
							ReadInstructionList(branchTrace, branchInsLists);
							branchInstruction = branchInsLists.c3;
						}
						else
						{
							auto& ins = ReadInstruction(branchInstruction, branchInsLists);
							switch (ins.type)
							{
							case AstInsType::EndObject:
								// if we see EndObject, find its balanced BeginObject or BeginObjectLeftRecursive
								FindBalancedBoOrBolr(branchTrace, branchInstruction, branchObjectCount, branchReopenCount);
								ReadInstructionList(branchTrace, branchInsLists);
								break;
							case AstInsType::ReopenObject:
								branchReopenCount++;
								break;
							case AstInsType::DelayFieldAssignment:
								branchReopenCount--;
								if (branchReopenCount == 0)
								{
									return;
								}
								break;
							}
						}
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
MergeAmbiguityType
***********************************************************************/

			void TraceManager::MergeAmbiguityType(vint32_t& ambiguityType, vint32_t branchType)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::MergeAmbiguityType(vint32_t&, vint32_t)#"
				if (ambiguityType == -1)
				{
					ambiguityType = branchType;
				}
				else if (typeCallback)
				{
					vint32_t newType = typeCallback->FindCommonBaseClass(ambiguityType, branchType);
					CHECK_ERROR(newType != -1, ERROR_MESSAGE_PREFIX L"Failed to merge from ambiguity types.");
					ambiguityType = newType;
				}
				else
				{
					CHECK_ERROR(ambiguityType == branchType, ERROR_MESSAGE_PREFIX L"TraceManager::ITypeCallback is not installed, unable to merge from ambiguity types.");
				}
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
FillAmbiguityInfoForMergingTrace
***********************************************************************/

			TraceManager::SharedBeginObject TraceManager::FillAmbiguityInfoForMergingTrace(Trace* trace)
			{
				// assuming that this is a ambiguity resolving trace
				// find the first instruction that accesses the object which is closed by the first EndObject in this trace
				// such instruction must be BeginObject
				// it is possible that the object closed by EndObject is created by a BeginObjectLeftRecursive
				// in this case we need to keep searching
				// until we find the BeginObject which creates the object that is consumed by BeginObjectLeftRecursive
				// by executing from such BeginObject instead of BeginObjectLeftRecursive for all branches
				// we prevent the object created by such BeginObject to be shared in multiple other objects

#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::FillAmbiguityInfoForMergingTrace(Trace*)#"
				// skip if the instruction has been found
				if (trace->ambiguity.traceBeginObject != -1)
				{
					SharedBeginObject shared;
					shared.traceBeginObject = GetTrace(trace->ambiguity.traceBeginObject);
					shared.insBeginObject = trace->ambiguity.insBeginObject;
					shared.type = trace->ambiguity.ambiguityType;
					return shared;
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

				SharedBeginObject shared;

				// call FindBalancedBoOrBolr on all predecessors
				auto predecessorId = trace->predecessors.first;
				while (predecessorId != -1)
				{
					auto predecessor = GetTrace(predecessorId);
					{
						// run all instructions before and including the EndObject instruction
						// since we know EndObject addes 1 to the counter
						// so we don't really need to call RunInstruction on it
						// we could begin the counter from 1

						SharedBeginObject branch;
						FindBalancedBeginObject(predecessor, 1, branch);

						// if EndObject is not the first instruction
						// then the all instruction prefix are stored in predecessors
						// so no need to really touch the prefix in this trace.

						MergeAmbiguityType(shared.type, branch.type);

						// BeginObject found from different predecessors must be the same
						// Otherwise, multiple BeginObject must belong to successors of the same trace, and the instructions prefix before these BeginObject must be identical
						if (shared.traceBeginObject == nullptr)
						{
							shared.traceBeginObject = branch.traceBeginObject;
							shared.insBeginObject = branch.insBeginObject;
						}
						else if (shared.traceBeginObject == branch.traceBeginObject)
						{
							CHECK_ERROR(shared.insBeginObject == branch.insBeginObject, ERROR_MESSAGE_PREFIX L"BeginObject searched from different branches are not the same.");
						}
						else
						{
							// ensure traces containing these BeginObject share the same predecessor
							TraceInsLists parentInsLists;
							Trace* parentTrace = shared.traceBeginObject;

#define ERROR_MESSAGE ERROR_MESSAGE_PREFIX L"Failed to merge prefix from BeginObject of multiple successors."
							CHECK_ERROR(branch.traceBeginObject->predecessors.first == branch.traceBeginObject->predecessors.last, ERROR_MESSAGE);
							if (parentTrace->allocatedIndex != branch.traceBeginObject->predecessors.first)
							{
								CHECK_ERROR(parentTrace->predecessors.first == parentTrace->predecessors.last, ERROR_MESSAGE);
								parentTrace = GetTrace(parentTrace->predecessors.first);

								ReadInstructionList(parentTrace, parentInsLists);
								shared.traceBeginObject = parentTrace;
								shared.insBeginObject += parentInsLists.c3;
							}
							else
							{
								CHECK_ERROR(parentTrace->allocatedIndex == branch.traceBeginObject->predecessors.last, ERROR_MESSAGE);
								ReadInstructionList(parentTrace, parentInsLists);
							}

							// ensure all instruction prefix before BeginObject are identical
							Trace* firstBranch = GetTrace(parentTrace->successors.first);
							CHECK_ERROR(firstBranch != branch.traceBeginObject, ERROR_MESSAGE);

							TraceInsLists firstBranchInsLists, branchInsLists;
							ReadInstructionList(firstBranch, firstBranchInsLists);
							ReadInstructionList(branch.traceBeginObject, branchInsLists);

							vint32_t firstInstruction = shared.insBeginObject - parentInsLists.c3;
							CHECK_ERROR(firstInstruction == branch.insBeginObject, ERROR_MESSAGE);
							for (vint32_t i = 0; i < firstInstruction; i++)
							{
								auto& ins1 = ReadInstruction(i, firstBranchInsLists);
								auto& ins2 = ReadInstruction(i, branchInsLists);
								CHECK_ERROR(ins1 == ins2, ERROR_MESSAGE);
							}
#undef ERROR_MESSAGE
						}
					}
					predecessorId = predecessor->predecessors.siblingNext;
				}

				{
					CHECK_ERROR(insEndObject == insLists.c1 - trace->ambiguityInsPostfix, L"ambiguityInsPostfix and insEndObject does not match.");
					trace->ambiguity.insEndObject = insEndObject;
					trace->ambiguity.insBeginObject = shared.insBeginObject;
					trace->ambiguity.traceBeginObject = shared.traceBeginObject->allocatedIndex;
					trace->ambiguity.ambiguityType = shared.type;
					return shared;
				}
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
FillAmbiguityInfoForPredecessorTraces
***********************************************************************/

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

/***********************************************************************
CreateLastMergingTrace
***********************************************************************/

			void TraceManager::CreateLastMergingTrace(Trace* rootTraceCandidate, vint32_t& ambiguityType)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CreateLastMergingTrace()#"

				// collect all successor traces from the root trace
				List<Trace*> orderedRootSuccessors;
				{
					vint32_t successorId = rootTraceCandidate->successors.first;
					while (successorId != -1)
					{
						auto successor = GetTrace(successorId);
						orderedRootSuccessors.Add(successor);
						successorId = successor->successors.siblingNext;
					}
				}

				// find the BeginObject instruction for all surviving traces
				Group<Trace*, Trace*> successorToSurvivings;
				Dictionary<Trace*, SharedBeginObject> ambiguities;

				for (vint i = 0; i < concurrentCount; i++)
				{
					auto survivingTrace = concurrentTraces->Get(i);

					// EndObject must be its last instruction
					TraceInsLists insLists;
					ReadInstructionList(survivingTrace, insLists);
					CHECK_ERROR(insLists.c3 > 0, ERROR_MESSAGE_PREFIX L"Last instruction is not EndObject.");

					auto&& lastIns = ReadInstruction(insLists.c3 - 1, insLists);
					CHECK_ERROR(lastIns.type == AstInsType::EndObject, ERROR_MESSAGE_PREFIX L"Last instruction is not EndObject.");

					// find the BeginObject instruction
					SharedBeginObject shared;
					FindBalancedBeginObject(survivingTrace, 0, shared);
					CHECK_ERROR(shared.traceBeginObject->predecessors.first == rootTraceCandidate->allocatedIndex, ERROR_MESSAGE_PREFIX L"The BeginObject instruction for the last EndObject instruction must locate in a successor trace from the root trace.");

					MergeAmbiguityType(ambiguityType, shared.type);
					successorToSurvivings.Add(shared.traceBeginObject, survivingTrace);
					ambiguities.Add(survivingTrace, shared);
				}

				// create merging traces for surviving traces that share the same traceBeginObject and insBeginObject
				List<Trace*> mergedSurvivingTraces;
				for (auto successor : orderedRootSuccessors)
				{
					// if this successor trace doesn't branch
					// put the surviving trace to the list
					// otherwise put the merging trace to the list

					auto&& survivings = successorToSurvivings.Get(successor);
					if (survivings.Count() == 1)
					{
						mergedSurvivingTraces.Add(survivings[0]);
					}
					else
					{
						// check if they are mergable
						auto&& firstAmbiguity = ambiguities[survivings[0]];
						for (auto survivingTrace : From(survivings).Skip(1))
						{
							auto&& nextAmbiguity = ambiguities[survivingTrace];
							CHECK_ERROR(
								firstAmbiguity.traceBeginObject == nextAmbiguity.traceBeginObject && firstAmbiguity.insBeginObject == nextAmbiguity.insBeginObject,
								ERROR_MESSAGE_PREFIX L"BeginObject searched from different surviving traces are not the same."
								);
						}

						// create a merging trace
						auto trace = AllocateTrace();
						mergedSurvivingTraces.Add(trace);
						{
							auto tid = trace->allocatedIndex;
							*trace = *survivings[0];
							trace->allocatedIndex = tid;
						}

						// ambiguityInsPostfix begins with the EndingObject instruction
						// which has been verified to be the last instruction in surviving traces
						// so ambiguityInsPostfix is always 1
						trace->ambiguityInsPostfix = 1;
						trace->ambiguity.traceBeginObject = firstAmbiguity.traceBeginObject->allocatedIndex;
						trace->ambiguity.insBeginObject = firstAmbiguity.insBeginObject;
						trace->ambiguity.ambiguityType = ambiguityType;
						{
							TraceInsLists insLists;
							ReadInstructionList(trace, insLists);
							trace->ambiguity.insEndObject = insLists.c3 - 1;
						}

						trace->predecessors.first = -1;
						trace->predecessors.last = -1;
						for (auto survivingTrace : survivings)
						{
							AddTraceToCollection(trace, survivingTrace, &Trace::predecessors);
							AddTraceToCollection(survivingTrace, trace, &Trace::successors);

							survivingTrace->executedReturnStack = -1;
							survivingTrace->ambiguityInsPostfix = 1;
						}
					}
				}

				// create a merging trace with no instruction
				auto trace = AllocateTrace();
				{
					auto survivingTrace = mergedSurvivingTraces[0];
					trace->state = survivingTrace->state;
					trace->currentTokenIndex = survivingTrace->currentTokenIndex;
				}

				for (auto survivingTrace : mergedSurvivingTraces)
				{
					AddTraceToCollection(trace, survivingTrace, &Trace::predecessors);
					AddTraceToCollection(survivingTrace, trace, &Trace::successors);

					// the root trace and the merging trace have no instruction
					// ExecuteTrace should handle this case correctly
					trace->ambiguity.insEndObject = 0;
					trace->ambiguity.traceBeginObject = rootTraceCandidate->allocatedIndex;
					trace->ambiguity.insBeginObject = 0;
					trace->ambiguity.ambiguityType = ambiguityType;
				}

				BeginSwap();
				AddTrace(trace);
				EndSwap();
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
PrepareTraceRoute
***********************************************************************/

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

				// if there are multiple surviving traces
				// check if the ambiguity happens in the root AST
				if (concurrentCount > 1)
				{
					vint32_t ambiguityType = -1;
					CreateLastMergingTrace(rootTraceCandidate, ambiguityType);
				}

				rootTrace = rootTraceCandidate;
				return rootTrace;
			}
		}
	}
}