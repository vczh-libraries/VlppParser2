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

			class AstInsOptimizer : public Object, public virtual IAstInsReceiver
			{
			protected:
				IAstInsReceiver&				receiver;

				bool							cachedStackBegin = false;
				const regex::RegexToken*		cachedStackBeginToken = nullptr;
				vint32_t						cachedStackBeginTokenIndex = -1;

			public:
				AstInsOptimizer(IAstInsReceiver& _receiver)
					:receiver(_receiver)
				{
				}

				void Execute(AstIns instruction, const regex::RegexToken& token, vint32_t tokenIndex) override
				{
					if (cachedStackBegin)
					{
						if (instruction.type == AstInsType::StackEnd)
						{
							cachedStackBegin = false;
							return;
						}

						receiver.Execute({ AstInsType::StackBegin }, *cachedStackBeginToken, cachedStackBeginTokenIndex);
						cachedStackBegin = false;
					}

					if (instruction.type == AstInsType::StackBegin)
					{
						cachedStackBegin = true;
						cachedStackBeginToken = &token;
						cachedStackBeginTokenIndex = tokenIndex;
						return;
					}

					receiver.Execute(instruction, token, tokenIndex);
				}

				Ptr<ParsingAstBase> Finished() override
				{
					if (cachedStackBegin)
					{
						receiver.Execute({ AstInsType::StackBegin }, *cachedStackBeginToken, cachedStackBeginTokenIndex);
						cachedStackBegin = false;
					}
					return receiver.Finished();
				}
			};

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
				default:
					{
						auto raTrace = GetTrace(Ref<Trace>(step->et_ra.trace));
						raTrace = EnsureTraceWithValidStates(raTrace);
						auto raToken = raTrace->currentTokenIndex;

						switch (step->type)
						{
						case ExecutionType::RA_Begin:
							{
								if (raToken == -1) raToken = 0;
								AstIns ins = { AstInsType::StackBegin };
								receiver.Execute(ins, tokens[raToken], raToken);
							}
							break;
						case ExecutionType::RA_Branch:
							{
								AstIns ins = { AstInsType::StackSlot,-1,ResolveAmbiguitySlotIndex };
								receiver.Execute(ins, tokens[raToken], raToken);
							}
							break;
						case ExecutionType::RA_End:
							{
								AstIns ins = { AstInsType::ResolveAmbiguity,step->et_ra.type,0 };
								receiver.Execute(ins, tokens[raToken], raToken);
							}
							{
								AstIns ins = { AstInsType::StackEnd };
								receiver.Execute(ins, tokens[raToken], raToken);
							}
							break;
						default:;
						}
					}
				}
			}

			Ptr<ParsingAstBase> TraceManager::ExecuteTrace(IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens)
			{
				CHECK_ERROR(state == TraceManagerState::ResolvedAmbiguity, ERROR_MESSAGE_PREFIX L"Wrong timing to call this function.");

				// execute from the first step
				AstInsOptimizer optimizedReceiver(receiver);
				auto step = GetInitialExecutionStep();
				CHECK_ERROR(step != nullptr, L"Internal error: execution steps not built!");
				while (step)
				{
					// execute step
					ExecuteSingleStep(optimizedReceiver, step, tokens);

					// find the next step
					step = step->next == nullref ? nullptr : GetExecutionStep(step->next);
				}

				return optimizedReceiver.Finished();
			}
#undef ERROR_MESSAGE_PREFIX
		}
	}
}