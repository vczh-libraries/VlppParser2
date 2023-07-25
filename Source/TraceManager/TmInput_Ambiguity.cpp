#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;

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

			Trace* TraceManager::MergeTwoEndingInputTrace(Trace* newTrace, Trace* candidate)
			{
				// goal of this function is to create a structure
				// NEWTRACE ---+->AMBIGUITY
				//             |
				// CANDIDATE --+

				// if CANDIDATE is not a merged trace
				// a former trace will copy CANDIDATE and insert before CANDIDATE
				// and CANDIDATE will be initialized to an empty trace

				// if a former trace is created to replace the candidate
				// in which case the candidate becomes a merge trace
				// the former trace is returned

				if (candidate->state == -1)
				{
					AddTraceToCollection(candidate, newTrace, &Trace::predecessors);
					return nullptr;
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
					return formerTrace;
				}
			}

/***********************************************************************
TryMergeSurvivingTraces
***********************************************************************/

			void TraceManager::TryMergeSurvivingTraces()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::TryMergeSurvivingTraces()#"

				struct EndingTraceData
				{
					bool			surviving = true;			// becomes false when this trace does not survive anymore
				};

				vint32_t survivingTraceCount = concurrentCount;
				Dictionary<Trace*, EndingTraceData> endingOrMergeTraces;
				Group<vint32_t, Trace*> endingOrMergeTracesByState;

				// index surviving traces
				for (vint i = 0; i < survivingTraceCount; i++)
				{
					auto trace = backupTraces->Get(i);
					endingOrMergeTraces.Add(trace, {});

					if (trace->state == -1)
					{
						endingOrMergeTracesByState.Add(EnsureTraceWithValidStates(trace)->state, trace);
					}
					else if (trace->byInput == Executable::EndingInput)
					{
						endingOrMergeTracesByState.Add(trace->state, trace);
					}
				}

#if defined VCZH_MSVC && defined _DEBUG
				// check assumptions
				for (vint i = 0; i < survivingTraceCount; i++)
				{
					auto trace = backupTraces->Get(i);
					if (trace->state == -1)
					{
						auto predecessorId = trace->predecessors.first;
						while (predecessorId != nullref)
						{
							auto predecessor = GetTrace(predecessorId);
							predecessorId = predecessor->predecessors.siblingNext;

							CHECK_ERROR(endingOrMergeTraces.Keys().IndexOf(predecessor) == -1, ERROR_MESSAGE_PREFIX L"Internal error: Predecessors of a merge trace should not survive.");
						}
					}
					else if (trace->byInput == Executable::EndingInput)
					{
						CHECK_ERROR(trace->predecessors.first == trace->predecessors.last, ERROR_MESSAGE_PREFIX L"Internal error: Executable::EndingInput trace could not have multiple predecessors.");
					}
				}
#endif

				// check if a trace survived
				// if an Executable::EndingInput trace survived but its only predecessor does not
				// it becomes not survived
				auto ensureEndingTraceSurvived = [this, &endingOrMergeTraces](Trace* trace)
				{
					if (trace == initialTrace) return true;

					auto& data = const_cast<EndingTraceData&>(endingOrMergeTraces[trace]);
					if (!data.surviving) return false;

					if (trace->state != -1)
					{
						auto predecessorId = trace->predecessors.first;
						if (predecessorId != nullref)
						{
							auto predecessor = GetTrace(predecessorId);
							if (!endingOrMergeTraces[predecessor].surviving)
							{
								data.surviving = false;
							}
						}
					}
					return data.surviving;
				};

				// find surviving traces that merge
				for (vint i = 0; i < survivingTraceCount; i++)
				{
					// get the next surviving Executable::EndingInput trace
					auto trace = backupTraces->Get(i);
					if (trace->state != -1 && trace->byInput != Executable::EndingInput) continue;
					if (!ensureEndingTraceSurvived(trace)) continue;
					auto realTrace = EnsureTraceWithValidStates(trace);

					// find all traces Executable::EndingInput traces with the same state that after it
					auto&& candidates = endingOrMergeTracesByState[realTrace->state];
					vint index = candidates.IndexOf(trace);
					for (vint j = index + 1; j < candidates.Count(); j++)
					{
						// ensure the candidate also survived
						auto candidate = candidates[j];
						if (!ensureEndingTraceSurvived(candidate)) continue;
						auto realCandidate = EnsureTraceWithValidStates(candidate);

						if (AreTwoEndingInputTraceEqual(realTrace, realCandidate))
						{
							// merge two traces
							if (trace == realTrace)
							{
								if (candidate == realCandidate)
								{
								}
								else
								{
								}
							}
							else
							{
								if (candidate == realCandidate)
								{
								}
								else
								{
								}
							}
							CHECK_FAIL(L"Fuck");
						}
					}
				}

				// mark all unsurviving traces

				// clean up surviving trace list

#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}