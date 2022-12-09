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
				// LriFetch + LriStore
				bool					lriFetch = false;

				// AccumulatedDfa
				vint32_t				adfaCount = 0;
				vint32_t				adfaIndex = -1;
				regex::RegexToken*		adfaToken = nullptr;

				// AccumulatedEoRo
				vint32_t				aeoroCount = 0;
				vint32_t				aeoroIndex = -1;
				regex::RegexToken*		aeoroToken = nullptr;

				// Caching EndObject / LriFetch
				AstIns					cachedIns;
				vint32_t				cachedIndex = -1;
				regex::RegexToken*		cachedToken = nullptr;

				IAstInsReceiver*		receiver = nullptr;

				void Submit(AstIns& ins, regex::RegexToken& token, vint32_t tokenIndex)
				{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManagerSubmitter::Submit(AstIns&, RegexToken&, vint32_t)#"
					// LriFetch+LriStore disappear
					// multiple DelayFieldAssignment of the same token are compressed to single AccumulatedDfa
					// multiple EndObject+ReopenObject of the same token are compressed to single AccumulatedEoRo

					// cache availability conditions
					//   lriFetch == true && cachedToken != nullptr
					//     one LriFetch instruction is cached
					//     cachedIns is the cached instruction
					//     { cachedToken,cachedIndex } is the token with this instruction
					//   lriFetch == false && cachedToken != nullptr
					//     one EndObject instruction is cached
					//     cachedIns is the cached instruction
					//     { cachedToken,cachedIndex } is the token with this instruction
					//   aeoroToken != nullptr
					//     aeoroCount EndObject+ReopenObject instruction pairs is cached
					//     { aeoroToken,aeoroIndex } is the token with this instruction
					//   adfaToken != nullptr
					//     aeoroCount DelayFieldAssignment instructions is cached
					//     { adfaToken,adfaIndex } is the token with this instruction

					bool cacheLf = lriFetch == true && cachedToken != nullptr;
					bool cacheEo = lriFetch == false && cachedToken != nullptr;
					bool cacheEoRo = aeoroToken != nullptr;
					bool cacheDfa = adfaToken != nullptr;
					bool cacheAvailable = cacheLf || cacheEo || cacheEoRo || cacheDfa;
					CHECK_ERROR(
						(!cacheLf && !cacheEo && !cacheEoRo && !cacheDfa) ||
						( cacheLf && !cacheEo && !cacheEoRo && !cacheDfa) ||
						(!cacheLf &&  cacheEo && !cacheEoRo && !cacheDfa) ||
						(!cacheLf && !cacheEo &&  cacheEoRo && !cacheDfa) ||
						(!cacheLf &&  cacheEo &&  cacheEoRo && !cacheDfa) ||
						(!cacheLf && !cacheEo && !cacheEoRo &&  cacheDfa),
						ERROR_MESSAGE_PREFIX L"Internal error: instruction cache corrupted."
						);

					// clear cache if it is unrelated to the current instruction
					if (cacheAvailable)
					{
						bool cacheRelated = false;

						switch (ins.type)
						{
						case AstInsType::LriStore:
							if (cacheLf) cacheRelated = true;
							break;
						case AstInsType::DelayFieldAssignment:
							if (cacheDfa && adfaToken == &token) cacheRelated = true;
							break;
						case AstInsType::EndObject:
							if ((cacheEoRo && aeoroToken == &token) && !cacheEo) cacheRelated = true;
							break;
						case AstInsType::ReopenObject:
							if ((cacheEo && cachedToken == &token) && (!cacheEoRo || aeoroToken == &token)) cacheRelated = true;
							break;
						default:;
						}

						if (!cacheRelated)
						{
							ExecuteSubmitted();
							cacheAvailable = false;
						}
					}

					if (cacheAvailable)
					{
						// execute instructions with cache
						switch (ins.type)
						{
						case AstInsType::LriStore:
							{
								lriFetch = false;
								cachedToken = nullptr;
							}
							break;
						case AstInsType::DelayFieldAssignment:
							{
								adfaCount++;
							}
							break;
						case AstInsType::EndObject:
							{
								cachedIns = ins;
								cachedIndex = tokenIndex;
								cachedToken = &token;
							}
							break;
						case AstInsType::ReopenObject:
							{
								if (cacheEoRo)
								{
									aeoroCount++;
								}
								else
								{
									aeoroCount = 1;
									aeoroIndex = tokenIndex;
									aeoroToken = &token;
								}
								cachedToken = nullptr;
							}
							break;
						default:
							CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Internal error: unrelated cache should have been cleared.");
						}
					}
					else
					{
						// execute instructions without cache
						switch (ins.type)
						{
						case AstInsType::LriFetch:
							{
								lriFetch = true;
								cachedIns = ins;
								cachedIndex = tokenIndex;
								cachedToken = &token;
							}
							break;
						case AstInsType::DelayFieldAssignment:
							{
								adfaCount = 1;
								adfaIndex = tokenIndex;
								adfaToken = &token;
							}
							break;
						case AstInsType::EndObject:
							{
								cachedIns = ins;
								cachedIndex = tokenIndex;
								cachedToken = &token;
							}
							break;
						default:
							receiver->Execute(ins, token, tokenIndex);
						}
					}
#undef ERROR_MESSAGE_PREFIX
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
						receiver->Execute(cachedIns, *cachedToken, cachedIndex);
						cachedToken = nullptr;
						lriFetch = false;
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
						AstIns ins = { AstInsType::ResolveAmbiguity,step->et_ra.type,step->et_ra.count };
						auto raTrace = GetTrace(Ref<Trace>(step->et_ra.trace));
						raTrace = EnsureTraceWithValidStates(raTrace);
						auto raToken = raTrace->currentTokenIndex;
						submitter.Submit(ins, tokens[raToken], raToken);
					}
					break;
				default:;
				}
			}

			Ptr<ParsingAstBase> TraceManager::ExecuteTrace(IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens)
			{
				CHECK_ERROR(state == TraceManagerState::ResolvedAmbiguity, ERROR_MESSAGE_PREFIX L"Wrong timing to call this function.");

				TraceManagerSubmitter submitter;
				submitter.receiver = &receiver;

				// execute from the first step
				auto step = GetInitialExecutionStep();
				CHECK_ERROR(step != nullptr, L"Internal error: execution steps not built!");
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