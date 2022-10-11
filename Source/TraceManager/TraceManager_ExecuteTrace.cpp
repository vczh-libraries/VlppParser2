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

			struct TraceManagerSubmitter
			{
				// AccumulatedDfa
				vint32_t				adfaCount = 0;
				vint32_t				adfaIndex = -1;
				regex::RegexToken*		adfaToken = nullptr;

				// AccumulatedEoRo
				vint32_t				aeoroCount = 0;
				vint32_t				aeoroIndex = -1;
				regex::RegexToken*		aeoroToken = nullptr;

				// Caching
				AstIns					cachedIns;
				vint32_t				cachedIndex = -1;
				regex::RegexToken*		cachedToken = nullptr;

				IAstInsReceiver*		receiver = nullptr;

				void Submit(AstIns& ins, regex::RegexToken& token, vint32_t tokenIndex)
				{
					// multiple DelayFieldAssignment are compressed to single AccumulatedDfa
					// multiple EndObject+ReopenObject are compressed to single AccumulatedEoRo

					switch (ins.type)
					{
					case AstInsType::DelayFieldAssignment:
						if (aeoroToken == nullptr && cachedToken == nullptr && (adfaToken == nullptr || adfaToken == &token))
						{
							adfaCount++;
							adfaIndex = tokenIndex;
							adfaToken = &token;
						}
						else
						{
							ExecuteSubmitted();
							adfaCount = 1;
							adfaIndex = tokenIndex;
							adfaToken = &token;
						}
						break;
					case AstInsType::EndObject:
						if (adfaToken == nullptr && cachedToken == nullptr)
						{
							cachedIns = ins;
							cachedIndex = tokenIndex;
							cachedToken = &token;
						}
						else
						{
							ExecuteSubmitted();
							cachedIns = ins;
							cachedIndex = tokenIndex;
							cachedToken = &token;
						}
						break;
					case AstInsType::ReopenObject:
						if (adfaToken != nullptr || cachedToken == nullptr || cachedIns.type != AstInsType::EndObject)
						{
							ExecuteSubmitted();
							receiver->Execute(ins, token, tokenIndex);
						}
						else if ((aeoroToken == nullptr || aeoroToken == &token) && cachedToken == &token)
						{
							aeoroCount++;
							aeoroIndex = tokenIndex;
							aeoroToken = &token;
							cachedToken = nullptr;
						}
						else if (cachedToken == &token)
						{
							cachedToken = nullptr;
							ExecuteSubmitted();
							aeoroCount = 1;
							aeoroIndex = tokenIndex;
							aeoroToken = &token;
						}
						else
						{
							ExecuteSubmitted();
							receiver->Execute(ins, token, tokenIndex);
						}
						break;
					default:
						ExecuteSubmitted();
						receiver->Execute(ins, token, tokenIndex);
					}
				}

				void ExecuteSubmitted()
				{
					if (adfaToken)
					{
						if (adfaCount == 1)
						{
							AstIns ins = { AstInsType::DelayFieldAssignment };
							receiver->Execute(ins, *adfaToken, adfaIndex);
						}
						else
						{
							AstIns ins = { AstInsType::AccumulatedDfa,-1,adfaCount };
							receiver->Execute(ins, *adfaToken, adfaIndex);
						}
						adfaCount = 0;
						adfaToken = nullptr;
					}
					if (aeoroToken)
					{
						AstIns ins = { AstInsType::AccumulatedEoRo,-1,aeoroCount };
						receiver->Execute(ins, *aeoroToken, aeoroIndex);
						aeoroCount = 0;
						aeoroToken = nullptr;
					}
					if (cachedToken)
					{
						receiver->Execute(cachedIns, *cachedToken, aeoroIndex);
						cachedToken = nullptr;
					}
				}
			};

#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::ExecuteTrace(Trace*, IAstInsReceiver&, List<RegexToken>&)#"

			void TraceManager::ExecuteSingleTrace(TraceManagerSubmitter& submitter, Trace* trace, vint32_t firstIns, vint32_t lastIns, TraceInsLists& insLists, collections::List<regex::RegexToken>& tokens)
			{
				for (vint32_t i = firstIns; i <= lastIns; i++)
				{
					auto& ins = ReadInstruction(i, insLists);
					auto& token = tokens[trace->currentTokenIndex];
					submitter.Submit(ins, token, trace->currentTokenIndex);
				}
			}

			void TraceManager::ExecuteSingleStep(TraceManagerSubmitter& submitter, ExecutionStep* step, collections::List<regex::RegexToken>& tokens)
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
								lastIns = insLists->c3 - 1;
							}

							// execute instructions
							ExecuteSingleTrace(submitter, trace, firstIns, lastIns, *insLists, tokens);

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
						AstIns ins = { AstInsType::ResolveAmbiguity,step->ei_ra.type,step->ei_ra.count };
						submitter.Submit(ins, tokens[step->ei_ra.token], step->ei_ra.token);
					}
					break;
				}
			}

			Ptr<ParsingAstBase> TraceManager::ExecuteTrace(IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens)
			{
				CHECK_ERROR(state == TraceManagerState::ResolvedAmbiguity, ERROR_MESSAGE_PREFIX L"Wrong timing to call this function.");

				TraceManagerSubmitter submitter;
				submitter.receiver = &receiver;

				// execute from the first step
				auto step = GetInitialExecutionStep();
				CHECK_ERROR(step != nullptr, L"Ambiguity not implemented!");
				while (step)
				{
					// execute step
					ExecuteSingleStep(submitter, step, tokens);

					// find the next step
					step = step->next == nullref ? nullptr : GetExecutionStep(step->next);
				}

				submitter.ExecuteSubmitted();
				return receiver.Finished();
			}
#undef ERROR_MESSAGE_PREFIX
		}
	}
}