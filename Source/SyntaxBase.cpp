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
				SERIALIZE(insAfterInput)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(EdgeDesc)
				SERIALIZE(fromState)
				SERIALIZE(toState)
				SERIALIZE(insBeforeInput)
				SERIALIZE(insAfterInput)
				SERIALIZE(returnIndices)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(StateDesc)
				SERIALIZE(rule)
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
			using namespace collections;
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

				auto&& ownerCollection = owner->*collection;
				auto&& elementCollection = element->*collection;
				CHECK_ERROR(elementCollection.siblingNext == -1, errorMessage);
				CHECK_ERROR(elementCollection.siblingPrev == -1, errorMessage);

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

			TraceManager::TraceManager(Executable& _executable)
				:executable(_executable)
			{
			}

			ReturnStack* TraceManager::GetReturnStack(vint index)
			{
				return returnStacks.Get(index);
			}

			ReturnStack* TraceManager::AllocateReturnStack()
			{
				auto returnStack = returnStacks.Get(returnStacks.Allocate());
				returnStack->previous = -1;
				returnStack->returnIndex = -1;
				return returnStack;
			}

			Trace* TraceManager::GetTrace(vint index)
			{
				return traces.Get(index);
			}

			Trace* TraceManager::AllocateTrace()
			{
				auto trace = traces.Get(traces.Allocate());

				trace->predecessors.first = -1;
				trace->predecessors.last = -1;
				trace->predecessors.siblingPrev = -1;
				trace->predecessors.siblingNext = -1;

				trace->successors.first = -1;
				trace->successors.last = -1;
				trace->successors.siblingPrev = -1;
				trace->successors.siblingNext = -1;

				trace->state = -1;
				trace->returnStack = -1;
				trace->executedReturn = -1;
				trace->byEdge = -1;
				trace->byInput = -1;
				trace->previousTokenIndex = -1;
				trace->currentTokenIndex = -1;
				trace->traceBeginObject = -1;
				trace->traceAfterBranch = -1;
				return trace;
			}

			void TraceManager::Initialize(vint startState)
			{
				returnStacks.Clear();
				traces.Clear();
				traces1.Clear();
				traces2.Clear();
				concurrentTraces = &traces1;
				backupTraces = &traces2;

				auto trace = AllocateTrace();
				trace->state = startState;
				concurrentCount = 1;
				concurrentTraces->Add(trace);
			}

/***********************************************************************
TraceManager::Input
***********************************************************************/

			Trace* TraceManager::WalkAlongSingleEdge(
				vint previousTokenIndex,
				vint currentTokenIndex,
				vint input,
				Trace* trace,
				vint byEdge,
				EdgeDesc& edgeDesc
			)
			{
				vint state = edgeDesc.toState;
				vint returnStack = trace->returnStack;
				vint executedReturn = -1;

				if (input == Executable::EndingInput)
				{
					CHECK_ERROR(edgeDesc.returnIndices.count == 0, L"vl::glr::automaton::TraceManager::WalkAlongSingleEdge(vint, vint, vint, Trace*, vint, EdgeDesc&)#Ending input edge is not allowed to push the return stack.");
					if (returnStack != -1)
					{
						auto rs = GetReturnStack(returnStack);
						returnStack = rs->previous;
						executedReturn = rs->returnIndex;
						state = executable.returns[executedReturn].returnState;
					}
				}

				for (vint i = 0; i < concurrentCount; i++)
				{
					auto candidate = backupTraces->Get(i);
					if (state == candidate->state && executedReturn == candidate->executedReturn)
					{
						auto r1 = returnStack;
						auto r2 = candidate->returnStack;
						while (true)
						{
							if (r1 == r2) goto MERGABLE_TRACE_FOUND;
							auto rs1 = GetReturnStack(r1);
							auto rs2 = GetReturnStack(r2);
							if (rs1->returnIndex != rs2->returnIndex)
							{
								auto& rd1 = executable.returns[rs1->returnIndex];
								auto& rd2 = executable.returns[rs2->returnIndex];
								if (rd1.returnState != rd2.returnState) goto MERGABLE_TRACE_NOT_FOUND;
								if (rd1.insAfterInput.count != rd2.insAfterInput.count) goto MERGABLE_TRACE_NOT_FOUND;
								for (vint insRef = 0; insRef < rd1.insAfterInput.count; insRef++)
								{
									auto& ins1 = executable.instructions[rd1.insAfterInput.start + insRef];
									auto& ins2 = executable.instructions[rd2.insAfterInput.start + insRef];
									if (ins1 != ins2) goto MERGABLE_TRACE_NOT_FOUND;
								}
							}
							r1 = rs1->previous;
							r2 = rs2->previous;
						}
					MERGABLE_TRACE_FOUND:
						AddTraceToCollection(candidate, trace, &Trace::predecessors);
						return nullptr;
					}
				MERGABLE_TRACE_NOT_FOUND:;
				}

				auto newTrace = AllocateTrace();
				AddTrace(newTrace);

				newTrace->predecessors.first = trace->allocatedIndex;
				newTrace->predecessors.last = trace->allocatedIndex;
				newTrace->state = state;
				newTrace->returnStack = returnStack;
				newTrace->executedReturn = executedReturn;
				newTrace->byEdge = byEdge;
				newTrace->byInput = input;
				newTrace->previousTokenIndex = previousTokenIndex;
				newTrace->currentTokenIndex = currentTokenIndex;

				for (vint returnRef = 0; returnRef < edgeDesc.returnIndices.count; returnRef++)
				{
					vint returnIndex = executable.returnIndices[edgeDesc.returnIndices.start + returnRef];
					auto returnStack = AllocateReturnStack();
					returnStack->previous = newTrace->returnStack;
					returnStack->returnIndex = returnIndex;
					newTrace->returnStack = returnStack->allocatedIndex;
				}

				return newTrace;
			}

			void TraceManager::WalkAlongTokenEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				vint input,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					if (auto newTrace = WalkAlongSingleEdge(previousTokenIndex, currentTokenIndex, input, trace, byEdge, edgeDesc))
					{
						WalkAlongEpsilonEdges(previousTokenIndex, currentTokenIndex, newTrace);
					}
				}
			}

			void TraceManager::WalkAlongEpsilonEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				Trace* trace
			)
			{
				{
					vint transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + Executable::LeftrecInput;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongLeftrecEdges(previousTokenIndex, currentTokenIndex, trace, edgeArray);
				}
				{
					vint transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + Executable::EndingInput;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongEndingEdges(previousTokenIndex, currentTokenIndex, trace, edgeArray);
				}
			}

			void TraceManager::WalkAlongLeftrecEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					WalkAlongSingleEdge(previousTokenIndex, currentTokenIndex, Executable::LeftrecInput, trace, byEdge, edgeDesc);
				}
			}

			void TraceManager::WalkAlongEndingEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					if (auto newTrace = WalkAlongSingleEdge(previousTokenIndex, currentTokenIndex, Executable::EndingInput, trace, byEdge, edgeDesc))
					{
						WalkAlongEpsilonEdges(previousTokenIndex, currentTokenIndex, newTrace);
					}
				}
			}

			void TraceManager::Input(vint currentTokenIndex, vint token)
			{
				vint traceCount = concurrentCount;
				vint previousTokenIndex = currentTokenIndex - 1;
				vint input = Executable::TokenBegin + token;

				BeginSwap();
				for (vint traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					vint transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + input;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongTokenEdges(previousTokenIndex, currentTokenIndex, input, trace, edgeArray);
				}
				EndSwap();

				for (vint traceIndex = concurrentCount; traceIndex < concurrentTraces->Count(); traceIndex++)
				{
					concurrentTraces->Set(traceIndex, nullptr);
				}
			}

			void TraceManager::EndOfInput()
			{
				vint traceCount = concurrentCount;
				BeginSwap();
				for (vint traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					auto& stateDesc = executable.states[trace->state];
					if (trace->returnStack == -1 && stateDesc.endingState)
					{
						AddTrace(trace);
					}
				}
				EndSwap();
			}

/***********************************************************************
TraceManager::PrepareTraceRoute
***********************************************************************/

			Trace* TraceManager::PrepareTraceRoute()
			{
				Trace* rootTrace = nullptr;
				SortedList<Trace*> available;
				List<Trace*> visited;

				for (vint i = 0; i < concurrentCount; i++)
				{
					auto trace = concurrentTraces->Get(i);
					visited.Add(trace);
				}

				for (vint i = 0; i < visited.Count(); i++)
				{
					auto visiting = visited[i];
					if (available.Contains(visiting)) continue;
					available.Add(visiting);

					if (visiting->predecessors.first == -1)
					{
						CHECK_ERROR(rootTrace == nullptr, L"vl::glr::automaton::TraceManager::PrepareTraceRoute()#Impossible to have more than one root trace.");
						rootTrace = visiting;
					}

					auto predecessorId = visiting->predecessors.first;
					while (predecessorId != -1)
					{
						auto predecessor = GetTrace(predecessorId);
						AddTraceToCollection(predecessor, visiting, &Trace::successors);
						predecessorId = predecessor->predecessors.siblingNext;
						visited.Add(predecessor);
					}
				}
				return rootTrace;
			}

/***********************************************************************
TraceManager::ExecuteTrace
***********************************************************************/

			struct TraceManagerSubmitter
			{
				AstIns*					submittedInstruction = nullptr;
				regex::RegexToken*		submittedToken = nullptr;
				IAstInsReceiver*		receiver = nullptr;

				void Submit(AstIns& ins, regex::RegexToken& token)
				{
					if (submittedInstruction && submittedInstruction->type == AstInsType::EndObject)
					{
						if (ins.type == AstInsType::ReopenObject)
						{
							submittedInstruction = nullptr;
							submittedToken = nullptr;
							return;
						}
					}

					ExecuteSubmitted();
					submittedInstruction = &ins;
					submittedToken = &token;
				}

				void ExecuteSubmitted()
				{
					if (submittedInstruction)
					{
						receiver->Execute(*submittedInstruction, *submittedToken);
						submittedInstruction = nullptr;
						submittedToken = nullptr;
					}
				}
			};

			Ptr<ParsingAstBase> TraceManager::ExecuteTrace(Trace* trace, IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens)
			{
				TraceManagerSubmitter submitter;
				submitter.receiver = &receiver;

				while (trace)
				{
					if (trace->byEdge != -1)
					{
						auto& edgeDesc = executable.edges[trace->byEdge];
						for (vint insRef = 0; insRef < edgeDesc.insBeforeInput.count; insRef++)
						{
							vint insIndex = edgeDesc.insBeforeInput.start + insRef;
							auto& ins = executable.instructions[insIndex];
							auto& token = tokens[trace->currentTokenIndex];
							submitter.Submit(ins, token);
						}
						for (vint insRef = 0; insRef < edgeDesc.insAfterInput.count; insRef++)
						{
							vint insIndex = edgeDesc.insAfterInput.start + insRef;
							auto& ins = executable.instructions[insIndex];
							auto& token = tokens[trace->currentTokenIndex];
							submitter.Submit(ins, token);
						}
					}

					if (trace->executedReturn != -1)
					{
						auto& returnDesc = executable.returns[trace->executedReturn];
						for (vint insRef = 0; insRef < returnDesc.insAfterInput.count; insRef++)
						{
							vint insIndex = returnDesc.insAfterInput.start + insRef;
							auto& ins = executable.instructions[insIndex];
							auto& token = tokens[trace->currentTokenIndex];
							submitter.Submit(ins, token);
						}
					}

					if (trace->successors.first == -1)
					{
						trace = nullptr;
					}
					else
					{
						CHECK_ERROR(trace->successors.first == trace->successors.last, L"vl::glr::automaton::TraceManager::ExecuteTrace(Trace*, IAstInsReceiver&, List<RegexToken>&)#Ambiguous trace not implemented.");
						trace = GetTrace(trace->successors.first);
					}
				}

				submitter.ExecuteSubmitted();
				return receiver.Finished();
			}
		}
	}
}