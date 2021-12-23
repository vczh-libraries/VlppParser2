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
				break;
			case ErrorType::InvalidToken:
				break;
			case ErrorType::InputIncomplete:
				break;
			case ErrorType::UnexpectedAstType:
				break;
			default:
				CHECK_FAIL(L"vl::glr::ErrorArgs::ToParsingError()#Unknown error type.");
			}
		}
	}
}