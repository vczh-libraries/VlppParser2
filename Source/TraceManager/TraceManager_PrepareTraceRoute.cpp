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
AdjustToRealTrace
***********************************************************************/

			void TraceManager::AdjustToRealTrace(SharedBeginObject& shared)
			{
				TraceInsLists insLists;
				ReadInstructionList(shared.traceBeginObject, insLists);
				if (shared.insBeginObject >= insLists.c3)
				{
					shared.traceBeginObject = GetTrace(shared.traceBeginObject->successors.first);
					shared.insBeginObject -= insLists.c3;
				}
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
				ReadInstructionList(balanced.traceBeginObject, insLists);

				while (true)
				{
					if (balanced.traceBeginObject->predecessors.first != balanced.traceBeginObject->predecessors.last)
					{
						// if there are multiple predecessors
						// then this is a ambiguity resolving trace
						auto ambiguityBegin = FillAmbiguityInfoForMergingTrace(balanced.traceBeginObject);

						// execute all instructions until it reaches the first EndObject instruction
						// and this EndObject instruction is not executed
						for (auto i = balanced.insBeginObject; i > balanced.traceBeginObject->ambiguity.insEndObject; i--)
						{
							if (RunInstruction(i, insLists, objectCount, reopenCount))
							{
								balanced.insBeginObject = i;
								return;
							}
						}

						// since the BeginObject instruction for this EndObject instruction will be executed after calling FillAmbiguityInfoForMergingTrace
						// must jump to the instruction before that BeginObject instruction

						balanced = ambiguityBegin;
						if (objectCount == 0)
						{
							return;
						}
						else
						{
							AdjustToRealTrace(balanced);
							balanced.insBeginObject--;
							ReadInstructionList(balanced.traceBeginObject, insLists);
						}
					}
					else
					{
						// if there is only one predecessor
						// run all instructions until we find the correct BeginObject or BeginObjectLeftRecursive instruction

						vint32_t minIns = 0;
						vint32_t maxIns = insLists.c3;
						if (balanced.traceBeginObject->ambiguityMergeInsPostfix != -1)
						{
							minIns = insLists.c3 - balanced.traceBeginObject->ambiguityMergeInsPostfix;
						}
						if (balanced.traceBeginObject->ambiguityBranchInsPostfix != -1)
						{
							maxIns = insLists.c3 - balanced.traceBeginObject->ambiguityBranchInsPostfix - 1;
						}
						if (balanced.insBeginObject > maxIns)
						{
							balanced.insBeginObject = maxIns;
						}

						for (auto i = balanced.insBeginObject; i >= minIns; i--)
						{
							if (RunInstruction(i, insLists, objectCount, reopenCount))
							{
								balanced.insBeginObject = i;
								return;
							}
						}

						// if not found, then we continue searching in the predecessor trace
						CHECK_ERROR(balanced.traceBeginObject->predecessors.first != -1, ERROR_MESSAGE_PREFIX L"Encountered unbalanced instructions.");

						balanced.traceBeginObject = GetTrace(balanced.traceBeginObject->predecessors.first);
						ReadInstructionList(balanced.traceBeginObject, insLists);
						balanced.insBeginObject = insLists.c3 - 1;
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
FindBalancedBoOrDfa
***********************************************************************/

			void TraceManager::FindBalancedBoOrDfa(Trace* trace, vint32_t objectCount, SharedBeginObject& branch)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::FindBalancedBoOrDfa(Trace*, vint32_t, Trace*&, vint32_t&, vint32_t&)#"
				// find the first balanced BeginObject or BeginObjectLeftRecursive
				TraceInsLists branchInsLists;
				ReadInstructionList(trace, branchInsLists);
				branch.traceBeginObject = trace;
				branch.insBeginObject = branchInsLists.c3 - 1;
				vint32_t branchObjectCount = objectCount;
				vint32_t branchReopenCount = 0;
				FindBalancedBoOrBolr(branch, branchObjectCount, branchReopenCount);

				// no matter if we found BeginObject or BeginObjectLeftRecursive
				// we now know what type of the AST we need to resolve
				AdjustToRealTrace(branch);
				ReadInstructionList(branch.traceBeginObject, branchInsLists);
				auto ins = ReadInstruction(branch.insBeginObject, branchInsLists);
				branch.type = ins.param;

				// if we found a BeginObjectLeftRecursive which creates the bottom object in stack
				// then we should continue until we reach the BeginObject
				// because such BeginObject creates objects that eventually become part of BeginObjectLeftRecursive created objects
				// we cannot allow sharing the same child AST object in different parent AST objects.
				while (ins.type == AstInsType::BeginObjectLeftRecursive)
				{
					branch.insBeginObject--;
					FindBalancedBoOrBolr(branch, branchObjectCount, branchReopenCount);
					AdjustToRealTrace(branch);
					ReadInstructionList(branch.traceBeginObject, branchInsLists);
					ins = ReadInstruction(branch.insBeginObject, branchInsLists);
				}

				// if branchReopenCount > 0
				// it means there must be this amount of DelayFieldAssignment instruction before BeginOpen
				// the first DelayFieldAssignment must be located
				if (branchReopenCount > 0)
				{
					ReadInstructionList(branch.traceBeginObject, branchInsLists);
					while (true)
					{
						branch.insBeginObject--;
						if (branch.insBeginObject == -1)
						{
							// a merging trace must at least have one EndObject before the balanced BeginObject
							// so it is not possible to see it here

							CHECK_ERROR(
								branch.traceBeginObject->predecessors.first == branch.traceBeginObject->predecessors.last,
								ERROR_MESSAGE_PREFIX L"Unexpected merging trace when searching for DelayFieldAssignment."
								);
							CHECK_ERROR(
								branch.traceBeginObject->predecessors.first != -1,
								ERROR_MESSAGE_PREFIX L"Unexpected root trace when searching for DelayFieldAssignment."
								);

							branch.traceBeginObject = GetTrace(branch.traceBeginObject->predecessors.first);
							ReadInstructionList(branch.traceBeginObject, branchInsLists);
							branch.insBeginObject = branchInsLists.c3;
						}
						else
						{
							auto& ins = ReadInstruction(branch.insBeginObject, branchInsLists);
							switch (ins.type)
							{
							case AstInsType::EndObject:
								// if we see EndObject, find its balanced BeginObject or BeginObjectLeftRecursive
								FindBalancedBoOrBolr(branch, branchObjectCount, branchReopenCount);
								AdjustToRealTrace(branch);
								ReadInstructionList(branch.traceBeginObject, branchInsLists);
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
MergeSharedBeginObjectsSingleRoot
***********************************************************************/

			TraceManager::SharedBeginObject TraceManager::MergeSharedBeginObjectsSingleRoot(Trace* trace, collections::Dictionary<Trace*, SharedBeginObject>& predecessorToBranches)
			{
				// predecessorToBranches.Count() == 1
				// it means all values in predecessorToBranches must be identical

#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::MergeSharedBeginObjectsSingleRoot(Trace*, Dictionary<Trace*, SharedBeginObject>&)#"
				SharedBeginObject shared;
				vint32_t predecessorId = trace->predecessors.first;
				while (predecessorId != -1)
				{
					auto predecessor = GetTrace(predecessorId);
					auto branch = predecessorToBranches[predecessor];
					predecessorId = predecessor->predecessors.siblingNext;

					// BeginObject found from different predecessors must be the same
					if (shared.traceBeginObject == nullptr)
					{
						shared = branch;
					}
					else
					{
						CHECK_ERROR(shared.traceBeginObject == branch.traceBeginObject && shared.insBeginObject == branch.insBeginObject, ERROR_MESSAGE_PREFIX L"BeginObject searched from different branches are not the same.");
					}
				}

				shared.type = -1;
				return shared;
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
MergeSharedBeginObjectsMultipleRoot
***********************************************************************/

			TraceManager::SharedBeginObject TraceManager::MergeSharedBeginObjectsMultipleRoot(Trace* trace, collections::Dictionary<Trace*, SharedBeginObject>& predecessorToBranches)
			{
				// predecessorToBranches.Count() == number of predecessors
				// it means all values in predecessorToBranches must be identical
				// after they are adjusted to locate in the common predecessor

#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::MergeSharedBeginObjectsMultipleRoot(Trace*, Dictionary<Trace*, SharedBeginObject>&)#"
				SharedBeginObject shared;
				vint32_t predecessorId = trace->predecessors.first;

				Trace* firstBranch = nullptr;
				while (predecessorId != -1)
				{
					auto predecessor = GetTrace(predecessorId);
					auto branch = predecessorToBranches[predecessor];
					predecessorId = predecessor->predecessors.siblingNext;

					if (firstBranch == nullptr)
					{
						firstBranch = branch.traceBeginObject;
					}
					Trace* currentBranch = branch.traceBeginObject;

					// adjust branch to locate in its predecessor
					{
						TraceInsLists parentInsLists;
						Trace* parentTrace = GetTrace(branch.traceBeginObject->predecessors.first);

						ReadInstructionList(parentTrace, parentInsLists);
						branch.traceBeginObject = parentTrace;
						branch.insBeginObject += parentInsLists.c3;
					}

					// multiple BeginObject must belong to successors of the same trace
					// and the instructions prefix before these BeginObject must be identical
					if (shared.traceBeginObject == nullptr)
					{
						shared = branch;
					}
					else
					{
#define ERROR_MESSAGE ERROR_MESSAGE_PREFIX L"Failed to merge prefix from BeginObject of multiple successors."
						CHECK_ERROR(shared.traceBeginObject == branch.traceBeginObject && shared.insBeginObject == branch.insBeginObject, ERROR_MESSAGE);

						TraceInsLists sharedInsLists, firstInsLists, currentInsLists;
						ReadInstructionList(shared.traceBeginObject, sharedInsLists);
						ReadInstructionList(firstBranch, firstInsLists);
						ReadInstructionList(currentBranch, currentInsLists);

						vint32_t insBeginObject = shared.insBeginObject - sharedInsLists.c3;
						CHECK_ERROR(insBeginObject < firstInsLists.c3, ERROR_MESSAGE);
						CHECK_ERROR(insBeginObject < currentInsLists.c3, ERROR_MESSAGE);

						for (vint32_t i = 0; i < insBeginObject; i++)
						{
							auto& ins1 = ReadInstruction(i, firstInsLists);
							auto& ins2 = ReadInstruction(i, currentInsLists);
							CHECK_ERROR(ins1 == ins2, ERROR_MESSAGE);
						}
#undef ERROR_MESSAGE
					}
				}

				shared.type = -1;
				return shared;
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
MergeSharedBeginObjectsMultipleRoot
***********************************************************************/

			TraceManager::SharedBeginObject TraceManager::MergeSharedBeginObjectsPartialMultipleRoot(Trace* trace, vint32_t ambiguityType, collections::Group<Trace*, Trace*>& beginToPredecessors, collections::Dictionary<Trace*, SharedBeginObject>& predecessorToBranches)
			{
				// some values in predecessorToBranches are the same but some are not
				// the result is the same to one when all values in predecessorToBranches are different
				// but we need to merge subset of predecessors which share the same value

#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::MergeSharedBeginObjectsPartialMultipleRoot(Trace*, vint32_t, Group<Trace*, Trace*>&, Dictionary<Trace*, SharedBeginObject>&)#"
				vint32_t predecessorId = trace->predecessors.first;
				while (predecessorId != -1)
				{
					auto predecessor = GetTrace(predecessorId);
					auto branch = predecessorToBranches[predecessor];
					predecessorId = predecessor->predecessors.siblingNext;

					// we start with the first predecessor from all subset
					auto& subset = beginToPredecessors[branch.traceBeginObject];
					if (subset.Count()==1 || predecessor != subset[0]) continue;

#define ERROR_MESSAGE ERROR_MESSAGE_PREFIX L"Failed to merge prefix from BeginObject of multiple successors."
					// ensure all value in the subset are identical
					for (vint i = 1; i < subset.Count(); i++)
					{
						auto anotherBranch = predecessorToBranches[subset[i]];
						CHECK_ERROR(branch.traceBeginObject == anotherBranch.traceBeginObject && branch.insBeginObject == anotherBranch.insBeginObject, ERROR_MESSAGE);
					}

					// ensure all predecessor in the subset are identical in their critical content
					TraceInsLists insLists;
					ReadInstructionList(predecessor, insLists);
					for (vint i = 1; i < subset.Count(); i++)
					{
						auto anotherPredecessor = subset[i];
						CHECK_ERROR(predecessor->state == anotherPredecessor->state, ERROR_MESSAGE);
						CHECK_ERROR(predecessor->byInput == anotherPredecessor->byInput, ERROR_MESSAGE);
						CHECK_ERROR(predecessor->currentTokenIndex == anotherPredecessor->currentTokenIndex, ERROR_MESSAGE);
						CHECK_ERROR(predecessor->returnStack == anotherPredecessor->returnStack, ERROR_MESSAGE);

						TraceInsLists anotherInsLists;
						ReadInstructionList(anotherPredecessor, anotherInsLists);
						CHECK_ERROR(insLists.c3 == anotherInsLists.c3, ERROR_MESSAGE);
						for (vint32_t j = 0; j < insLists.c3; j++)
						{
							auto& ins1 = ReadInstruction(j, insLists);
							auto& ins2 = ReadInstruction(j, anotherInsLists);
							CHECK_ERROR(ins1 == ins2, ERROR_MESSAGE);
						}
					}

					// ensure all predecessor in the subset has only one predecessor
					// otherwise we are replacing the current issue with another same issue
					for (vint i = 0; i < subset.Count(); i++)
					{
						auto anotherPredecessor = subset[i];
						CHECK_ERROR(anotherPredecessor->predecessors.first == anotherPredecessor->predecessors.last, ERROR_MESSAGE);
					}

					// connect all predecessors of predecessors in the subset to the first one
					for (vint i = 1; i < subset.Count(); i++)
					{
						auto p1 = subset[i];
						auto p2 = GetTrace(p1->predecessors.first);
						p2->successors = {};
						AddTraceToCollection(predecessor, p2, &Trace::predecessors);
						AddTraceToCollection(p2, predecessor, &Trace::successors);
					}

					// remove all predecessors in the subset except the first one
					for (vint i = 1; i < subset.Count(); i++)
					{
						auto p = subset[i];
						if (p->predecessors.siblingPrev != -1)
						{
							GetTrace(p->predecessors.siblingPrev)->predecessors.siblingNext = p->predecessors.siblingNext;
						}
						if (p->predecessors.siblingNext != -1)
						{
							GetTrace(p->predecessors.siblingNext)->predecessors.siblingPrev = p->predecessors.siblingPrev;
						}
						if (trace->predecessors.first == p->allocatedIndex)
						{
							trace->predecessors.first = p->predecessors.siblingNext;
						}
						if (trace->predecessors.last == p->allocatedIndex)
						{
							trace->predecessors.last = p->predecessors.siblingPrev;
						}
					}

					// fix predecessor->ambiguity
					predecessor->ambiguity.traceBeginObject = branch.traceBeginObject->allocatedIndex;
					predecessor->ambiguity.insBeginObject = branch.insBeginObject;
					predecessor->ambiguity.ambiguityType = ambiguityType;

					for (vint32_t i = 0; i < insLists.c3; i++)
					{
						auto& ins = ReadInstruction(i, insLists);
						if (ins.type == AstInsType::EndObject)
						{
							predecessor->ambiguity.insEndObject = i;
							break;
						}
					}
					CHECK_ERROR(predecessor->ambiguity.insEndObject != -1, ERROR_MESSAGE);
#undef ERROR_MESSAGE
				}
				return MergeSharedBeginObjectsMultipleRoot(trace, predecessorToBranches);
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
				if (trace->ambiguityMergeInsPostfix != -1)
				{
					CHECK_ERROR(insEndObject == insLists.c1 - trace->ambiguityMergeInsPostfix - 1, L"ambiguityMergeInsPostfix and insEndObject does not match.");
				}

				vint32_t ambiguityType = -1;
				Group<Trace*, Trace*> beginToPredecessors;
				Dictionary<Trace*, SharedBeginObject> predecessorToBranches;

				// call FindBalancedBoOrBolr on all predecessors
				auto predecessorId = trace->predecessors.first;
				vint predecessorCount = 0;
				while (predecessorId != -1)
				{
					predecessorCount++;
					auto predecessor = GetTrace(predecessorId);
					// run all instructions before and including the EndObject instruction
					// since we know EndObject addes 1 to the counter
					// so we don't really need to call RunInstruction on it
					// we could begin the counter from 1

					SharedBeginObject branch;
					FindBalancedBoOrDfa(predecessor, 1, branch);

					beginToPredecessors.Add(branch.traceBeginObject, predecessor);
					predecessorToBranches.Add(predecessor, branch);

					// if EndObject is not the first instruction
					// then the all instruction prefix are stored in predecessors
					// so no need to really touch the prefix in this trace.

					MergeAmbiguityType(ambiguityType, branch.type);

					predecessorId = predecessor->predecessors.siblingNext;
				}

				SharedBeginObject shared;
				if (beginToPredecessors.Count() == 1)
				{
					shared = MergeSharedBeginObjectsSingleRoot(trace, predecessorToBranches);
				}
				else if (beginToPredecessors.Count() == predecessorCount)
				{
					shared = MergeSharedBeginObjectsMultipleRoot(trace, predecessorToBranches);
				}
				else
				{
					shared = MergeSharedBeginObjectsPartialMultipleRoot(trace, ambiguityType, beginToPredecessors, predecessorToBranches);
				}

				trace->ambiguity.insEndObject = insEndObject;
				trace->ambiguity.insBeginObject = shared.insBeginObject;
				trace->ambiguity.traceBeginObject = shared.traceBeginObject->allocatedIndex;
				trace->ambiguity.ambiguityType = ambiguityType;
				return shared;
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

			void TraceManager::CreateLastMergingTrace(Trace* rootTraceCandidate)
			{
				// if there are multiple surviving traces
				// they are all EndingInput transition to the ending state
				// and their last instruction are EndObject
				// so we could merge every surviving trace to one

				// first, we need to determine the ambiguity type
				vint32_t ambiguityType = -1;
				for (vint i = 0; i < concurrentCount; i++)
				{
					auto trace = concurrentTraces->Get(i);
					if (trace->predecessors.first == trace->predecessors.last)
					{
						// if this trace has only one predecessor
						// find its BeginObject or BeginObjectLeftRecursive instruction for the type
						TraceInsLists insLists;
						ReadInstructionList(trace, insLists);

						SharedBeginObject balanced;
						balanced.traceBeginObject = trace;
						balanced.insBeginObject = insLists.c3 - 1;
						vint32_t objectCount = 0;
						vint32_t reopenCount = 0;
						FindBalancedBoOrBolr(balanced, objectCount, reopenCount);

						AdjustToRealTrace(balanced);
						ReadInstructionList(balanced.traceBeginObject, insLists);
						auto ins = ReadInstruction(balanced.insBeginObject, insLists);
						MergeAmbiguityType(ambiguityType, ins.param);
					}
					else
					{
						// otherwise, the type has been calculated before
						MergeAmbiguityType(ambiguityType, trace->ambiguity.ambiguityType);
					}
				}

				// second, create a merging ending trace
				// such merging ending trace has no instruction
				// it just merges all ending traces
				auto mergingTrace = AllocateTrace();
				for (vint i = 0; i < concurrentCount; i++)
				{
					auto trace = concurrentTraces->Get(i);
					if (i == 0)
					{
						// copy data from the first one
						mergingTrace->state = trace->state;
						mergingTrace->byInput = trace->byInput;
						mergingTrace->currentTokenIndex = trace->currentTokenIndex;

						// set the ambiguity data
						// rootTraceCandidate has no instruction
						// the first instructions of all successors are all we need
						TraceInsLists insLists;
						ReadInstructionList(mergingTrace, insLists);
						mergingTrace->ambiguity.traceBeginObject = rootTraceCandidate->allocatedIndex;
						mergingTrace->ambiguity.insBeginObject = 0;
						mergingTrace->ambiguity.insEndObject = 0;
						mergingTrace->ambiguity.ambiguityType = ambiguityType;
					}

					AddTraceToCollection(mergingTrace, trace, &Trace::predecessors);
					AddTraceToCollection(trace, mergingTrace, &Trace::successors);
				}

				// finally, the new merging trace should be the only surviving trace
				concurrentCount = 1;
				concurrentTraces->Set(0, mergingTrace);
				for (vint i = 1; i < concurrentTraces->Count(); i++)
				{
					concurrentTraces->Set(i, nullptr);
				}
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
					CreateLastMergingTrace(rootTraceCandidate);
				}

				rootTrace = rootTraceCandidate;
				return rootTrace;
			}
		}
	}
}