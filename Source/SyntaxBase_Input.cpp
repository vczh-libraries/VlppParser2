#include "SyntaxBase.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{

/***********************************************************************
Resolving Ambiguity
***********************************************************************/

			bool TraceManager::AreReturnDescEqual(vint32_t ri1, vint32_t ri2)
			{
				// two returns equal to each other if
				//   1) they shares the same id, so we are comparing a return with itself
				//   2) they have exactly the same data

				// we cannot just compare ri1 == ri2 because
				// if two alternative branches ends with the same rule and same instructions
				// then two ReturnDesc will have the same data in it
				// TODO: verify (this function could be deleted if AreReturnStackEqual doesn't need it anymore)

				if (ri1 == ri2) return true;
				auto& rd1 = executable.returns[ri1];
				auto& rd2 = executable.returns[ri2];
				if (rd1.returnState != rd2.returnState) return false;
				if (rd1.insAfterInput.count != rd2.insAfterInput.count) return false;
				for (vint insRef = 0; insRef < rd1.insAfterInput.count; insRef++)
				{
					auto& ins1 = executable.instructions[rd1.insAfterInput.start + insRef];
					auto& ins2 = executable.instructions[rd2.insAfterInput.start + insRef];
					if (ins1 != ins2) return false;
				}
				return true;
			}

			bool TraceManager::AreReturnStackEqual(vint32_t r1, vint32_t r2)
			{
				return r1 == r2;

				// two return stacks equal to each other if
				//   1) they shares the same id, so we are comparing a return stack with itself
				//   2) both top returns equal, and both remaining return stack equals

				// could we just compare r1 == r2 (TODO: verify)
				// Ambiguity resolving requires different branchs should share
				// BeginObject, BeginObjectLeftRecursive and EndObject in exactly the same place (trace + ins)
				// so their return stack should just be the same object

				// TODO: when ambiguity is created because two left recursive clauses consume the same series of inputs
				// then the BeginObjectLeftRecursive could belong to different traces
				// maybe we should just compare the BeginObject before merging branches
				// instead of try to find the BeginObject from BeginObjectLeftRecursive after merging branches

				// TODO: is it possible that we must (or not just could) compare r1 == r2?
				// try to build this case

				while (true)
				{
					if (r1 == r2) return true;
					if (r1 == -1 || r2 == -1) return false;
					auto rs1 = GetReturnStack(r1);
					auto rs2 = GetReturnStack(r2);
					if (!AreReturnDescEqual(rs1->returnIndex, rs2->returnIndex))
					{
						return false;
					}
					r1 = rs1->previous;
					r2 = rs2->previous;
				}
			}

			bool TraceManager::AreTwoTraceEqual(vint32_t state, vint32_t returnStack, vint32_t executedReturn, vint32_t acId, Trace* candidate)
			{
				// two traces equal to each other if
				//   1) they are in the same state
				//   2) they have the same executedReturn
				//   3) they are attending same competitions
				//   4) they have the same return stack
				if (state == candidate->state &&
					executedReturn == candidate->executedReturn &&
					acId == candidate->runtimeRouting.attendingCompetitions)
				{
					auto r1 = returnStack;
					auto r2 = candidate->returnStack;
					if (AreReturnStackEqual(r1, r2))
					{
						return true;
					}
				}
				return false;
			}

			vint32_t TraceManager::GetInstructionPostfix(EdgeDesc& oldEdge, EdgeDesc& newEdge)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::GetInstructionPostfix(EdgeDesc&, EdgeDesc&)#"
				// given two equal traces, calculate their common instruction postfix length
				// EndObject is the first instruction of the postfix

				// EndObject may not be the first instruction in both edges
				// and instructions before EndObject could be different
				// the most common case is different {field = value} before EndObject
				// if the ambiguity is created by two left recursive clauses which consume the same series of tokens

				CHECK_ERROR(oldEdge.insAfterInput.count == 0, ERROR_MESSAGE_PREFIX L"EndingInput edge is not allowed to have insAfterInput.");
				CHECK_ERROR(newEdge.insAfterInput.count == 0, ERROR_MESSAGE_PREFIX L"EndingInput edge is not allowed to have insAfterInput.");

				// find the first EndObject instruction
				vint32_t i1 = -1;
				vint32_t i2 = -1;

				for (vint32_t insRef = 0; insRef < oldEdge.insBeforeInput.count; insRef++)
				{
					auto&& ins = executable.instructions[oldEdge.insBeforeInput.start + insRef];
					if (ins.type == AstInsType::EndObject)
					{
						i1 = insRef;
						break;
					}
				}

				for (vint32_t insRef = 0; insRef < newEdge.insBeforeInput.count; insRef++)
				{
					auto&& ins = executable.instructions[newEdge.insBeforeInput.start + insRef];
					if (ins.type == AstInsType::EndObject)
					{
						i2 = insRef;
						break;
					}
				}

				CHECK_ERROR(i1 != -1, ERROR_MESSAGE_PREFIX L"EndObject from oldEdge not found.");
				CHECK_ERROR(i2 != -1, ERROR_MESSAGE_PREFIX L"EndObject from newEdge not found.");

				// ensure they have the same instruction postfix starting from EndObject
				CHECK_ERROR(oldEdge.insBeforeInput.count - i1 == newEdge.insBeforeInput.count - i2, L"Two instruction postfix after EndObject not equal.");

				vint32_t postfix = oldEdge.insBeforeInput.count - i1;
				for (vint32_t postfixRef = 0; postfixRef < postfix; postfix++)
				{
					auto&& ins1 = executable.instructions[oldEdge.insBeforeInput.start + i1 + postfixRef];
					auto&& ins2 = executable.instructions[newEdge.insBeforeInput.start + i2 + postfixRef];
					CHECK_ERROR(ins1 == ins2, L"Two instruction postfix after EndObject not equal.");
				}
				return postfix;
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
Competitions
***********************************************************************/

			vint32_t TraceManager::AttendCompetitionIfNecessary(Trace* trace, EdgeDesc& edgeDesc)
			{
				// attend a competition if the priority of the transition is set
				if (edgeDesc.priority != EdgePriority::NoCompetition)
				{
					// check if a competition object has been created for this trace
					Competition* competition = nullptr;
					if (trace->runtimeRouting.holdingCompetition == -1)
					{
						competition = AllocateCompetition();
						competition->ownerTrace = trace->allocatedIndex;
						trace->runtimeRouting.holdingCompetition = competition->allocatedIndex;

						competition->next = activeCompetitions;
						activeCompetitions = competition->allocatedIndex;
					}
					else
					{
						competition = GetCompetition(trace->runtimeRouting.holdingCompetition);
					}

					// target traces from the current trace should attend all competitions that the current trace attends
					// so only one AttendingCompetitions object needs to be created per bet
					// it is fine for different traces share all or part of AttendingCompetitions in their RuntimeRouting::attendingCompetitions linked list
					// because if a competition is settled in the future
					// AttendingCompetitions objects for this competition is going to be removed anyway
					// sharing a linked list doesn't change the result

					switch (edgeDesc.priority)
					{
					case EdgePriority::HighPriority:
						if (competition->highBet == -1)
						{
							// create an AttendingCompetitions for this competition for high priority bet if it is not created
							auto ac = AllocateAttendingCompetitions();
							ac->next = trace->runtimeRouting.attendingCompetitions;
							ac->competition = competition->allocatedIndex;
							ac->forHighPriority = true;
							competition->highBet = ac->allocatedIndex;
						}
						return competition->highBet;
					case EdgePriority::LowPriority:
						if (competition->lowBet == -1)
						{
							// create an AttendingCompetitions for this competition for high priority bet if it is not created
							auto ac = AllocateAttendingCompetitions();
							ac->next = trace->runtimeRouting.attendingCompetitions;
							ac->competition = competition->allocatedIndex;
							ac->forHighPriority = false;
							competition->lowBet = ac->allocatedIndex;
						}
						return competition->lowBet;
					}
				}
				return trace->runtimeRouting.attendingCompetitions;
			}

			void TraceManager::CheckAttendingCompetitionsOnEndingEdge(Trace* trace, EdgeDesc& edgeDesc, vint32_t acId, vint32_t returnStack)
			{
				while (acId != -1)
				{
					// when executing an EndingInput transition, we announce high priority win a competition if
					//   1) such EndingInput transitions ends the clause where the state of the trace holding competition is in the same clause
					//      we ensure this by comparing both returnStack object (not content)
					//      because a ReturnStack object is created when entering a new clause
					//   2) if the EndingInput transition begins from the trace holding the competition, it cannot be a low priority transition
					//      visiting such transitions only mean a low priority trace survives the clause
					//   3) the competition has not been settled
					auto ac = GetAttendingCompetitions(acId);
					auto cpt = GetCompetition(ac->competition);
					auto cptr = GetTrace(cpt->ownerTrace);
					if (cptr->returnStack == returnStack)
					{
						if (cptr != trace || edgeDesc.priority != EdgePriority::LowPriority)
						{
							if (cpt->status != CompetitionStatus::LowPriorityWin)
							{
								cpt->status = CompetitionStatus::HighPriorityWin;
								break;
							}
						}
					}
					acId = ac->next;
				}
			}

			void TraceManager::CheckBackupTracesBeforeSwapping(vint32_t currentTokenIndex)
			{
				// try to find if any competition could be settled at this moment

				{
					// reset highCounter and lowCounter for any active competitions
					auto cId = activeCompetitions;
					while (cId != -1)
					{
						auto cpt = GetCompetition(cId);
						cpt->highCounter = 0;
						cpt->lowCounter = 0;
						cId = cpt->next;
					}
				}

				// for any surviving traces
				// add itself to the appriopriate counter for all attending competitions
				for (vint i = 0; i < concurrentCount; i++)
				{
					auto trace = backupTraces->Get(i);
					auto acId = trace->runtimeRouting.attendingCompetitions;
					while (acId != -1)
					{
						auto ac = GetAttendingCompetitions(acId);
						auto cpt = GetCompetition(ac->competition);
						(ac->forHighPriority ? cpt->highCounter : cpt->lowCounter)++;
						acId = ac->next;
					}
				}

				// revisit all active competitions
				// some competitions could have been settled
				// but settled competitions will only be removed before consuming the next token
				{
					auto cId = activeCompetitions;
					while (cId != -1)
					{
						auto cpt = GetCompetition(cId);
						if (cpt->status == CompetitionStatus::Holding)
						{
							if (cpt->highCounter > 0 && cpt->lowCounter == 0)
							{
								// if only high bet traces survive, high priority win
								cpt->status = CompetitionStatus::HighPriorityWin;
							}
							else if (cpt->highCounter == 0 && cpt->lowCounter > 0)
							{
								// if only low bet traces survive, low priority win
								// after at least one token is consumed from when the competition is created
								// low priority epsilon transitions could have been visited right after a competition is created
								// but high priority token transitions could only be visited when consuming the next token
								// if all high priority transitions are token token transitions
								// and all low priority transitions are epsilon transitions
								// closing the competition too early will direct to a wrong result
								// so we need to wait at least one step to see if any trace will visit the high priority transition in the future
								auto cptr = GetTrace(cpt->ownerTrace);
								if (cptr->currentTokenIndex != currentTokenIndex)
								{
									cpt->status = CompetitionStatus::LowPriorityWin;
								}
							}
						}
						cId = cpt->next;
					}
				}

				// for any surviving traces
				// if it loses any one of its attending competitions
				// this trace will be removed
				for (vint i = concurrentCount - 1; i >= 0; i--)
				{
					auto trace = backupTraces->Get(i);
					auto acId = trace->runtimeRouting.attendingCompetitions;
					while (acId != -1)
					{
						auto ac = GetAttendingCompetitions(acId);
						auto cpt = GetCompetition(ac->competition);
						if (cpt->status != CompetitionStatus::Holding)
						{
							ac->closed = true;
							if (ac->forHighPriority != (cpt->status == CompetitionStatus::HighPriorityWin))
							{
								concurrentCount--;
								backupTraces->RemoveAt(i);
								goto TRACE_REMOVED;
							}
						}
						acId = ac->next;
					}
				TRACE_REMOVED:;
				}

				// remove all settled competition from the active competitions linked list
				{
					vint32_t* pnext = &activeCompetitions;
					while (*pnext != -1)
					{
						auto cpt = GetCompetition(*pnext);
						if (cpt->status != CompetitionStatus::Holding || (cpt->highCounter == 0 && cpt->lowCounter == 0))
						{
							*pnext = cpt->next;
						}
						else
						{
							pnext = &cpt->next;
						}
					}
				}

				// remove all settled AttendingCompetitions object from linked lists of any surviving trace
				for (vint i = 0; i < concurrentCount; i++)
				{
					auto trace = backupTraces->Get(i);
					vint32_t* pnext = &trace->runtimeRouting.attendingCompetitions;
					while (*pnext != -1)
					{
						auto ac = GetAttendingCompetitions(*pnext);
						if (ac->closed)
						{
							*pnext = ac->next;
						}
						else
						{
							pnext = &ac->next;
						}
					}
				}
			}

/***********************************************************************
TraceManager::WalkAlongSingleEdge
***********************************************************************/

			Trace* TraceManager::WalkAlongSingleEdge(
				vint32_t currentTokenIndex,
				vint32_t input,
				Trace* trace,
				vint32_t byEdge,
				EdgeDesc& edgeDesc
			)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::WalkAlongSingleEdge(vint, vint, vint, Trace*, vint, EdgeDesc&)#"
				vint32_t state = edgeDesc.toState;
				vint32_t returnStack = trace->returnStack;
				vint32_t executedReturn = -1;
				Trace* ambiguityTraceToMerge = nullptr;

				// attend a competition hold by the current trace if the priority is set for this output transition
				vint32_t acId = AttendCompetitionIfNecessary(trace, edgeDesc);

				if (input == Executable::EndingInput)
				{
					// an EndingInput transition consume return record in the return stack
					// such return will be popped from the return stack and stored in Trace::executedReturn
					CHECK_ERROR(edgeDesc.returnIndices.count == 0, ERROR_MESSAGE_PREFIX L"Ending input edge is not allowed to push something into the return stack.");
					if (returnStack != -1)
					{
						auto rs = GetReturnStack(returnStack);
						returnStack = rs->previous;
						executedReturn = rs->returnIndex;
						state = executable.returns[executedReturn].returnState;
					}

					// an EndingInput transition also settle a competition if
					//   1) there is a competition
					//   2) the returnStack of the trace holding the competition is the same to the current returnStack
					//   3) the target trace bets high priority
					// in this case, high priority traces wins the competition
					// but no traces are being removed for now, just mark the competition
					CheckAttendingCompetitionsOnEndingEdge(trace, edgeDesc, acId, trace->returnStack);

					// if the target trace has exactly the same to another surviving trace
					// stop creating a Trace instance for the target trace
					// instead connect the correct trace to that surviving trace and form a ambiguity resolving structure
					for (vint i = 0; i < concurrentCount; i++)
					{
						auto candidate = backupTraces->Get(i);
						if (AreTwoTraceEqual(state, returnStack, executedReturn, acId, candidate))
						{
							ambiguityTraceToMerge = candidate;
							break;
						}
					}
				}

				if (ambiguityTraceToMerge)
				{
					// if ambiguity resolving happens
					// find the instruction postfix
					// the instruction postfix starts from EndObject of a trace
					// and both instruction postfix should equal
					auto& oldEdge = executable.edges[ambiguityTraceToMerge->byEdge];
					vint32_t postfix = GetInstructionPostfix(oldEdge, edgeDesc);

					if (ambiguityTraceToMerge->ambiguityInsPostfix == -1)
					{
						if (oldEdge.insBeforeInput.count == postfix)
						{
							// if EndObject is the first instruction
							// no need to insert another trace
							ambiguityTraceToMerge->ambiguityInsPostfix = postfix;
						}
						else
						{
							// if EndObject is not the first instruction
							// insert another trace before ambiguityTraceMerge
							// and ambiguityTraceMerge should not have had multiple predecessors at this moment
							CHECK_ERROR(ambiguityTraceToMerge->predecessors.first == ambiguityTraceToMerge->predecessors.last, ERROR_MESSAGE_PREFIX L"An ambiguity resolving traces should have been cut.");

							// although executedReturn is executed by EndObject
							// but executedReturn is stored in the first trace
							// and EndObject is available in the second trace
							// so newTrace->executedReturn should be -1

							auto formerTrace = AllocateTrace();
							{
								vint32_t formerId = formerTrace->allocatedIndex;
								*formerTrace = *ambiguityTraceToMerge;
								formerTrace->allocatedIndex = formerId;
							}

							// ambiguity is filled by PrepareTraceRoute, skipped
							// runtimeRouting.holdingCompetition always belong to the second trace
							// runtimeRouting.attendingCompetitions is inherited
							formerTrace->runtimeRouting = {};
							formerTrace->runtimeRouting.attendingCompetitions = ambiguityTraceToMerge->runtimeRouting.attendingCompetitions;

							// both traces need to have the same ambiguityInsPostfix
							formerTrace->ambiguityInsPostfix = postfix;
							ambiguityTraceToMerge->ambiguityInsPostfix = postfix;

							// connect two traces
							// formerTrace has already copied predecessors, skipped
							// successors of both traces are filled byPrepareTraceRoute, skipped
							// insert formerTrace before ambiguityTraceToMerge because
							// we don't successors of ambiguityTraceToMerge, cannot redirect their predecessors
							ambiguityTraceToMerge->predecessors.first = formerTrace->allocatedIndex;
							ambiguityTraceToMerge->predecessors.last = formerTrace->allocatedIndex;
						}
					}

					if (edgeDesc.insBeforeInput.count == postfix)
					{
						// if EndObject is the first instruction of the new trace
						// then no need to create the new trace
						AddTraceToCollection(ambiguityTraceToMerge, trace, &Trace::predecessors);
					}
					else
					{

					}

					return nullptr;
				}
				else
				{
					// if ambiguity resolving doesn't happen
					// create an instance of the target trace
					// and connect the current trace to this target trace
					auto newTrace = AllocateTrace();
					AddTrace(newTrace);

					newTrace->predecessors.first = trace->allocatedIndex;
					newTrace->predecessors.last = trace->allocatedIndex;
					newTrace->state = state;
					newTrace->returnStack = returnStack;
					newTrace->executedReturn = executedReturn;
					newTrace->byEdge = byEdge;
					newTrace->byInput = input;
					newTrace->currentTokenIndex = currentTokenIndex;
					newTrace->runtimeRouting.attendingCompetitions = acId;

					// push returns to the return stack if the transition requires
					for (vint returnRef = 0; returnRef < edgeDesc.returnIndices.count; returnRef++)
					{
						auto returnIndex = executable.returnIndices[edgeDesc.returnIndices.start + returnRef];
						auto returnStack = AllocateReturnStack();
						returnStack->previous = newTrace->returnStack;
						returnStack->returnIndex = returnIndex;
						newTrace->returnStack = returnStack->allocatedIndex;
					}

					return newTrace;
				}
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
TraceManager::WalkAlongTokenEdges
***********************************************************************/

			void TraceManager::WalkAlongLeftrecEdges(
				vint32_t currentTokenIndex,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint32_t byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					WalkAlongSingleEdge(currentTokenIndex, Executable::LeftrecInput, trace, byEdge, edgeDesc);

					// A LeftrecInput transition points to a non ending state in another clause
					// so there is no need to find other epsilon transitions after LeftrecInput
				}
			}

			void TraceManager::WalkAlongEndingEdges(
				vint32_t currentTokenIndex,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint32_t byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					if (auto newTrace = WalkAlongSingleEdge(currentTokenIndex, Executable::EndingInput, trace, byEdge, edgeDesc))
					{
						// EndingInput could be followed by EndingInput or LeftrecInput
						WalkAlongEpsilonEdges(currentTokenIndex, newTrace);
					}
				}
			}

			void TraceManager::WalkAlongEpsilonEdges(
				vint32_t currentTokenIndex,
				Trace* trace
			)
			{
				{
					// LeftrecInput transition is an epsilon transition
					vint32_t transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + Executable::LeftrecInput;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongLeftrecEdges(currentTokenIndex, trace, edgeArray);
				}
				{
					// EndingInput transition is an epsilon transition
					vint32_t transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + Executable::EndingInput;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongEndingEdges(currentTokenIndex, trace, edgeArray);
				}
			}

			void TraceManager::WalkAlongTokenEdges(
				vint32_t currentTokenIndex,
				vint32_t input,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				// find all transitions that has the expected input
				// there could be multiple transitions with the same input
				// but with different instructions and destinations
				for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint32_t byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					if (auto newTrace = WalkAlongSingleEdge(currentTokenIndex, input, trace, byEdge, edgeDesc))
					{
						// continue with as much EndingInput and LeftrecInput transitions as possible
						// TokenInput could be followed by EndingInput or LeftrecInput
						WalkAlongEpsilonEdges(currentTokenIndex, newTrace);
					}
				}
			}

/***********************************************************************
TraceManager::Input
***********************************************************************/

			void TraceManager::Input(vint32_t currentTokenIndex, vint32_t token)
			{
				CHECK_ERROR(state == TraceManagerState::WaitingForInput, L"vl::glr::automaton::TraceManager::Input(vint, vint)#Wrong timing to call this function.");
				vint32_t traceCount = concurrentCount;
				vint32_t input = Executable::TokenBegin + token;

				BeginSwap();

				// for each surviving trace
				// step one TokenInput transition
				// followed by multiple and EndingInput, LeftrecInput and their combination
				// one surviving trace could create multiple surviving trace
				for (vint32_t traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					vint32_t transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + input;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongTokenEdges(currentTokenIndex, input, trace, edgeArray);
				}

				// if competitions happen between new surviving traces
				// remove traces that known to have lost the competition
				CheckBackupTracesBeforeSwapping(currentTokenIndex);

				EndSwap();

				// if there are unused spaces in concurrentTraces
				// set them to nullptr to clear out traces from the last round
				for (vint32_t traceIndex = concurrentCount; traceIndex < concurrentTraces->Count(); traceIndex++)
				{
					concurrentTraces->Set(traceIndex, nullptr);
				}
			}

/***********************************************************************
TraceManager::EndOfInput
***********************************************************************/

			void TraceManager::EndOfInput()
			{
				CHECK_ERROR(state == TraceManagerState::WaitingForInput, L"vl::glr::automaton::TraceManager::EndOfInput()#Wrong timing to call this function.");
				state = TraceManagerState::Finished;

				vint32_t traceCount = concurrentCount;
				BeginSwap();

				// check all surviving traces and remove all that
				//   1) does not stay in an ending state
				//   2) return stack is not empty
				// the remaining are all traces that successfully walked to the ending state of the root rule
				for (vint32_t traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					auto& stateDesc = executable.states[trace->state];
					if (trace->returnStack == -1 && stateDesc.endingState)
					{
						AddTrace(trace);
					}
				}

				EndSwap();
			}
		}
	}
}