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
				vint								start = -1;
				vint								count = 0;
			};

			struct ReturnIndexArray
			{
				vint								start = -1;
				vint								count = -1;
			};

			struct EdgeArray
			{
				vint								start = -1;
				vint								count = 0;
			};

			struct ReturnDesc
			{
				vint								consumedRule = -1;
				vint								returnState = -1;
				InstructionArray					insAfterInput;
			};

			struct EdgeDesc
			{
				vint								fromState = -1;
				vint								toState = -1;
				InstructionArray					insBeforeInput;
				InstructionArray					insAfterInput;
				ReturnIndexArray					returnIndices;
			};

			struct StateDesc
			{
				vint								rule = -1;
				bool								endingState = false;
			};

			struct Executable
			{
				static constexpr vint				EndOfInputInput = -1;
				static constexpr vint				EndingInput = 0;
				static constexpr vint				LeftrecInput = 1;
				static constexpr vint				TokenBegin = 2;

				vint								tokenCount = 0;
				vint								ruleCount = 0;
				collections::Array<vint>			ruleStartStates;		// ruleStartStates[rule] = the start state of this rule.
				collections::Array<EdgeArray>		transitions;			// transitions[state * (TokenBegin + tokenCount) + input] = edges from state with specified input.
				collections::Array<AstIns>			instructions;			// referenced by InstructionArray
				collections::Array<vint>			returnIndices;			// referenced by ReturnIndexArray
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

				T* Get(vint index)
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

				vint Allocate()
				{
					if (remains == 0)
					{
						buffers.Add(new collections::Array<T>(blockSize));
						remains = blockSize;
					}
					vint index = blockSize * (buffers.Count() - 1) + (blockSize - remains);
					buffers[buffers.Count() - 1]->operator[](blockSize - remains).allocatedIndex = index;
					remains--;
					return index;
				}

				void Clear()
				{
					remains = 0;
					buffers.Clear();
				}
			};

			struct ReturnStack
			{
				vint	allocatedIndex;				// id of this ReturnStack
				vint	previous;					// id of the previous ReturnStack
				vint	returnIndex;				// index of ReturnDesc
			};

			struct Trace
			{
				vint	allocatedIndex;				// id of this Trace
				vint	previous;					// id of the previous Trace
				vint	selectedNext;				// id of the selected next Trace
				vint	state;						// id of the current StateDesc
				vint	returnStack;				// id of the current ReturnStack
				vint	executedReturn;				// id of the executed ReturnDesc
				vint	byEdge;						// id of the last EdgeDesc that make this trace
				vint	byInput;					// the last input that make this trace
				vint	previousTokenIndex;			// the index of the token before byInput
				vint	currentTokenIndex;			// the index of the token that is byInput
				vint	traceBeginObject;			// the id of the Trace which contains the latest AstInsType::BeginObject
				vint	traceAfterBranch;			// the id of the Trace which is the first trace of the current branch
			};

			class TraceManager : public Object
			{
			protected:
				Executable&							executable;
				AllocateOnly<ReturnStack>			returnStacks;
				AllocateOnly<Trace>					traces;
				collections::List<Trace*>			traces1;
				collections::List<Trace*>			traces2;

				void								BeginSwap();
				void								AddTrace(Trace* trace);
				void								EndSwap();

				Trace*								WalkAlongSingleEdge(vint previousTokenIndex, vint currentTokenIndex, vint input, Trace* trace, vint byEdge, EdgeDesc& edgeDesc);
				void								WalkAlongTokenEdges(vint previousTokenIndex, vint currentTokenIndex, vint input, Trace* trace, EdgeArray& edgeArray);
				void								WalkAlongEpsilonEdges(vint previousTokenIndex, vint currentTokenIndex, Trace* trace);
				void								WalkAlongLeftrecEdges(vint previousTokenIndex, vint currentTokenIndex, Trace* trace, EdgeArray& edgeArray);
				void								WalkAlongEndingEdges(vint previousTokenIndex, vint currentTokenIndex, Trace* trace, EdgeArray& edgeArray);
			public:
				TraceManager(Executable& _executable);

				vint								concurrentCount = 0;
				collections::List<Trace*>*			concurrentTraces = nullptr;
				collections::List<Trace*>*			backupTraces = nullptr;

				ReturnStack*						GetReturnStack(vint index);
				ReturnStack*						AllocateReturnStack();
				Trace*								GetTrace(vint index);
				Trace*								AllocateTrace();

				void								Initialize(vint startState);
				void								Input(vint currentTokenIndex, vint token);
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
			Event<Callback>							OnPreparedTraceRoute;

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
				const wchar_t* errorMessage = L"vl::glr::ParserBase<...>::ParseWithReceiver<TReceiver2>(const WString&, TState, TReceiver2&, vint)#Error happens during parsing.";
				TokenList tokens;
				lexer->Parse(input, {}, codeIndex).ReadToEnd(tokens, deleter);

				automaton::TraceManager tm(*executable.Obj());
				tm.Initialize((vint)State);
				for (vint i = 0; i < tokens.Count(); i++)
				{
					auto&& token = tokens[i];
					tm.Input(i, token.token);
					// TODO: log errors instead of crashing (failed to parse)
					CHECK_ERROR(tm.concurrentCount > 0, errorMessage);
				}

				tm.EndOfInput();
				OnEndOfInput(tokens, *executable.Obj(), tm, nullptr);
				// TODO: log errors instead of crashing (input not complete, unresolvable ambiguity)
				CHECK_ERROR(tm.concurrentCount == 1, errorMessage);
				CHECK_ERROR(executable->states[tm.concurrentTraces->Get(0)->state].endingState, errorMessage);

				auto rootTrace = tm.PrepareTraceRoute();
				OnPreparedTraceRoute(tokens, *executable.Obj(), tm, rootTrace);

				TReceiver receiver;
				auto ast = tm.ExecuteTrace(rootTrace, receiver, tokens);
				auto typedAst = ast.Cast<typename TStateTypes<State>::Type>();
				CHECK_ERROR(typedAst, L"vl::glr::ParserBase<...>::Parse<State>(const WString&)#Unexpected type of the created AST.");
				return typedAst;
			}
		};
	}
}

#endif