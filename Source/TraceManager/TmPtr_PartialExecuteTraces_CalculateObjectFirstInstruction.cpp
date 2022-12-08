#include "TraceManager.h"

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

			void TraceManager::InjectFirstInstruction(Ref<Trace> trace, vint32_t ins, Ref<InsExec_ObjRefLink> injectTargets, vuint64_t magicInjection)
			{
				auto objLinkRef = injectTargets;
				while (objLinkRef != nullref)
				{
					auto objLink = GetInsExec_ObjRefLink(objLinkRef);
					objLinkRef = objLink->previous;
					auto ieObject = GetInsExec_Object(objLink->id);

					if (ieObject->mergeCounter == magicInjection) continue;
					ieObject->mergeCounter = magicInjection;

					if (
						trace < ieObject->topTrace ||
						(trace == ieObject->topTrace && ins < ieObject->topIns)
						)
					{
						ieObject->topTrace = trace;
						ieObject->topIns = ins;
						InjectFirstInstruction(trace, ins, ieObject->injectObjectIds, magicInjection);
					}
				}
			}

			void TraceManager::CalculateObjectFirstInstruction()
			{
				// check all individual objects
				{
					auto objRef = firstObject;
					while (objRef != nullref)
					{
						auto ieObject = GetInsExec_Object(objRef);
						objRef = ieObject->previous;

						// set the top trace to its create trace
						ieObject->topTrace = ieObject->bo_bolr_Trace;
						ieObject->topIns = ieObject->bo_bolr_Ins;

						// check all DFA instructions
						auto insRefLinkId = ieObject->dfaInsRefs;
						while (insRefLinkId != nullref)
						{
							auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
							insRefLinkId = insRefLink->previous;
							if (
								insRefLink->trace < ieObject->topTrace ||
								(insRefLink->trace == ieObject->topTrace && insRefLink->ins < ieObject->topIns)
								)
							{
								ieObject->topTrace = insRefLink->trace;
								ieObject->topIns = insRefLink->ins;
							}
						}
					}
				}

				// check all inject into targets
				{
					auto objRef = firstObject;
					while (objRef != nullref)
					{
						auto ieObject = GetInsExec_Object(objRef);
						objRef = ieObject->previous;

						NEW_MERGE_STACK_MAGIC_COUNTER;
						auto magicInjection = MergeStack_MagicCounter;
						ieObject->mergeCounter = magicInjection;
						InjectFirstInstruction(ieObject->topTrace, ieObject->topIns, ieObject->injectObjectIds, magicInjection);
					}
				}
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}