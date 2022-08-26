#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{

/***********************************************************************
AreTwoTraceEqual
***********************************************************************/

			bool TraceManager::AreTwoEndingInputTraceEqual(Trace* newTrace, Trace* candidate)
			{
				// two traces equal to each other if
				//   1) they are in the same state
				//   2) they have the same executedReturnStack (and therefore the same returnStack)
				//   3) they are attending same competitions
				//   4) they have the same switchValues
				//   5) the candidate has an ending input

				if (candidate->state == -1)
				{
					candidate = GetTrace(candidate->predecessors.first);
				}

				if (candidate->byInput != Executable::EndingInput) return false;
				if (newTrace->state != candidate->state) return false;
				if (newTrace->executedReturnStack != candidate->executedReturnStack) return false;
				if (newTrace->returnStack != candidate->returnStack) return false;
				if (newTrace->competitionRouting.attendingCompetitions != candidate->competitionRouting.attendingCompetitions) return false;
				if (newTrace->switchValues != candidate->switchValues) return false;
				return true;
			}

/***********************************************************************
MergeTwoEndingInputTrace
***********************************************************************/

			void TraceManager::MergeTwoEndingInputTrace(Trace* newTrace, Trace*& candidate)
			{
				// goal of this function is to create a structure
				// NEWTRACE ---+->AMBIGUITY
				//             |
				// CANDIDATE --+

				// create a merged trace if candidate has not been involved in ambiguity
				Trace* mergedTrace = candidate;
				if (mergedTrace->state != -1)
				{
					mergedTrace = AllocateTrace();
					AddTraceToCollection(mergedTrace, candidate, &Trace::predecessors);

					// replace the candidate in the current trace list
					candidate = mergedTrace;
				}

				AddTraceToCollection(mergedTrace, newTrace, &Trace::predecessors);
			}
		}
	}
}