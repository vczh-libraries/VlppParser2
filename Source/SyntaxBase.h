/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAXBASE
#define VCZH_PARSER2_SYNTAXBASE

#include "Executable.h"

namespace vl
{
	namespace glr
	{
/***********************************************************************
ParserBase<TTokens, TStates, TReceiver, TStateTypes>
***********************************************************************/

		enum class ErrorType
		{
			UnrecognizedToken,		// (token)										the token is not recognizable to the tokenizer
			InvalidToken,			// (token, tokens, executable, traceManager)	the token cause the parser to stop
			InputIncomplete,		// (tokens, executable, traceManager)			all traces do not reach the end
			UnexpectedAstType,		// (tokens, executable, traceManager, ast)		unexpected type of the created AST
		};

		enum class TraceProcessingPhase
		{
			EndOfInput,
			PrepareTraceRoute,
		};

		struct TraceProcessingArgs
		{
			collections::List<regex::RegexToken>&			tokens;
			automaton::Executable&							executable;
			automaton::IExecutor*							executor;
			bool											ambiguityInvolved;
			TraceProcessingPhase							phase;
		};

		struct ReadyToExecuteArgs
		{
			collections::List<regex::RegexToken>& tokens;
			automaton::Executable& executable;
			automaton::IExecutor* executor;
			bool											ambiguityInvolved;
		};

		struct ErrorArgs
		{
			bool											throwError;
			ErrorType										error;
			vint											codeIndex;
			regex::RegexToken&								token;
			collections::List<regex::RegexToken>&			tokens;
			automaton::Executable&							executable;
			automaton::IExecutor*							executor;
			Ptr<ParsingAstBase>								ast;

			static ErrorArgs								UnrecognizedToken	(const regex::RegexToken& token);
			static ErrorArgs								InvalidToken		(regex::RegexToken& token, collections::List<regex::RegexToken>& tokens, automaton::Executable& executable, automaton::IExecutor* executor);
			static ErrorArgs								InputIncomplete		(vint codeIndex, collections::List<regex::RegexToken>& tokens, automaton::Executable& executable, automaton::IExecutor* executor);
			static ErrorArgs								UnexpectedAstType	(collections::List<regex::RegexToken>& tokens, automaton::Executable& executable, automaton::IExecutor* executor, Ptr<ParsingAstBase> ast);

			ParsingError									ToParsingError();
		};

		template<typename TParser>
		Ptr<EventHandler> InstallDefaultErrorMessageGenerator(TParser& parser, collections::List<ParsingError>& errors)
		{
			return parser.OnError.Add([&errors](ErrorArgs& args)
			{
				args.throwError = false;
				errors.Add(args.ToParsingError());
			});
		}

		template<
			typename TTokens,
			typename TStates,
			typename TReceiver
		>
		class ParserBase : public Object
		{
			static_assert(std::is_enum_v<TTokens>);
			static_assert(std::is_enum_v<TStates>);
			static_assert(std::is_convertible_v<TReceiver*, IAstInsReceiver*>);

			using Deleter = bool(*)(vint);
			using TokenList = collections::List<regex::RegexToken>;
			using TraceProcessingCallback = void(TraceProcessingArgs&);
			using ReadyToExecuteCallback = void(ReadyToExecuteArgs&);
			using ErrorCallback = void(ErrorArgs&);
		protected:
			Deleter									deleter;
			Ptr<regex::RegexLexer>					lexer;
			Ptr<automaton::Executable>				executable;

		public:
			Event<TraceProcessingCallback>			OnTraceProcessing;
			Event<ReadyToExecuteCallback>			OnReadyToExecute;
			Event<ErrorCallback>					OnError;

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

			regex::RegexLexer& Lexer() const
			{
				return *lexer.Obj();
			}

			Deleter LexerDeleter() const
			{
				return deleter;
			}

			void Tokenize(const WString& input, TokenList& tokens, vint codeIndex = -1) const
			{
				input.Buffer();
				auto enumerable = lexer->Parse(input, {}, codeIndex);
				Ptr<collections::IEnumerator<regex::RegexToken>> enumerator = enumerable.CreateEnumerator();
				while (enumerator->Next())
				{
					auto&& token = enumerator->Current();
					if (token.token == -1 || !token.completeToken)
					{
						auto args = ErrorArgs::UnrecognizedToken(token);
						args.throwError = false;
						OnError(args);
						if (args.throwError)
						{
							CHECK_FAIL(L"vl::glr::ParserBase<...>::Tokenize(const WString&, List<RegexToken>&, vint)#Unrecognized token.");
						}
					}
					else if (!deleter(token.token))
					{
						tokens.Add(token);
					}
				}
			}

		protected:
			Ptr<ParsingAstBase> ParseInternal(TokenList& tokens, vint32_t state, automaton::IExecutor* executor, const automaton::IExecutor::ITypeCallback* typeCallback, vint codeIndex) const
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::ParserBase<...>::ParseInternal(List<RegexToken>&, vint32_t TraceManager::ITypeCallback*)#"
				if (codeIndex == -1 && tokens.Count() > 0)
				{
					codeIndex = tokens[0].codeIndex;
				}

				executor->Initialize(state);
				for (vint32_t i = 0; i < tokens.Count(); i++)
				{
					auto token = &tokens[i];
					auto lookAhead = i == tokens.Count() - 1 ? nullptr : &tokens[i + 1];

					if (!executor->Input(i, token, lookAhead))
					{
						auto args = ErrorArgs::InvalidToken(*token, tokens, *executable.Obj(), executor);
						OnError(args);
						if (args.throwError) CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Error happens during parsing.");
						return nullptr;
					}
				}

				bool ambiguityInvolved = false;
				if (!executor->EndOfInput(ambiguityInvolved))
				{
					auto args = ErrorArgs::InputIncomplete(codeIndex, tokens, *executable.Obj(), executor);
					OnError(args);
					if (args.throwError) CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Input is incomplete.");
					return nullptr;
				}

				{
					TraceProcessingArgs args = { tokens, *executable.Obj(), executor, ambiguityInvolved, TraceProcessingPhase::EndOfInput };
					OnTraceProcessing(args);
				}
				if (ambiguityInvolved)
				{
					{
						executor->PrepareTraceRoute();
						TraceProcessingArgs args = { tokens, *executable.Obj(), executor, ambiguityInvolved, TraceProcessingPhase::PrepareTraceRoute };
						OnTraceProcessing(args);
					}
				}
				{
					ReadyToExecuteArgs args = { tokens, *executable.Obj(), executor, ambiguityInvolved };
					OnReadyToExecute(args);
				}

				TReceiver receiver;
				return executor->ExecuteTrace(receiver, tokens);

#undef ERROR_MESSAGE_PREFIX
			}

			template<typename TAst, TStates State>
			Ptr<TAst> ParseWithTokens(TokenList& tokens, const automaton::IExecutor::ITypeCallback* typeCallback, vint codeIndex) const
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::ParserBase<...>::Parse<TAst, TStates>(List<RegexToken>& TraceManager::ITypeCallback*)#"
				auto executor = automaton::CreateExecutor(*executable.Obj(), typeCallback);
				auto ast = ParseInternal(tokens, (vint32_t)State, executor.Obj(), typeCallback, codeIndex);
				auto typedAst = ast.template Cast<TAst>();

				if (ast && !typedAst)
				{
					auto args = ErrorArgs::UnexpectedAstType(tokens, *executable.Obj(), executor.Obj(), ast);
					OnError(args);
					if (args.throwError) CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unexpected type of the created AST.");
				}
				return typedAst;
#undef ERROR_MESSAGE_PREFIX
			}

			template<typename TAst, TStates State>
			Ptr<TAst> ParseWithString(const WString& input, const automaton::IExecutor::ITypeCallback* typeCallback, vint codeIndex) const
			{
				TokenList tokens;
				Tokenize(input, tokens, codeIndex);
				return ParseWithTokens<TAst, State>(tokens, typeCallback, codeIndex);
			}
		};
	}
}

#endif