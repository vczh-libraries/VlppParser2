#include "SyntaxBase.h"

namespace vl
{
	namespace glr
	{
/***********************************************************************
ErrorArgs
***********************************************************************/

		ErrorArgs ErrorArgs::UnrecognizedToken(const regex::RegexToken& token)
		{
			return {
				true,
				ErrorType::UnrecognizedToken,
				token.codeIndex,
				const_cast<regex::RegexToken&>(token),
				*static_cast<collections::List<regex::RegexToken>*>(nullptr),
				*static_cast<automaton::Executable*>(nullptr),
				*static_cast<automaton::TraceManager*>(nullptr),
				nullptr
			};
		}

		ErrorArgs ErrorArgs::InvalidToken(regex::RegexToken& token, collections::List<regex::RegexToken>& tokens, automaton::Executable& executable, automaton::TraceManager& traceManager)
		{
			return {
				true,
				ErrorType::InvalidToken,
				token.codeIndex,
				token,
				tokens,
				executable,
				traceManager,
				nullptr
			};
		}

		ErrorArgs ErrorArgs::InputIncomplete(vint codeIndex, collections::List<regex::RegexToken>& tokens, automaton::Executable& executable, automaton::TraceManager& traceManager)
		{
			return {
				true,
				ErrorType::InputIncomplete,
				codeIndex,
				*static_cast<regex::RegexToken*>(nullptr),
				tokens,
				executable,
				traceManager,
				nullptr
			};
		}

		ErrorArgs ErrorArgs::UnexpectedAstType(collections::List<regex::RegexToken>& tokens, automaton::Executable& executable, automaton::TraceManager& traceManager, Ptr<ParsingAstBase> ast)
		{
			return {
				true,
				ErrorType::UnexpectedAstType,
				ast->codeRange.codeIndex,
				*static_cast<regex::RegexToken*>(nullptr),
				tokens,
				executable,
				traceManager,
				ast
			};
		}

		ParsingError ErrorArgs::ToParsingError()
		{
			switch (error)
			{
			case ErrorType::UnrecognizedToken:
				return {
					nullptr,
					{&token,&token},
					WString::Unmanaged(L"Unrecognized token: \"") + WString::CopyFrom(token.reading,token.length) + WString::Unmanaged(L"\".")
				};
			case ErrorType::InvalidToken:
				return {
					nullptr,
					{&token,&token},
					WString::Unmanaged(L"Parser stops at incorrect input: \"") + WString::CopyFrom(token.reading,token.length) + WString::Unmanaged(L"\".")
				};
			case ErrorType::InputIncomplete:
				if (tokens.Count() == 0)
				{
					return {
						nullptr,
						{&tokens[tokens.Count()-1],&tokens[tokens.Count() - 1]},
						L"Input is incomplete."
					};
				}
				else
				{
					return {
						nullptr,
						{{0,0,0},{0,0,0},codeIndex},
						L"Input is incomplete."
					};
				}
			case ErrorType::UnexpectedAstType:
				return {
					nullptr,
					ast->codeRange,
					L"Unexpected type of the created AST."
				};
			default:
				CHECK_FAIL(L"vl::glr::ErrorArgs::ToParsingError()#Unknown error type.");
			}
		}
	}
}