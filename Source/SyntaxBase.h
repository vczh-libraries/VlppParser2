/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAXBASE
#define VCZH_PARSER2_SYNTAXBASE

#include "TraceManager/TraceManager.h"

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

		struct EndOfInputArgs
		{
			collections::List<regex::RegexToken>&			tokens;
			automaton::Executable&							executable;
			automaton::TraceManager&						traceManager;
			automaton::Trace*								rootTrace;
		};

		struct ErrorArgs
		{
			bool											throwError;
			ErrorType										error;
			vint											codeIndex;
			regex::RegexToken&								token;
			collections::List<regex::RegexToken>&			tokens;
			automaton::Executable&							executable;
			automaton::TraceManager&						traceManager;
			Ptr<ParsingAstBase>								ast;

			static ErrorArgs								UnrecognizedToken	(const regex::RegexToken& token);
			static ErrorArgs								InvalidToken		(regex::RegexToken& token, collections::List<regex::RegexToken>& tokens, automaton::Executable& executable, automaton::TraceManager& traceManager);
			static ErrorArgs								InputIncomplete		(vint codeIndex, collections::List<regex::RegexToken>& tokens, automaton::Executable& executable, automaton::TraceManager& traceManager);
			static ErrorArgs								UnexpectedAstType	(collections::List<regex::RegexToken>& tokens, automaton::Executable& executable, automaton::TraceManager& traceManager, Ptr<ParsingAstBase> ast);

			ParsingError									ToParsingError();
		};

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
			using EndOfInputCallback = void(EndOfInputArgs&);
			using ErrorCallback = void(ErrorArgs&);
		protected:
			Deleter									deleter;
			Ptr<regex::RegexLexer>					lexer;
			Ptr<automaton::Executable>				executable;

		public:
			Event<EndOfInputCallback>				OnEndOfInput;
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
				auto enumerable = lexer->Parse(input, {}, codeIndex);
				Ptr<collections::IEnumerator<regex::RegexToken>> enumerator = enumerable.CreateEnumerator();
				while (enumerator->Next())
				{
					auto&& token = enumerator->Current();
					if (token.token == -1)
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
			template<TStates State>
			auto Parse(TokenList& tokens, const automaton::TraceManager::ITypeCallback* typeCallback, vint codeIndex) const -> Ptr<typename TStateTypes<State>::Type>
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::ParserBase<...>::Parse<TStates>(List<RegexToken>&, TraceManager::ITypeCallback*)#"

				automaton::TraceManager tm(*executable.Obj(), typeCallback);
				tm.Initialize((vint32_t)State);
				for (vint32_t i = 0; i < tokens.Count(); i++)
				{
					auto&& token = tokens[i];
					auto lookAhead = i == tokens.Count() - 1 ? -1 : tokens[i + 1].token;
					tm.Input(i, (vint32_t)token.token, (vint32_t)lookAhead);

					if (tm.concurrentCount == 0)
					{
						auto args = ErrorArgs::InvalidToken(token, tokens, *executable.Obj(), tm);
						OnError(args);
						if (args.throwError) CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Error happens during parsing.");
						return nullptr;
					}
				}

				tm.EndOfInput();
				if (tm.concurrentCount == 0)
				{
					auto args = ErrorArgs::InputIncomplete(tokens, *executable.Obj(), tm);
					OnError(args);
					if (args.throwError) CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Input is incomplete.");
					return nullptr;
				}

				auto rootTrace = tm.PrepareTraceRoute();
				{
					EndOfInputArgs args = { tokens, *executable.Obj(), tm, rootTrace };
					OnEndOfInput(args);
				}

				TReceiver receiver;
				auto ast = tm.ExecuteTrace(rootTrace, receiver, tokens);
				auto typedAst = ast.Cast<typename TStateTypes<State>::Type>();

				if (!typedAst)
				{
					auto args = ErrorArgs::UnexpectedAstType(tokens, *executable.Obj(), tm, ast);
					OnError(args);
					if (args.throwError) CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unexpected type of the created AST.");
					return nullptr;
				}
				return typedAst;

#undef ERROR_MESSAGE_PREFIX
			}

			template<TStates State>
			auto Parse(const WString& input, const automaton::TraceManager::ITypeCallback* typeCallback, vint codeIndex) const
			{
				TokenList tokens;
				Tokenize(input, tokens, codeIndex);
				return Parse<State>(tokens, typeCallback, codeIndex);
			}
		};
	}
}

#endif