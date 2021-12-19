#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{

/***********************************************************************
TraceManager
***********************************************************************/

			void TraceManager::BeginSwap()
			{
				concurrentCount = 0;
			}

			void TraceManager::AddTrace(Trace* trace)
			{
				if (concurrentCount < backupTraces->Count())
				{
					backupTraces->Set(concurrentCount, trace);
				}
				else
				{
					backupTraces->Add(trace);
				}
				concurrentCount++;
			}

			void TraceManager::EndSwap()
			{
				auto t = concurrentTraces;
				concurrentTraces = backupTraces;
				backupTraces = t;
			}

			void TraceManager::AddTraceToCollection(Trace* owner, Trace* element, TraceCollection(Trace::* collection))
			{
				auto errorMessage = L"vl::glr::automaton::TraceManager::AddTraceToCollection(Trace*, Trace*, TraceCollection(Trace::*))#Multiple to multiple predecessor-successor relationship is not supported.";
				auto&& elementCollection = element->*collection;
				if (elementCollection.siblingNext == -1 && elementCollection.siblingPrev == -1)
				{
					auto&& ownerCollection = owner->*collection;
					if (ownerCollection.first == -1)
					{
						ownerCollection.first = element->allocatedIndex;
						ownerCollection.last = element->allocatedIndex;
					}
					else
					{
						auto sibling = GetTrace(ownerCollection.last);
						auto&& siblingCollection = sibling->*collection;
						CHECK_ERROR(siblingCollection.siblingNext == -1, errorMessage);

						siblingCollection.siblingNext = element->allocatedIndex;
						elementCollection.siblingPrev = sibling->allocatedIndex;
						ownerCollection.last = element->allocatedIndex;
					}
				}
				else if (collection == &Trace::predecessors)
				{
					// there is a valid scenario when
					//                B(ending) ---+
					//                             |
					// O(origin) -+-> A(ending) -+-+-> C(merged)
					//                           |
					//                           +---> D(token)

					// in this case, we need to copy A(ending) to avoid the multiple to multiple relationship
					// the reason we cannot have such relationship is that
					// TraceCollection::(siblingPrev|siblingNext) is a linked list
					// it represents a predecessor collections of owner
					// if a trace is shared in two predecessor collections
					// there is no place for a second linked list
					// the data structure is not able to represent such relationship

					// but this it is not doable if A also has multiple predecessors
					// because copying A replaces a new multiple to multiple relationship to an old one like this
					// O1(origin) -+-+   B(ending) -+
					//             | |              |
					// O2(origin) -+-+-> A(ending) -+-> C(merged)
					//             |
					//             +---> X(ending) ---> D(token)
					CHECK_ERROR(element->predecessors.first == element->predecessors.last, errorMessage);

					auto copiedElement = AllocateTrace();
					{
						vint32_t copiedId = copiedElement->allocatedIndex;
						*copiedElement = *element;
						copiedElement->allocatedIndex = copiedId;
					}

					// clear sibilingPrev and sibilingNext because it belongs to no collection at this moment
					// keep first and last so that it still knows its predecessors
					copiedElement->predecessors.siblingPrev = -1;
					copiedElement->predecessors.siblingNext = -1;

					// now it becomes
					//                B(ending) -+
					//                           |
					// O(origin) -+-> A(ending) -+-> C(merged)
					//            |
					//            +-> X(ending) ---> D(token)
					AddTraceToCollection(owner, copiedElement, collection);
				}
				else
				{
					// Trace::predecessors is filled by Input
					// Trace::successors is filled by PrepareTraceRoute
					// if Input and EndOfInput succeeded
					// there should not be any multiple to multiple relationship
					CHECK_FAIL(errorMessage);
				}
			}

			TraceManager::TraceManager(Executable& _executable, ITypeCallback* _typeCallback)
				: executable(_executable)
				, typeCallback(_typeCallback)
			{
			}

			ReturnStack* TraceManager::GetReturnStack(vint32_t index)
			{
				return returnStacks.Get(index);
			}

			ReturnStack* TraceManager::AllocateReturnStack()
			{
				return returnStacks.Get(returnStacks.Allocate());
			}

			Trace* TraceManager::GetTrace(vint32_t index)
			{
				return traces.Get(index);
			}

			Trace* TraceManager::AllocateTrace()
			{
				return traces.Get(traces.Allocate());
			}

			Competition* TraceManager::GetCompetition(vint32_t index)
			{
				return competitions.Get(index);
			}

			Competition* TraceManager::AllocateCompetition()
			{
				return competitions.Get(competitions.Allocate());
			}

			AttendingCompetitions* TraceManager::GetAttendingCompetitions(vint32_t index)
			{
				return attendingCompetitions.Get(index);
			}

			AttendingCompetitions* TraceManager::AllocateAttendingCompetitions()
			{
				return attendingCompetitions.Get(attendingCompetitions.Allocate());
			}

			void TraceManager::Initialize(vint32_t startState)
			{
				state = TraceManagerState::WaitingForInput;

				returnStacks.Clear();
				traces.Clear();
				competitions.Clear();
				attendingCompetitions.Clear();

				traces1.Clear();
				traces2.Clear();
				concurrentTraces = &traces1;
				backupTraces = &traces2;

				activeCompetitions = -1;
				initialReturnStackSuccessors = {};

				initialTrace = AllocateTrace();
				initialTrace->state = startState;
				concurrentCount = 1;
				concurrentTraces->Add(initialTrace);
			}
		}
	}
}