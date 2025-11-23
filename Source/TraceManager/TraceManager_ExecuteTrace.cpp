#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
TraceManager::ExecuteTrace
***********************************************************************/

#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::ExecuteTrace(Trace*, IAstInsReceiver&, List<RegexToken>&)#"

			void TraceManager::ExecuteSingleTrace(IAstInsReceiver& receiver, Trace* trace, vint32_t firstIns, vint32_t lastIns, TraceInsLists& insLists, collections::List<regex::RegexToken>& tokens)
			{
				for (vint32_t i = firstIns; i <= lastIns; i++)
				{
					auto& ins = ReadInstruction(i, insLists);
					auto& token = tokens[trace->currentTokenIndex];
					receiver.Execute(ins, token, trace->currentTokenIndex);
				}
			}

			void TraceManager::ExecuteSingleStep(IAstInsReceiver& receiver, ExecutionStep* step, collections::List<regex::RegexToken>& tokens)
			{
				TraceInsLists temp;

				switch (step->type)
				{
				case ExecutionType::Instruction:
					{
						// execute from the start trace
						auto trace = GetTrace(Ref<Trace>(step->et_i.startTrace));

						while (trace)
						{
							vint32_t firstIns = -1;
							vint32_t lastIns = -1;
							auto insLists = &temp;
							if (trace->traceExecRef == nullref)
							{
								ReadInstructionList(trace, temp);
							}
							else
							{
								insLists = &GetTraceExec(trace->traceExecRef)->insLists;
							}

							// find instruction range to execute
							if (trace->allocatedIndex == step->et_i.startTrace)
							{
								firstIns = step->et_i.startIns;
							}
							else
							{
								firstIns = 0;
							}

							if (trace->allocatedIndex == step->et_i.endTrace)
							{
								lastIns = step->et_i.endIns;
							}
							else
							{
								lastIns = insLists->countAll - 1;
							}

							// execute instructions
							ExecuteSingleTrace(receiver, trace, firstIns, lastIns, *insLists, tokens);

							// find the next trace
							if (step->et_i.endTrace == trace->allocatedIndex)
							{
								break;
							}
							else if (trace->successors.first == nullref)
							{
								CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Successor trace missing!");
							}
							else if (trace->successors.first == trace->successors.last)
							{
								trace = GetTrace(trace->successors.first);
							}
							else
							{
								CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Ambiguity should not happen inside one execution step!");
							}
						}
					}
					break;
				case ExecutionType::ResolveAmbiguity:
					{
						AstIns ins = { AstInsType::ResolveAmbiguity,step->et_ra.type,step->et_ra.count };
						auto raTrace = GetTrace(Ref<Trace>(step->et_ra.trace));
						raTrace = EnsureTraceWithValidStates(raTrace);
						auto raToken = raTrace->currentTokenIndex;
						receiver.Execute(ins, tokens[raToken], raToken);
					}
					break;
				default:;
				}
			}

			Ptr<ParsingAstBase> TraceManager::ExecuteTrace(IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens)
			{
				CHECK_ERROR(state == TraceManagerState::ResolvedAmbiguity, ERROR_MESSAGE_PREFIX L"Wrong timing to call this function.");

				// execute from the first step
				auto step = GetInitialExecutionStep();
				CHECK_ERROR(step != nullptr, L"Internal error: execution steps not built!");
				while (step)
				{
					// execute step
					ExecuteSingleStep(receiver, step, tokens);

					// find the next step
					step = step->next == nullref ? nullptr : GetExecutionStep(step->next);
				}

				return receiver.Finished();
			}
#undef ERROR_MESSAGE_PREFIX
		}
	}
}