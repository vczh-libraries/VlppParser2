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
				regex::RegexToken*		adfaToken = nullptr;

				// AccumulatedEoRo
				vint32_t				aeoroCount = 0;
				regex::RegexToken*		aeoroToken = nullptr;

				// Caching
				AstIns					cachedIns;
				regex::RegexToken*		cachedToken = nullptr;

				IAstInsReceiver*		receiver = nullptr;

				void Submit(AstIns& ins, regex::RegexToken& token)
				{
					// multiple DelayFieldAssignment are compressed to single AccumulatedDfa
					// multiple EndObject+ReopenObject are compressed to single AccumulatedEoRo

					switch (ins.type)
					{
					case AstInsType::DelayFieldAssignment:
						if (aeoroToken == nullptr && cachedToken == nullptr && (adfaToken == nullptr || adfaToken == &token))
						{
							adfaCount++;
							adfaToken = &token;
						}
						else
						{
							ExecuteSubmitted();
							adfaCount = 1;
							adfaToken = &token;
						}
						break;
					case AstInsType::EndObject:
						if (adfaToken == nullptr && cachedToken == nullptr)
						{
							cachedIns = ins;
							cachedToken = &token;
						}
						else
						{
							ExecuteSubmitted();
							cachedIns = ins;
							cachedToken = &token;
						}
						break;
					case AstInsType::ReopenObject:
						if (adfaToken != nullptr || cachedToken == nullptr || cachedIns.type != AstInsType::EndObject)
						{
							ExecuteSubmitted();
							receiver->Execute(ins, token);
						}
						else if ((aeoroToken == nullptr || aeoroToken == &token) && cachedToken == &token)
						{
							aeoroCount++;
							aeoroToken = &token;
							cachedToken = nullptr;
						}
						else if (cachedToken == &token)
						{
							cachedToken = nullptr;
							ExecuteSubmitted();
							aeoroCount = 1;
							aeoroToken = &token;
						}
						else
						{
							ExecuteSubmitted();
							receiver->Execute(ins, token);
						}
						break;
					default:
						ExecuteSubmitted();
						receiver->Execute(ins, token);
					}
				}

				void ExecuteSubmitted()
				{
					if (adfaToken)
					{
						if (adfaCount == 1)
						{
							AstIns ins = { AstInsType::DelayFieldAssignment };
							receiver->Execute(ins, *adfaToken);
						}
						else
						{
							AstIns ins = { AstInsType::AccumulatedDfa,-1,adfaCount };
							receiver->Execute(ins, *adfaToken);
						}
						adfaCount = 0;
						adfaToken = nullptr;
					}
					if (aeoroToken)
					{
						AstIns ins = { AstInsType::AccumulatedEoRo,-1,aeoroCount };
						receiver->Execute(ins, *aeoroToken);
						aeoroCount = 0;
						aeoroToken = nullptr;
					}
					if (cachedToken)
					{
						receiver->Execute(cachedIns, *cachedToken);
						cachedToken = nullptr;
					}
				}
			};

			Ptr<ParsingAstBase> TraceManager::ExecuteTrace(Trace* trace, IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::ExecuteTrace(Trace*, IAstInsReceiver&, List<RegexToken>&)#"
				CHECK_ERROR(state == TraceManagerState::PreparedTraceRoute, ERROR_MESSAGE_PREFIX L"Wrong timing to call this function.");

				TraceManagerSubmitter submitter;
				submitter.receiver = &receiver;

				// execute from the root trace

				vint32_t startIns = 0;
				while (trace)
				{
					TraceInsLists insLists;
					ReadInstructionList(trace, insLists);

					// if the current trace is an ambiguity resolving trace
					// we check if all predecessors has been visited
					// if yes, we continue
					// if no, we jump to the BeginObject and repeat it again

					if (trace->ambiguity.traceBeginObject != -1 && trace->ambiguityRouting.predecessorCount == -1)
					{
						// we need to know how many predecessors there
						// the number is calculated and cached when an ambiguity resolving trace is visited for the first time
						trace->ambiguityRouting.predecessorCount = 0;
						auto predecessorId = trace->predecessors.first;
						while (predecessorId != -1)
						{
							trace->ambiguityRouting.predecessorCount++;
							predecessorId = GetTrace(predecessorId)->predecessors.siblingNext;
						}
					}

					if (trace->ambiguity.traceBeginObject != -1)
					{
						if (0 <= trace->ambiguity.insEndObject && trace->ambiguity.insEndObject < insLists.c3)
						{
							// execute the EndObject instruction if insEndObject exists
							auto& ins = ReadInstruction(trace->ambiguity.insEndObject, insLists);
							auto& token = tokens[trace->currentTokenIndex];
							submitter.Submit(ins, token);
						}
						else
						{
							// otherwise this must be the trace created by CreateLastMergingTrace
							CHECK_ERROR(insLists.c3 == 0 && trace->successors.first == -1 && trace->successors.last == -1, ERROR_MESSAGE_PREFIX L"Instruction index out of range.");
						}

						// for any ambiguity resolving trace
						// we check all predecessors has been visited
						trace->ambiguityRouting.branchVisited++;
						auto traceBeginObject = GetTrace(trace->ambiguity.traceBeginObject);

						if (trace->ambiguityRouting.branchVisited == trace->ambiguityRouting.predecessorCount)
						{
							// if all predecessors has been visited
							// we reset the number to 0
							// because TraceManager::ExecuteTrace could be called multiple time
							trace->ambiguityRouting.branchVisited = 0;
							{
								// submit a ResolveAmbiguity instruction
								auto& token = tokens[trace->currentTokenIndex];
								AstIns insResolve = { AstInsType::ResolveAmbiguity,trace->ambiguity.ambiguityType,trace->ambiguityRouting.predecessorCount };
								submitter.Submit(insResolve, token);
							}

							// execute all instructions after EndObject
							// these part should not be repeated
							for (vint32_t i = trace->ambiguity.insEndObject + 1; i < insLists.c3; i++)
							{
								auto& ins = ReadInstruction(i, insLists);
								auto& token = tokens[trace->currentTokenIndex];
								submitter.Submit(ins, token);
							}
						}
						else
						{
							// if there are unvisited predecessors
							// we jump to the BeginObject instruction and repeat it again
							startIns = trace->ambiguity.insBeginObject;
							trace = traceBeginObject;
							goto FOUND_NEXT_TRACE;
						}
					}
					else
					{
						// otherwise, just submit instructions
						vint32_t endIns = insLists.c3 - 1;
						if (trace->ambiguityInsPostfix != -1)
						{
							endIns = insLists.c1 - trace->ambiguityInsPostfix - 1;
						}

						for (vint32_t i = startIns; i <= endIns; i++)
						{
							auto& ins = ReadInstruction(i, insLists);
							auto& token = tokens[trace->currentTokenIndex];
							submitter.Submit(ins, token);
						}
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
						// if there are multiple successors
						// whenever this trace is visited
						// we pick a different successor to continue
						auto nextSuccessorId = trace->successors.first;
						Trace* successor = nullptr;
						for (vint i = 0; i <= trace->ambiguityRouting.branchVisited; i++)
						{
							CHECK_ERROR(nextSuccessorId != -1, ERROR_MESSAGE_PREFIX L"branchVisited corrupted.");
							successor = GetTrace(nextSuccessorId);
							nextSuccessorId = successor->successors.siblingNext;
						}

						if (nextSuccessorId == -1)
						{
							trace->ambiguityRouting.branchVisited = 0;
						}
						else
						{
							trace->ambiguityRouting.branchVisited++;
						}

						// this could happen when all BeginObject are in successors
						// if the current successor is the first successor
						// then we need to execute the prefix
						if (startIns >= insLists.c3)
						{
							startIns -= insLists.c3;
							if (trace->successors.first == successor->allocatedIndex)
							{
								ReadInstructionList(successor, insLists);
								for (vint32_t i = 0; i < startIns; i++)
								{
									auto& ins = ReadInstruction(i, insLists);
									auto& token = tokens[successor->currentTokenIndex];
									submitter.Submit(ins, token);
								}
							}
						}
						else
						{
							startIns = 0;
						}
						trace = successor;
					}
				FOUND_NEXT_TRACE:;
				}

				submitter.ExecuteSubmitted();
				return receiver.Finished();
#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}