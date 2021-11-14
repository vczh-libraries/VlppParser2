#include "../../../Source/Lexer/LexerCppGen.h"
#include "../../Source/Calculator/Parser/Calculator_Assembler.h"
#include "../../Source/Calculator/Parser/CalculatorAst_Json.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::regex;
using namespace vl::glr;
using namespace vl::glr::parsergen;
using namespace calculator;

extern WString GetExePath();
extern void GenerateCalculatorLexer(LexerSymbolManager& manager);

namespace TestAstAssembler_TestObjects
{
	template<typename T>
	void AssertAst(Ptr<T> ast, const wchar_t* output)
	{
		List<WString> expected, actual;
		{
			StringReader reader(output);
			while (!reader.IsEnd())
			{
				expected.Add(reader.ReadLine());
			}
		}
		{
			MemoryStream actualStream;
			{
				StreamWriter writer(actualStream);
				json_visitor::AstVisitor visitor(writer);
				visitor.printAstCodeRange = false;
				visitor.printTokenCodeRange = false;
				visitor.Print(ast.Obj());
			}
			actualStream.SeekFromBegin(0);
			{
				StreamReader reader(actualStream);
				while (!reader.IsEnd())
				{
					expected.Add(reader.ReadLine());
				}
			}
		}
		TEST_ASSERT(CompareEnumerable(expected, actual) == 0);
	}
}
using namespace TestAstAssembler_TestObjects;

TEST_FILE
{
	ParserSymbolManager global;
	LexerSymbolManager lexerManager(global);
	GenerateCalculatorLexer(lexerManager);

	RegexLexer lexer(
		From(lexerManager.TokenOrder())
			.Select([&](const WString& name) { return lexerManager.Tokens()[name]->regex; })
		);
	TEST_CASE_ASSERT(lexerManager.TokenOrder().IndexOf(L"SPACE") == 23);

#define LEXER(INPUT, NAME)\
		List<RegexToken> NAME;\
		lexer.Parse(INPUT).ReadToEnd(NAME, [](vint id) { return id == 23; })\

	TEST_CASE(L"export 1")
	{
		auto input = LR"(
export 1
)";
		LEXER(input, tokens);
		CalculatorAstInsReceiver receiver;
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::Module }, tokens[0]);
		receiver.Execute({ AstInsType::BeginObject, (vint32_t)CalculatorClasses::NumExpr }, tokens[1]);
		receiver.Execute({ AstInsType::Token }, tokens[1]);
		receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::NumExpr_value }, tokens[1]);
		receiver.Execute({ AstInsType::EndObject }, tokens[1]);
		receiver.Execute({ AstInsType::Field, (vint32_t)CalculatorFields::Module_exported }, tokens[1]);
		receiver.Execute({ AstInsType::EndObject }, tokens[1]);
		auto node = receiver.Finished();
		auto ast = node.Cast<Module>();
		TEST_ASSERT(ast);
		AssertAst(ast, LR"({
    "$ast": "Module",
    "exported": {
        "$ast": "NumExpr",
        "value": "1"
    }
})");
	});

#undef LEXER
}