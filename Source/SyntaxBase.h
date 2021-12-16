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

		protected:
			template<TStates State>
			auto Parse(const WString& input, automaton::TraceManager::ITypeCallback* typeCallback, vint codeIndex)
			{
				#define ERROR_MESSAGE_PREFIX L"vl::glr::ParserBase<...>::ParseWithReceiver<TReceiver2>(const WString&, TState, TReceiver2&, vint)#"

				TokenList tokens;
				lexer->Parse(input, {}, codeIndex).ReadToEnd(tokens, deleter);

				automaton::TraceManager tm(*executable.Obj(), typeCallback);
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