#include "SyntaxBase.h"

namespace vl
{
	namespace stream
	{
		namespace internal
		{
			using namespace glr;
			using namespace glr::automaton;

			SERIALIZE_ENUM(AstInsType)
			SERIALIZE_ENUM(EdgePriority)

			BEGIN_SERIALIZATION(AstIns)
				SERIALIZE(type)
				SERIALIZE(param)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(InstructionArray)
				SERIALIZE(start)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(ReturnIndexArray)
				SERIALIZE(start)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(EdgeArray)
				SERIALIZE(start)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(ReturnDesc)
				SERIALIZE(consumedRule)
				SERIALIZE(returnState)
				SERIALIZE(priority)
				SERIALIZE(insAfterInput)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(EdgeDesc)
				SERIALIZE(fromState)
				SERIALIZE(toState)
				SERIALIZE(priority)
				SERIALIZE(insBeforeInput)
				SERIALIZE(insAfterInput)
				SERIALIZE(returnIndices)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(StateDesc)
				SERIALIZE(rule)
				SERIALIZE(clause)
				SERIALIZE(endingState)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(Executable)
				SERIALIZE(tokenCount)
				SERIALIZE(ruleCount)
				SERIALIZE(ruleStartStates)
				SERIALIZE(transitions)
				SERIALIZE(instructions)
				SERIALIZE(returnIndices)
				SERIALIZE(returns)
				SERIALIZE(edges)
				SERIALIZE(states)
			END_SERIALIZATION
		}
	}

	namespace glr
	{
		namespace automaton
		{
			using namespace stream;

/***********************************************************************
Executable
***********************************************************************/

			Executable::Executable(stream::IStream& inputStream)
			{
				internal::ContextFreeReader reader(inputStream);
				reader << *this;
			}

			void Executable::Serialize(stream::IStream& outputStream)
			{
				internal::ContextFreeWriter writer(outputStream);
				writer << *this;
			}

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

					// TODO: what if A also has multiple predecessors?
				}
				else
				{
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

				rootTrace = nullptr;
				activeCompetitions = -1;

				auto trace = AllocateTrace();
				trace->state = startState;
				concurrentCount = 1;
				concurrentTraces->Add(trace);
			}
		}
	}
}