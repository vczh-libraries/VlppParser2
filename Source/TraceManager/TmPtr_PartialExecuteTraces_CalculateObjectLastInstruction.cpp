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
CalculateObjectLastInstruction
***********************************************************************/

			void TraceManager::CalculateObjectLastInstruction()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::CalculateObjectLastInstruction()#"
				// check all individual objects
				{
					auto objRef = firstObject;
					while (objRef != nullref)
					{
						auto ieObject = GetInsExec_Object(objRef);
						objRef = ieObject->previous;

						// all EndObject ending a BO/DFA are considered
						// there is no "bottom EndObject"
						// each EndObject should be in different branches
						auto topLocalTrace = GetTrace(ieObject->topLocalInsRef.trace);
						auto traceExec = GetTraceExec(topLocalTrace->traceExecRef);
						auto insExec = GetInsExec(traceExec->insExecRefs.start + ieObject->topLocalInsRef.ins);
						auto insRefLinkId = insExec->eoInsRefs;
						while (insRefLinkId != nullref)
						{
							auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
							insRefLinkId = insRefLink->previous;

							// filter out any result that does not happen after ieObject->createTrace
							// topLocalTrace could be a DFA created object, and multiple objects could share the same DFA object
							// in some cases its eoInsRefs could pointing to EndObject of completely unrelated objects
							// TODO: make it accurate
							auto bottomInsRef = insRefLink->insRef;
							PushInsRefLink(ieObject->bottomInsRefs, bottomInsRef);

#ifdef VCZH_DO_DEBUG_CHECK
							{
								auto eoTrace = GetTrace(bottomInsRef.trace);
								auto traceExec = GetTraceExec(eoTrace->traceExecRef);
								auto&& ins = ReadInstruction(bottomInsRef.ins, traceExec->insLists);
								CHECK_ERROR(ins.type == AstInsType::EndObject, ERROR_MESSAGE_PREFIX L"The found instruction is not a EndObject instruction.");
							}
#endif
						}
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}