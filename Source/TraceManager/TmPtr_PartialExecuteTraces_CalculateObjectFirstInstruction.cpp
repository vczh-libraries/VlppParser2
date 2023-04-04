#include "TraceManager.h"

#if defined VCZH_MSVC && defined _DEBUG
#define VCZH_DO_DEBUG_CHECK
#endif

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;

#define NEW_MERGE_STACK_MAGIC_COUNTER (void)(MergeStack_MagicCounter++)

/***********************************************************************
CalculateObjectFirstInstruction
***********************************************************************/

			bool TraceManager::UpdateTopTrace(InsRef& topInsRef, InsRef newInsRef)
			{
				if (
					topInsRef.trace == nullref ||
					topInsRef.trace > newInsRef.trace ||
					(topInsRef.trace == newInsRef.trace && topInsRef.ins > newInsRef.ins)
					)
				{
					topInsRef = newInsRef;
					return true;
				}
				else
				{
					return false;
				}
			}

			void TraceManager::InjectFirstInstruction(InsRef insRef, Ref<InsExec_ObjRefLink> injectTargets, vuint64_t magicInjection)
			{
				auto objLinkRef = injectTargets;
				while (objLinkRef != nullref)
				{
					auto objLink = GetInsExec_ObjRefLink(objLinkRef);
					objLinkRef = objLink->previous;
					auto ieObject = GetInsExec_Object(objLink->id);

					if (ieObject->mergeCounter == magicInjection) continue;
					ieObject->mergeCounter = magicInjection;

					// there will be only one top create instruction per object
					// even when object relationship is partial ordered
					// TODO: prove it
					if (UpdateTopTrace(ieObject->topInsRef, insRef))
					{
						InjectFirstInstruction(insRef, ieObject->assignedToObjectIds, magicInjection);
					}
				}
			}

			void TraceManager::CalculateObjectFirstInstruction()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CalculateObjectFirstInstruction()#"
				// check all individual objects
				{
					auto objRef = firstObject;
					while (objRef != nullref)
					{
						auto ieObject = GetInsExec_Object(objRef);
						objRef = ieObject->previous;

						// set the top local trace to its create trace
						UpdateTopTrace(ieObject->topLocalInsRef, ieObject->createInsRef);

						// check all DFA instructions
						auto insRefLinkId = ieObject->dfaInsRefs;
						while (insRefLinkId != nullref)
						{
							auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
							insRefLinkId = insRefLink->previous;

							// there will be only one top local create instruction per object
							// even when object relationship is partial ordered
							// TODO: prove it
							UpdateTopTrace(ieObject->topLocalInsRef, insRefLink->insRef);
						}

						// set the top trace to its top local trace
						UpdateTopTrace(ieObject->topInsRef, ieObject->topLocalInsRef);
					}
				}

				// check all assigned to targets
				{
					auto objRef = firstObject;
					while (objRef != nullref)
					{
						auto ieObject = GetInsExec_Object(objRef);
						objRef = ieObject->previous;

						NEW_MERGE_STACK_MAGIC_COUNTER;
						auto magicInjection = MergeStack_MagicCounter;
						ieObject->mergeCounter = magicInjection;
						InjectFirstInstruction(ieObject->topInsRef, ieObject->assignedToObjectIds, magicInjection);

#ifdef VCZH_DO_DEBUG_CHECK
						{
							auto createTrace = GetTrace(ieObject->topInsRef.trace);
							auto traceExec = GetTraceExec(createTrace->traceExecRef);
							auto&& ins = ReadInstruction(ieObject->topInsRef.ins, traceExec->insLists);
							CHECK_ERROR(ins.type == AstInsType::BeginObject || ins.type == AstInsType::DelayFieldAssignment, ERROR_MESSAGE_PREFIX L"The found instruction is not a BeginObject or DelayFieldAssignment instruction.");
						}
#endif
					}
				}
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}