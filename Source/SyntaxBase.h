/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAXBASE
#define VCZH_PARSER2_SYNTAXBASE

#include "AstBase.h"

namespace vl
{
	namespace glr
	{
/***********************************************************************
Automaton
***********************************************************************/

		namespace automaton
		{
			struct InstructionArray
			{
				vint32_t							start = -1;
				vint32_t							count = 0;
			};

			struct ReturnIndexArray
			{
				vint32_t							start = -1;
				vint32_t							count = -1;
			};

			struct EdgeArray
			{
				vint32_t							start = -1;
				vint32_t							count = 0;
			};

			enum class EdgePriority
			{
				NoCompetition,
				HighPriority,
				LowPriority,
			};

			struct ReturnDesc
			{
				vint32_t							consumedRule = -1;
				vint32_t							returnState = -1;
				EdgePriority						priority = EdgePriority::NoCompetition;
				InstructionArray					insAfterInput;
			};

			struct EdgeDesc
			{
				vint32_t							fromState = -1;
				vint32_t							toState = -1;
				EdgePriority						priority = EdgePriority::NoCompetition;
				InstructionArray					insBeforeInput;
				InstructionArray					insAfterInput;
				ReturnIndexArray					returnIndices;
			};

			struct StateDesc
			{
				vint32_t							rule = -1;
				bool								endingState = false;
			};

			struct Executable
			{
				static constexpr vint32_t			EndOfInputInput = -1;
				static constexpr vint32_t			EndingInput = 0;
				static constexpr vint32_t			LeftrecInput = 1;
				static constexpr vint32_t			TokenBegin = 2;

				vint32_t							tokenCount = 0;
				vint32_t							ruleCount = 0;
				collections::Array<vint32_t>		ruleStartStates;		// ruleStartStates[rule] = the start state of this rule.
				collections::Array<EdgeArray>		transitions;			// transitions[state * (TokenBegin + tokenCount) + input] = edges from state with specified input.
				collections::Array<AstIns>			instructions;			// referenced by InstructionArray
				collections::Array<vint32_t>		returnIndices;			// referenced by ReturnIndexArray
				collections::Array<ReturnDesc>		returns;				// referenced by Executable::returnIndices
				collections::Array<EdgeDesc>		edges;					// referenced by EdgeArray
				collections::Array<StateDesc>		states;					// refereced by returnState/fromState/toState

				Executable() = default;
				Executable(stream::IStream& inputStream);

				void								Serialize(stream::IStream& outputStream);
			};

			struct Metadata
			{
				collections::Array<WString>			ruleNames;
				collections::Array<WString>			stateLabels;
			};

/***********************************************************************
Execution
***********************************************************************/

			template<typename T>
			class AllocateOnly : public Object
			{
			protected:
				vint											blockSize;
				vint											remains;
				collections::List<Ptr<collections::Array<T>>>	buffers;

			public:
				AllocateOnly(vint _blockSize = 65536)
					: blockSize(_blockSize)
					, remains(0)
				{
				}

				T* Get(vint32_t index)
				{
					vint row = index / blockSize;
					vint column = index % blockSize;
					CHECK_ERROR(0 <= row && row < buffers.Count(), L"vl::glr::automaton::AllocateOnly<T>::Get(vint)#Index out of range.");
					if (row == buffers.Count() - 1)
					{
						CHECK_ERROR(0 <= column && column < (blockSize - remains), L"vl::glr::automaton::AllocateOnly<T>::Get(vint)#Index out of range.");
					}
					else
					{
						CHECK_ERROR(0 <= column && column < blockSize, L"vl::glr::automaton::AllocateOnly<T>::Get(vint)#Index out of range.");
					}
					return &buffers[row]->operator[](column);
				}

				vint32_t Allocate()
				{
					if (remains == 0)
					{
						buffers.Add(new collections::Array<T>(blockSize));
						remains = blockSize;
					}
					vint index = blockSize * (buffers.Count() - 1) + (blockSize - remains);
					buffers[buffers.Count() - 1]->operator[](blockSize - remains).allocatedIndex = (vint32_t)index;
					remains--;
					return (vint32_t)index;
				}

				void Clear()
				{
					remains = 0;
					buffers.Clear();
				}
			};

			struct ReturnStack
			{
				vint32_t				allocatedIndex = -1;		// id of this ReturnStack
				vint32_t				previous = -1;				// id of the previous ReturnStack
				vint32_t				returnIndex = -1;			// index of ReturnDesc
			};

			struct TraceCollection
			{
				vint32_t				first = -1;					// first trace in the collection
				vint32_t				last = -1;					// last trace in the collection
				vint32_t				siblingPrev = -1;			// previous trace in the collection of the owned trace
				vint32_t				siblingNext = -1;			// next trace in the collection of the owned trace
			};

			struct TraceAmbiguity
			{
				vint32_t				insEndObject = -1;			// the index of the first EndObject instruction
																	// in {byEdge.insBeforeInput, byEdge.insAfterInput, executedReturn.insAfterInput} combined

				vint32_t				traceBeginObject = -1;		// id of the trace containing BeginObject
																	// that ends by the above EndObject

				vint32_t				insBeginObject = -1;		// the index of the BeginObject instruction
																	// from traceBeginObject
																	// in {byEdge.insBeforeInput, byEdge.insAfterInput, executedReturn.insAfterInput} combined

				vint32_t				ambiguityType = -1;			// when the BeginObject creates an object that later be consumed by BeginObjectLeftRecursive
																	// than the correct type is the type in BeginObjectLeftRecursive
			};

			enum class CompetitionStatus
			{
				Holding,
				HighPriorityWin,
				LowPriorityWin,
			};

			struct Competition
			{
				vint32_t				allocatedIndex = -1;
				vint32_t				next = -1;					// next active Competition

				CompetitionStatus		status = CompetitionStatus::Holding;	// if predecessors from this trace have different priority, the competition begins
																				// when the competition is over, it will be changed to HighPriorityWin or LowPriorityWin
																				// if all candidates fail, it could be Holding forever

				vint32_t				ownerTrace = -1;			// the id of the Trace that holds this competition
				vint32_t				highBet = -1;				// the id of the high bet AttendingCompetitions for this competition
				vint32_t				lowBet = -1;				// the id of the low bet AttendingCompetitions for this competition

				vint32_t				highCounter = 0;			// temporary counter for all existing high bets
																	// in the current step of input
				vint32_t				lowCounter = 0;				// temporary counter for all existing low bets
																	// in the current step of input
			};

			struct AttendingCompetitions
			{
				vint32_t				allocatedIndex = -1;		// id of this AttendingCompetitions
				vint32_t				next = -1;					// the next AttendingCompetitions
				vint32_t				competition = -1;			// the id of the Competition
				bool					forHighPriority = false;	// bet of this competition

				bool					closed = false;				// true if the competition has been closed
																	// this flag is not always updated for discarded AttendingCompetitions objects
			};

			struct RuntimeRouting
			{
				vint32_t				predecessorCount = -1;		// the number of predecessors
																	// (filled by ExecuteTrace)

				vint32_t				branchVisited = 0;			// the number of visited branches in the current loop.
																	// if these branches are contained in a larger ambiguity resolving loop, all branches could be visited multiple times
																	// (filled by ExecuteTrace)

				vint32_t				holdingCompetition = -1;	// the id of the active Competition

				vint32_t				attendingCompetitions = -1;	// a linked list containing all AttendingCompetitions that this trace is attending
																	// predecessors could share and modify the same linked list
																	// if a competition is over, node could be removed from the linked list
																	// one competition only creates two AttendingCompetitions, traces with the same bet share the object
			};

			struct Trace
			{
				vint32_t				allocatedIndex = -1;		// id of this Trace
				TraceCollection			predecessors;				// id of the predecessor Trace
				TraceCollection			successors;					// successors (filled by PrepareTraceRoute)

				vint32_t				state = -1;					// id of the current StateDesc
				vint32_t				returnStack = -1;			// id of the current ReturnStack
				vint32_t				executedReturn = -1;		// id of the executed ReturnDesc
				vint32_t				byEdge = -1;				// id of the last EdgeDesc that make this trace
				vint32_t				byInput = -1;				// the last input that make this trace
				vint32_t				currentTokenIndex = -1;		// the index of the token that is byInput

				TraceAmbiguity			ambiguity;					// where to end resolving ambiguity in instructions from this trace
																	// this member is useful when it has multiple predecessors
																	// (filled by PrepareTraceRoute)

				RuntimeRouting			runtimeRouting;				// a data structure guiding instruction execution when a trace need to be executed multiple times
																	// this member is useful when it has multiple predecessors or successors
			};

			enum class TraceManagerState
			{
				Uninitialized,
				WaitingForInput,
				Finished,
				PreparedTraceRoute,
			};

			struct TraceInsLists
			{
				InstructionArray					edgeInsBeforeInput;
				InstructionArray					edgeInsAfterInput;
				InstructionArray					returnInsAfterInput;
				vint32_t							c1;
				vint32_t							c2;
				vint32_t							c3;
			};

			class TraceManager : public Object
			{
			protected:
				TraceManagerState					state = TraceManagerState::Uninitialized;
				Executable&							executable;
				AllocateOnly<ReturnStack>			returnStacks;
				AllocateOnly<Trace>					traces;
				AllocateOnly<Competition>			competitions;
				AllocateOnly<AttendingCompetitions>	attendingCompetitions;

				collections::List<Trace*>			traces1;
				collections::List<Trace*>			traces2;

				Trace*								rootTrace = nullptr;
				vint32_t							activeCompetitions = -1;

				void								BeginSwap();
				void								AddTrace(Trace* trace);
				void								EndSwap();
				void								AddTraceToCollection(Trace* owner, Trace* element, TraceCollection(Trace::* collection));

				bool								AreReturnDescEqual(vint32_t ri1, vint32_t ri2);
				bool								AreReturnStackEqual(vint32_t r1, vint32_t r2);
				bool								AreTwoTraceEqual(vint32_t state, vint32_t returnStack, vint32_t executedReturn, vint32_t acId, Trace* candidate);

				vint32_t							AttendCompetitionIfNecessary(Trace* trace, EdgeDesc& edgeDesc);
				void								CheckAttendingCompetitionsOnEndingEdge(vint32_t acId, vint32_t returnStack);
				void								CheckBackupTracesBeforeSwapping(vint32_t currentTokenIndex);

				Trace*								WalkAlongSingleEdge(vint32_t currentTokenIndex, vint32_t input, Trace* trace, vint32_t byEdge, EdgeDesc& edgeDesc);
				void								WalkAlongLeftrecEdges(vint32_t currentTokenIndex, Trace* trace, EdgeArray& edgeArray);
				void								WalkAlongEndingEdges(vint32_t currentTokenIndex, Trace* trace, EdgeArray& edgeArray);
				void								WalkAlongEpsilonEdges(vint32_t currentTokenIndex, Trace* trace);
				void								WalkAlongTokenEdges(vint32_t currentTokenIndex, vint32_t input, Trace* trace, EdgeArray& edgeArray);

				void								ReadInstructionList(Trace* trace, TraceInsLists& insLists);
				AstIns&								ReadInstruction(vint32_t instruction, TraceInsLists& insLists);
				bool								RunInstruction(vint32_t instruction, TraceInsLists& insLists, vint32_t& objectCount);
				void								FindBalancedBeginObject(Trace*& trace, vint32_t& instruction, vint32_t& objectCount);
				void								FillAmbiguityInfoForMergingTrace(Trace* trace);
				void								FillAmbiguityInfoForPredecessorTraces(Trace* trace);
			public:
				TraceManager(Executable& _executable);

				vint32_t							concurrentCount = 0;
				collections::List<Trace*>*			concurrentTraces = nullptr;
				collections::List<Trace*>*			backupTraces = nullptr;

				ReturnStack*						GetReturnStack(vint32_t index);
				ReturnStack*						AllocateReturnStack();
				Trace*								GetTrace(vint32_t index);
				Trace*								AllocateTrace();
				Competition*						GetCompetition(vint32_t index);
				Competition*						AllocateCompetition();
				AttendingCompetitions*				GetAttendingCompetitions(vint32_t index);
				AttendingCompetitions*				AllocateAttendingCompetitions();

				void								Initialize(vint32_t startState);
				void								Input(vint32_t currentTokenIndex, vint32_t token);
				void								EndOfInput();
				Trace*								PrepareTraceRoute();
				Ptr<ParsingAstBase>					ExecuteTrace(Trace* trace, IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens);
			};
		}

/***********************************************************************
Parser
***********************************************************************/

		template<
			typename TTokens,
			typename TStates,
			typename TReceiver,
			template<TStates> class TStateTypes
		>
		class ParserBase : public Object
		{
			static_assert(std::is_enum_v<TTokens>);
			static_assert(std::is_enum_v<TStates>);
			static_assert(std::is_convertible_v<TReceiver*, IAstInsReceiver*>);

			using Deleter = bool(*)(vint);
			using TokenList = collections::List<regex::RegexToken>;
			using Callback = void(TokenList&, automaton::Executable&, automaton::TraceManager&, automaton::Trace*);
		protected:
			Deleter									deleter;
			Ptr<regex::RegexLexer>					lexer;
			Ptr<automaton::Executable>				executable;

		public:
			Event<Callback>							OnEndOfInput;

			ParserBase(
				Deleter _deleter,
				void(*_lexerData)(stream::IStream&),
				void(*_parserData)(stream::IStream&)
			) : deleter(_deleter)
			{
				{
					stream::MemoryStream data;
					_lexerData(data);
					data.SeekFromBegin(0);
					lexer = new regex::RegexLexer(data);
				}
				{
					stream::MemoryStream data;
					_parserData(data);
					data.SeekFromBegin(0);
					executable = new automaton::Executable(data);
				}
			}

			template<TStates State>
			auto Parse(const WString& input, vint codeIndex = -1)
			{
				#define ERROR_MESSAGE_PREFIX L"vl::glr::ParserBase<...>::ParseWithReceiver<TReceiver2>(const WString&, TState, TReceiver2&, vint)#"

				TokenList tokens;
				lexer->Parse(input, {}, codeIndex).ReadToEnd(tokens, deleter);

				automaton::TraceManager tm(*executable.Obj());
				tm.Initialize((vint32_t)State);
				for (vint32_t i = 0; i < tokens.Count(); i++)
				{
					auto&& token = tokens[i];
					tm.Input(i, (vint32_t)token.token);
					// TODO: log errors instead of crashing (failed to parse)
					CHECK_ERROR(tm.concurrentCount > 0, ERROR_MESSAGE_PREFIX L"Error happens during parsing.");
				}

				tm.EndOfInput();
				auto rootTrace = tm.PrepareTraceRoute();
				OnEndOfInput(tokens, *executable.Obj(), tm, rootTrace);
				// TODO: log errors instead of crashing (input not complete, unresolvable ambiguity)
				CHECK_ERROR(tm.concurrentCount == 1, ERROR_MESSAGE_PREFIX L"Ambiguity not fully resolved.");
				CHECK_ERROR(executable->states[tm.concurrentTraces->Get(0)->state].endingState, ERROR_MESSAGE_PREFIX L"Input is incomplete.");

				TReceiver receiver;
				auto ast = tm.ExecuteTrace(rootTrace, receiver, tokens);
				auto typedAst = ast.Cast<typename TStateTypes<State>::Type>();
				CHECK_ERROR(typedAst, ERROR_MESSAGE_PREFIX L"#Unexpected type of the created AST.");
				return typedAst;

				#undef ERROR_MESSAGE_PREFIX
			}
		};
	}
}

#endif