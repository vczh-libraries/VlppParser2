#include "SyntaxBase.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{

/***********************************************************************
AttendCompetition
***********************************************************************/

			void TraceManager::AttendCompetition(Trace* trace, vint32_t& newAttendingCompetitions, vint32_t returnStack, vint32_t ruleId, vint32_t clauseId, bool forHighPriority)
			{
				// a competition is defined by its rule, clause and ReturnStack
				// we only create a new Competition object if it has not been created for the trace yet
				Competition* competition = nullptr;
				{
					vint32_t cid = trace->runtimeRouting.holdingCompetitions;
					while (cid != -1)
					{
						competition = GetCompetition(cid);
						if (competition->ruleId == ruleId && competition->clauseId == clauseId && competition->returnStack == returnStack)
						{
							break;
						}
						cid = competition->nextHoldCompetition;
					}
				}

				if (!competition)
				{
					// create a Competition object
					competition = AllocateCompetition();
					competition->nextHoldCompetition = trace->runtimeRouting.holdingCompetitions;
					trace->runtimeRouting.holdingCompetitions = competition->allocatedIndex;

					competition->returnStack = returnStack;
					competition->currentTokenIndex = trace->currentTokenIndex;
					competition->ruleId = ruleId;
					competition->clauseId = clauseId;

					competition->nextActiveCompetition = activeCompetitions;
					activeCompetitions = competition->allocatedIndex;
				}

				// target traces from the current trace could attend different competitions
				// but they also inherit all attending competitions from the current trace
				// it is fine for different traces share all or part of AttendingCompetitions in their RuntimeRouting::attendingCompetitions linked list
				// because if a competition is settled in the future
				// AttendingCompetitions objects for this competition is going to be removed anyway
				// sharing a linked list doesn't change the result

				auto ac = AllocateAttendingCompetitions();
				ac->next = newAttendingCompetitions;
				ac->competition = competition->allocatedIndex;
				ac->forHighPriority = forHighPriority;
				newAttendingCompetitions = ac->allocatedIndex;
			}

/***********************************************************************
AttendCompetitionIfNecessary
***********************************************************************/

			void TraceManager::AttendCompetitionIfNecessary(Trace* trace, EdgeDesc& edgeDesc, vint32_t& newAttendingCompetitions, vint32_t& newReturnStack)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::AttendCompetitionIfNecessary(Trace*, EdgeDesc&, vint32_t&, vint32_t&)#"
				newAttendingCompetitions = trace->runtimeRouting.attendingCompetitions;
				newReturnStack = trace->returnStack;

				// visit each compact transition in order
				//   1) returns + token
				//   2) ending
				//   3) leftrec
				// find out if any of them attends a competition

				vint32_t edgeFromState = edgeDesc.fromState;
				for (vint32_t returnRef = 0; returnRef < edgeDesc.returnIndices.count; returnRef++)
				{
					auto returnIndex = executable.returnIndices[edgeDesc.returnIndices.start + returnRef];
					auto&& returnDesc = executable.returns[returnIndex];

					if (returnDesc.priority != EdgePriority::NoCompetition)
					{
						// attend a competition from a ReturnDesc edge
						// find out the rule id and the clause id for this competition
						// a ReturnDesc is a compact transition which consumes a rule
						// so it does not points to the ending state
						// therefore we just need the toState of this ReturnDesc for reference
						auto&& stateForClause = executable.states[returnDesc.returnState];
						vint32_t competitionRule = stateForClause.rule;
						vint32_t competitionClause = stateForClause.clause;
						CHECK_ERROR(competitionRule != -1 && competitionClause != -1, ERROR_MESSAGE_PREFIX L"Illegal rule or clause id.");
						AttendCompetition(trace, newAttendingCompetitions, newReturnStack, competitionRule, competitionClause, returnDesc.priority == EdgePriority::HighPriority);
					}

					// push this ReturnDesc to the ReturnStack
					auto returnStack = AllocateReturnStack();
					returnStack->previous = newReturnStack;
					returnStack->returnIndex = returnIndex;
					newReturnStack = returnStack->allocatedIndex;

					edgeFromState = executable.ruleStartStates[returnDesc.consumedRule];
				}

				if (edgeDesc.priority != EdgePriority::NoCompetition)
				{
					// attend a competition from a EdgeDesc edge
					// find out the rule id and the clause id for this competition
					auto&& fromState = executable.states[edgeFromState];
					auto&& toState = executable.states[edgeDesc.toState];
					vint32_t competitionRule = toState.rule;
					vint32_t competitionClause = toState.clause;
					if (toState.endingState)
					{
						competitionRule = fromState.rule;
						competitionClause = fromState.clause;
					}
					CHECK_ERROR(competitionRule != -1 && competitionClause != -1, ERROR_MESSAGE_PREFIX L"Illegal rule or clause id.");
					AttendCompetition(trace, newAttendingCompetitions, newReturnStack, competitionRule, competitionClause, edgeDesc.priority == EdgePriority::HighPriority);
				}
			}

/***********************************************************************
CheckAttendingCompetitionsOnEndingEdge
***********************************************************************/

			void TraceManager::CheckAttendingCompetitionsOnEndingEdge(Trace* trace, EdgeDesc& edgeDesc, vint32_t acId, vint32_t returnStack)
			{
				while (acId != -1)
				{
					// when executing an EndingInput transition, we announce high priority win a competition if
					//   1) such EndingInput transitions ends the clause, and the state of the trace holding competition belongs to the same clause
					//      we ensure this by comparing rule id, clause id and returnStack object (not content)
					//      because a ReturnStack object is created when entering a new clause
					//   2) this trace bets high
					//   3) the competition has not been settled
					auto ac = GetAttendingCompetitions(acId);
					auto cpt = GetCompetition(ac->competition);
					if (cpt->returnStack == returnStack)
					{
						// ensure that this EndingInput edge and the competition belong to the same clause
						auto&& stateDesc = executable.states[edgeDesc.fromState];
						if (cpt->ruleId == stateDesc.rule && cpt->clauseId == stateDesc.clause)
						{
							// check if it is a high bet
							if (ac->forHighPriority && cpt->status == CompetitionStatus::Holding)
							{
								cpt->status = CompetitionStatus::HighPriorityWin;
							}
							break;
						}
					}
					acId = ac->next;
				}
			}

/***********************************************************************
CheckBackupTracesBeforeSwapping
***********************************************************************/

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
						cId = cpt->nextActiveCompetition;
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
								// if only low bet traces survive
								// low priority win after at least one token is consumed from when the competition is created
								// low priority epsilon transitions could have been visited right after a competition is created
								// but high priority token transitions could only be visited when consuming the next token
								// if all high priority transitions are token transitions
								// and all low priority transitions are epsilon transitions
								// closing the competition too early will direct to a wrong result
								// so we need to wait at least one step to see if any trace will visit the high priority transition in the future
								if (cpt->currentTokenIndex != currentTokenIndex)
								{
									cpt->status = CompetitionStatus::LowPriorityWin;
								}
							}
						}
						cId = cpt->nextActiveCompetition;
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
							*pnext = cpt->nextActiveCompetition;
						}
						else
						{
							pnext = &cpt->nextActiveCompetition;
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
		}
	}
}