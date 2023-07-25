#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{

/***********************************************************************
EnsureTraceWithValidStates
***********************************************************************/

			Trace* TraceManager::EnsureTraceWithValidStates(Trace* trace)
			{
				if (trace->state == -1)
				{
					return GetTrace(trace->predecessors.first);
				}
				else
				{
					return trace;
				}
			}

/***********************************************************************
AreTwoEndingInputTraceEqual
***********************************************************************/

			bool TraceManager::AreTwoEndingInputTraceEqual(Trace* newTrace, Trace* candidate)
			{
				// two traces equal to each other if
				//   1) they are in the same state
				//   2) they have the same executedReturnStack (and therefore the same returnStack)
				//   3) they are attending same competitions
				//   4) they have the same switchValues
				//   5) the candidate has an ending input

				candidate = EnsureTraceWithValidStates(candidate);

				if (candidate->byInput != Executable::EndingInput) return false;
				if (newTrace->state != candidate->state) return false;
				if (newTrace->executedReturnStack != candidate->executedReturnStack) return false;
				if (newTrace->returnStack != candidate->returnStack) return false;
				if (newTrace->competitionRouting.attendingCompetitions != candidate->competitionRouting.attendingCompetitions) return false;
				return true;
			}

/***********************************************************************
MergeTwoEndingInputTrace
***********************************************************************/

			void TraceManager::MergeTwoEndingInputTrace(Trace* newTrace, Trace* candidate)
			{
				// goal of this function is to create a structure
				// NEWTRACE ---+->AMBIGUITY
				//             |
				// CANDIDATE --+

				// if CANDIDATE is not a merged trace
				// a former trace will copy CANDIDATE and insert before CANDIDATE
				// and CANDIDATE will be initialized to an empty trace

				if (candidate->state == -1)
				{
					AddTraceToCollection(candidate, newTrace, &Trace::predecessors);
				}
				else
				{
					auto formerTrace = AllocateTrace();
					auto formerTraceId = formerTrace->allocatedIndex;
					auto candidateId = candidate->allocatedIndex;

					*formerTrace = *candidate;
					formerTrace->allocatedIndex = formerTraceId;

					*candidate = {};
					candidate->allocatedIndex = candidateId;

					AddTraceToCollection(candidate, formerTrace, &Trace::predecessors);
					AddTraceToCollection(candidate, newTrace, &Trace::predecessors);
				}
			}

/***********************************************************************
TryMergeSurvivedTraces
***********************************************************************/

			void TraceManager::TryMergeSurvivedTraces()
			{
			}
		}
	}
}