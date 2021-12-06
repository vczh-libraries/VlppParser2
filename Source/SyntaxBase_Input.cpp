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
				if (edgeDesc.priority != EdgePriority::NoCompetition)
				{
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

					switch (edgeDesc.priority)
					{
					case EdgePriority::HighPriority:
						if (competition->highBet == -1)
						{
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
							auto ac = AllocateAttendingCompetitions();
							ac->next = trace->runtimeRouting.attendingCompetitions;
							ac->competition = competition->allocatedIndex;
							ac->forHighPriority = false;
							competition->highBet = ac->allocatedIndex;
						}
						return competition->lowBet;
					}
				}
				return trace->runtimeRouting.attendingCompetitions;
			}

			void TraceManager::CheckAttendingCompetitionsOnEndingEdge(vint32_t acId, vint32_t returnIndex)
			{
				while (acId != -1)
				{
					auto ac = GetAttendingCompetitions(acId);
					auto cpt = GetCompetition(ac->competition);
					auto cptr = GetTrace(cpt->ownerTrace);
					if (cptr->returnStack == returnIndex)
					{
						CHECK_ERROR(cpt->status != CompetitionStatus::LowPriorityWin, L"The competition is closed too early.");
						cpt->status = CompetitionStatus::HighPriorityWin;
						break;
					}
					acId = ac->next;
				}
			}

			void TraceManager::CheckBackupTracesBeforeSwapping()
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
								cpt->status = CompetitionStatus::LowPriorityWin;
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
				vint32_t acId = AttendCompetitionIfNecessary(trace, edgeDesc);

				if (input == Executable::EndingInput)
				{
					CHECK_ERROR(edgeDesc.returnIndices.count == 0, L"vl::glr::automaton::TraceManager::WalkAlongSingleEdge(vint, vint, vint, Trace*, vint, EdgeDesc&)#Ending input edge is not allowed to push the return stack.");
					if (returnStack != -1)
					{
						auto rs = GetReturnStack(returnStack);
						returnStack = rs->previous;
						executedReturn = rs->returnIndex;
						state = executable.returns[executedReturn].returnState;
					}

					CheckAttendingCompetitionsOnEndingEdge(acId, executedReturn);

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
					vint32_t transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + Executable::LeftrecInput;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongLeftrecEdges(currentTokenIndex, trace, edgeArray);
				}
				{
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
				for (vint32_t edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint32_t byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					if (auto newTrace = WalkAlongSingleEdge(currentTokenIndex, input, trace, byEdge, edgeDesc))
					{
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
				for (vint32_t traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					vint32_t transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + input;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongTokenEdges(currentTokenIndex, input, trace, edgeArray);
				}
				// TODO: check if any bet wins and close competitions
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
				for (vint32_t traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					auto& stateDesc = executable.states[trace->state];
					if (trace->returnStack == -1 && stateDesc.endingState)
					{
						AddTrace(trace);
					}
				}
				CheckBackupTracesBeforeSwapping();
				EndSwap();
			}
		}
	}
}