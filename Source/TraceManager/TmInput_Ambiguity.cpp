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
				//   2) they have the same executedReturnStack and returnStack
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
				if (concurrentCount == 0) return;

				struct EndingOrMergeTraceData
				{
					bool			surviving = true;			// becomes false when this trace does not survive anymore
				};

				vint32_t removedTracesCount = 0;
				Dictionary<Trace*, EndingOrMergeTraceData> endingOrMergeTraces;
				Group<vint32_t, Trace*> endingOrMergeTracesByState;

				// index surviving traces
				for (vint32_t i = 0; i < concurrentCount; i++)
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
				for (vint32_t i = 0; i < concurrentCount; i++)
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

				// unsurvive a trace
				auto unsurviveTrace = [&](Trace* trace, EndingOrMergeTraceData& data)
				{
					if (data.surviving)
					{
						data.surviving = false;
						removedTracesCount++;
					}
				};

				// check if a trace survived
				// if an Executable::EndingInput trace survived but its only predecessor does not
				// it becomes not survived
				auto ensureEndingTraceSurvived = [&](Trace* trace)
				{
					if (trace == initialTrace) return true;

					auto& data = const_cast<EndingOrMergeTraceData&>(endingOrMergeTraces[trace]);
					if (!data.surviving) return false;

					if (trace->state != -1)
					{
						auto predecessorId = trace->predecessors.first;
						if (predecessorId != nullref)
						{
							auto predecessor = GetTrace(predecessorId);
							if (!endingOrMergeTraces[predecessor].surviving)
							{
								unsurviveTrace(trace, data);
							}
						}
					}
					return data.surviving;
				};

				// find surviving traces that merge
				for (vint32_t i = 0; i < concurrentCount; i++)
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
								// if trace is an ordinary trace
								if (candidate == realCandidate)
								{
									// if candidate is an ordinary trace
									// turn trace into a merge trace
									auto formerTrace = MergeTwoEndingInputTrace(candidate, trace);
									CHECK_ERROR(formerTrace != nullptr, ERROR_MESSAGE_PREFIX L"Internal error: formerTrace should not be null.");
									realTrace = formerTrace;
								}
								else
								{
									// if candidate is a merge trace
									// turn trace into a merge trace
									// give the rest of predecessors of candidate to trace
									auto candidateHead = GetTrace(candidate->predecessors.first);
									auto candidateNextId = candidateHead->predecessors.siblingNext;

									candidateHead->predecessors.siblingNext = nullref;
									auto formerTrace = MergeTwoEndingInputTrace(candidateHead, trace);
									CHECK_ERROR(formerTrace != nullptr, ERROR_MESSAGE_PREFIX L"Internal error: formerTrace should not be null.");
									realTrace = formerTrace;

									candidateHead->predecessors.siblingNext = candidateNextId;
									trace->predecessors.last = candidate->predecessors.last;
									candidate->predecessors.first = nullref;
									candidate->predecessors.last = nullref;
								}
							}
							else
							{
								// if trace is a merge trace
								if (candidate == realCandidate)
								{
									// if candidate is an ordinary trace
									// merge candidate into trace
									auto formerTrace = MergeTwoEndingInputTrace(candidate, trace);
									CHECK_ERROR(formerTrace == nullptr, ERROR_MESSAGE_PREFIX L"Internal error: formerTrace should be null.");
								}
								else
								{
									// if candidate is an ordinary trace
									// give all predecessors of candidate to trace
									auto traceTail = GetTrace(trace->predecessors.last);
									auto candidateHead = GetTrace(candidate->predecessors.first);

									traceTail->predecessors.siblingNext = candidateHead;
									candidateHead->predecessors.siblingPrev = traceTail;

									trace->predecessors.last = candidate->predecessors.last;
									candidate->predecessors.first = nullref;
									candidate->predecessors.last = nullref;
								}
							}

							auto& data = const_cast<EndingOrMergeTraceData&>(endingOrMergeTraces[candidate]);
							unsurviveTrace(candidate, data);
						}
					}
				}

				if (removedTracesCount > 0)
				{
					// mark all unsurviving traces
					for (vint32_t i = 0; i < concurrentCount; i++)
					{
						auto trace = backupTraces->Get(i);
						vint index = endingOrMergeTraces.Keys().IndexOf(trace);
						if (index == -1) continue;
						if (endingOrMergeTraces.Values()[index].surviving) continue;
						backupTraces->Set(i, nullptr);
					}

					// clean up surviving trace list
					vint writing = 0;
					for (vint32_t i = 0; i < concurrentCount; i++)
					{
						auto trace = backupTraces->Get(i);
						if (!trace) continue;
						backupTraces->Set(writing++, trace);
					}
					concurrentCount -= removedTracesCount;
				}

#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}