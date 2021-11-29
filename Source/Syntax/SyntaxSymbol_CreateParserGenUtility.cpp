#include "SyntaxSymbolWriter.h"
#include "../../Source/ParserGen_Generated/ParserGen_Assembler.h"
#include "../../Source/ParserGen_Generated/ParserGen_Lexer.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			syntax_writer::Token tok(ParserGenTokens id)
			{
				auto d = ParserGenTokenDisplayText(id);
				auto n = ParserGenTokenId(id);
				return syntax_writer::tok(
					id,
					(d ? L"\"" + WString::Unmanaged(d) + L"\"" : WString::Unmanaged(n))
				);
			}

			syntax_writer::Token tok(ParserGenTokens id, ParserGenFields field)
			{
				auto d = ParserGenTokenDisplayText(id);
				auto n = ParserGenTokenId(id);
				return syntax_writer::tok(
					id,
					(d ? L"\"" + WString::Unmanaged(d) + L"\"" : WString::Unmanaged(n)),
					field
				);
			}
		}
	}
}