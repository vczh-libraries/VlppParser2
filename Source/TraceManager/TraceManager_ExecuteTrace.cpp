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

			Ptr<ParsingAstBase> TraceManager::ExecuteTrace(IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::ExecuteTrace(Trace*, IAstInsReceiver&, List<RegexToken>&)#"
				CHECK_ERROR(state == TraceManagerState::PreparedTraceRoute, ERROR_MESSAGE_PREFIX L"Wrong timing to call this function.");

				TraceManagerSubmitter submitter;
				submitter.receiver = &receiver;

				// execute from the root trace
				vint32_t startIns = 0;
				auto trace = initialTrace;
				while (trace)
				{
					TraceInsLists insLists;
					ReadInstructionList(trace, insLists);

					vint32_t minIns = 0;
					vint32_t maxIns = insLists.c3 - 1;
					CHECK_ERROR(minIns <= startIns, ERROR_MESSAGE_PREFIX L"startIns corrupted.");

					for (vint32_t i = startIns; i <= maxIns; i++)
					{
						auto& ins = ReadInstruction(i, insLists);
						auto& token = tokens[trace->currentTokenIndex];
						submitter.Submit(ins, token, trace->currentTokenIndex);
					}

					if (trace->successors.first == -1)
					{
						trace = nullptr;
						startIns = 0;
					}
					else if (trace->successors.first == trace->successors.last)
					{
						trace = GetTrace(trace->successors.first);
						startIns = 0;
					}
					else
					{
						CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Ambmgituiy not implemented yet!");
					}
				}

				submitter.ExecuteSubmitted();
				return receiver.Finished();
#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}