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
				// return ri1 == ri2; // also works, check later
				// TODO: create a cache to compare two returnIndex directly
				// instead of repeatly scanning the content here
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

			void TraceManager::CheckAttendingCompetitionsOnEndingEdge(Trace* trace, vint32_t acId, vint32_t returnStack)
			{
				while (acId != -1)
				{
					auto ac = GetAttendingCompetitions(acId);
					auto cpt = GetCompetition(ac->competition);
					auto cptr = GetTrace(cpt->ownerTrace);
					if (cptr != trace && cptr->returnStack == returnStack)
					{
						CHECK_ERROR(cpt->status != CompetitionStatus::LowPriorityWin, L"The competition is closed too early.");
						cpt->status = CompetitionStatus::HighPriorityWin;
						break;
					}
					acId = ac->next;
				}
			}

			void TraceManager::CheckBackupTracesBeforeSwapping(vint32_t currentTokenIndex)
			{
				{
					auto cId = activeCompetitions;
					while (cId != -1)
					{
						auto cpt = GetCompetition(cId);
						cpt->highCounter = 0;
						cpt->lowCounter = 0;
						cId = cpt->next;
					}
				}

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

				{
					auto cId = activeCompetitions;
					while (cId != -1)
					{
						auto cpt = GetCompetition(cId);
						if (cpt->status == CompetitionStatus::Holding)
						{
							if (cpt->highCounter > 0 && cpt->lowCounter == 0)
							{
								cpt->status = CompetitionStatus::HighPriorityWin;
							}
							else if (cpt->highCounter == 0 && cpt->lowCounter > 0)
							{
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
				vint32_t state = edgeDesc.toState;
				vint32_t returnStack = trace->returnStack;
				vint32_t executedReturn = -1;

				// attend a competition hold by the current trace if the priority is set for this output transition
				vint32_t acId = AttendCompetitionIfNecessary(trace, edgeDesc);

				if (input == Executable::EndingInput)
				{
					// an EndingInput transition consume return record in the return stack
					// such return will be popped from the return stack and stored in Trace::executedReturn
					CHECK_ERROR(edgeDesc.returnIndices.count == 0, L"vl::glr::automaton::TraceManager::WalkAlongSingleEdge(vint, vint, vint, Trace*, vint, EdgeDesc&)#Ending input edge is not allowed to push something into the return stack.");
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
					CheckAttendingCompetitionsOnEndingEdge(trace, acId, trace->returnStack);

					// if the target trace has exactly the same to another surviving trace
					// stop creating a Trace instance for the target trace
					// instead connect the correct trace to that surviving trace and form a ambiguity resolving structure
					for (vint i = 0; i < concurrentCount; i++)
					{
						auto candidate = backupTraces->Get(i);
						if (AreTwoTraceEqual(state, returnStack, executedReturn, acId, candidate))
						{
							AddTraceToCollection(candidate, trace, &Trace::predecessors);
							return nullptr;
						}
					}
				}

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