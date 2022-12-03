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
				if (elementCollection.siblingNext == nullref && elementCollection.siblingPrev == nullref)
				{
					auto&& ownerCollection = owner->*collection;
					if (ownerCollection.first == nullref)
					{
						ownerCollection.first = element;
						ownerCollection.last = element;
					}
					else
					{
						auto sibling = GetTrace(ownerCollection.last);
						auto&& siblingCollection = sibling->*collection;
						CHECK_ERROR(siblingCollection.siblingNext == nullref, errorMessage);

						siblingCollection.siblingNext = element;
						elementCollection.siblingPrev = sibling;
						ownerCollection.last = element;
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
					copiedElement->predecessors.siblingPrev = nullref;
					copiedElement->predecessors.siblingNext = nullref;

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
					// Trace::successors is filled by EndOfInput
					// if Input and EndOfInput succeeded
					// there should not be any multiple to multiple relationship
					CHECK_FAIL(errorMessage);
				}
			}

			TraceManager::TraceManager(Executable& _executable, const ITypeCallback* _typeCallback, vint blockSize)
				: executable(_executable)
				, typeCallback(_typeCallback)
				, returnStacks(blockSize)
				, traces(blockSize)
				, competitions(blockSize)
				, attendingCompetitions(blockSize)
				, traceExecs(blockSize)
				, insExec_Objects(blockSize)
				, insExec_InsRefLinks(blockSize)
				, insExec_ObjRefLinks(blockSize)
				, insExec_ObjectStacks(blockSize)
				, insExec_CreateStacks(blockSize)
				, traceAmbiguities(blockSize)
				, traceAmbiguityLinks(blockSize)
				, executionSteps(blockSize)
			{
			}

			ReturnStack* TraceManager::GetReturnStack(Ref<ReturnStack> index)
			{
				return returnStacks.Get(index);
			}

			ReturnStack* TraceManager::AllocateReturnStack()
			{
				return returnStacks.Get(returnStacks.Allocate());
			}

			Trace* TraceManager::GetTrace(Ref<Trace> index)
			{
				return traces.Get(index);
			}

			Trace* TraceManager::AllocateTrace()
			{
				return traces.Get(traces.Allocate());
			}

			Competition* TraceManager::GetCompetition(Ref<Competition> index)
			{
				return competitions.Get(index);
			}

			Competition* TraceManager::AllocateCompetition()
			{
				return competitions.Get(competitions.Allocate());
			}

			AttendingCompetitions* TraceManager::GetAttendingCompetitions(Ref<AttendingCompetitions> index)
			{
				return attendingCompetitions.Get(index);
			}

			AttendingCompetitions* TraceManager::AllocateAttendingCompetitions()
			{
				return attendingCompetitions.Get(attendingCompetitions.Allocate());
			}

			InsExec* TraceManager::GetInsExec(vint32_t index)
			{
				return &insExecs[index];
			}
			
			InsExec_Object* TraceManager::GetInsExec_Object(Ref<InsExec_Object> index)
			{
				return insExec_Objects.Get(index);
			}

			InsExec_InsRefLink* TraceManager::GetInsExec_InsRefLink(Ref<InsExec_InsRefLink> index)
			{
				return insExec_InsRefLinks.Get(index);
			}

			InsExec_ObjRefLink* TraceManager::GetInsExec_ObjRefLink(Ref<InsExec_ObjRefLink> index)
			{
				return insExec_ObjRefLinks.Get(index);
			}

			InsExec_ObjectStack* TraceManager::GetInsExec_ObjectStack(Ref<InsExec_ObjectStack> index)
			{
				return insExec_ObjectStacks.Get(index);
			}

			InsExec_CreateStack* TraceManager::GetInsExec_CreateStack(Ref<InsExec_CreateStack> index)
			{
				return insExec_CreateStacks.Get(index);
			}

			TraceExec* TraceManager::GetTraceExec(Ref<TraceExec> index)
			{
				return traceExecs.Get(index);
			}

			TraceAmbiguity* TraceManager::GetTraceAmbiguity(Ref<TraceAmbiguity> index)
			{
				return traceAmbiguities.Get(index);
			}

			TraceAmbiguityLink* TraceManager::GetTraceAmbiguityLink(Ref<TraceAmbiguityLink> index)
			{
				return traceAmbiguityLinks.Get(index);
			}

			ExecutionStep* TraceManager::GetExecutionStep(Ref<ExecutionStep> index)
			{
				return executionSteps.Get(index);
			}

/***********************************************************************
CreateExecutor
***********************************************************************/

			Ptr<IExecutor> CreateExecutor(Executable& executable, const IExecutor::ITypeCallback* typeCallback, vint blockSize)
			{
				return Ptr(new TraceManager(executable, typeCallback, blockSize));
			}
		}
	}
}