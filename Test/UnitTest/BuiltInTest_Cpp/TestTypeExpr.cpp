#include "../../Source/BuiltIn-Cpp/Generated/CppParser.h"
#include "../../Source/BuiltIn-Cpp/Generated/CppAst_Json.h"
#include "../../Source/LogTrace.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::regex;
using namespace cpp_parser;

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

template<typename ...TArgs>
struct AssertPtrStruct
{
};

template<>
struct AssertPtrStruct<>
{
	static void AssertPtr(CppTypeOrExpr* ast)
	{
		TEST_ASSERT(false);
	}
};

template<typename T, typename ...TArgs>
struct AssertPtrStruct<T, TArgs...>
{
	static void AssertPtr(CppTypeOrExpr* ast)
	{
		if (!dynamic_cast<T*>(ast)) AssertPtrStruct<TArgs...>::AssertPtr(ast);
	}
};

template<typename ...TArgs>
void ParseTypeExpr(cpp_parser::Parser& parser, const WString& code)
{
	auto ast = parser.Parse_TypeOrExpr(code);
	TEST_ASSERT(ast);
	AssertPtrStruct<TArgs...>::AssertPtr(ast.Obj());
}

void TracedTests()
{
	cpp_parser::Parser parser;
	WString indexName;
	WString caseName;
	FilePath dirOutput = GetOutputDir(L"BuiltIn-Cpp");

	parser.OnTraceProcessing.Add(
		[&](TraceProcessingArgs& args)
		{
			auto& traceManager = *dynamic_cast<TraceManager*>(args.executor);
			LogTraceManager(
				L"BuiltIn-Cpp",
				indexName + L"_" + caseName,
				args.executable,
				traceManager,
				args.phase,
				args.tokens,
				[=](vint32_t type) { return WString::Unmanaged(CppTypeName((CppClasses)type)); },
				[=](vint32_t field) { return WString::Unmanaged(CppFieldName((CppFields)field)); },
				[=](vint32_t token) { return WString::Unmanaged(CppTokenId((CppTokens)token)); },
				[=](vint32_t rule) { return WString::Unmanaged(ParserRuleName(rule)); },
				[=](vint32_t state) { return WString::Unmanaged(ParserStateLabel(state)); }
			);
		});
	parser.OnReadyToExecute.Add(
		[&](ReadyToExecuteArgs& args)
		{
			auto& traceManager = *dynamic_cast<TraceManager*>(args.executor);
			LogTraceExecution(
				L"BuiltIn-Cpp",
				indexName + L"_" + caseName,
				[=](vint32_t type) { return WString::Unmanaged(CppTypeName((CppClasses)type)); },
				[=](vint32_t field) { return WString::Unmanaged(CppFieldName((CppFields)field)); },
				[=](vint32_t token) { return WString::Unmanaged(CppTokenId((CppTokens)token)); },
				[&](IAstInsReceiver& receiver)
				{
					traceManager.ExecuteTrace(receiver, args.tokens);
				});
		});

	auto runParser = [&](const wchar_t* _indexName, const wchar_t* _caseName, auto parse)
	{
		indexName = WString::Unmanaged(_indexName);
		caseName = WString::Unmanaged(_caseName);
		auto ast = parse();
		auto astJson = PrintAstJson<json_visitor::AstVisitor>(ast);
		File(dirOutput / (L"Output[" + indexName + L"_" + caseName + L"].json")).WriteAllText(astJson, true, BomEncoder::Utf8);
	};

	TEST_CASE(L"int*")
	{
		runParser(L"TypeOrExpr", L"PointerOfInt", [&]() { return parser.Parse_TypeOrExpr(L"int*"); });
	});

	TEST_CASE(L"Name<int>")
	{
		runParser(L"TypeOrExpr", L"NameOfInt", [&]() { return parser.Parse_TypeOrExpr(L"Name<int>"); });
	});

	TEST_CASE(L"Name<A...>")
	{
		runParser(L"TypeOrExpr", L"NameOfAs", [&]() { return parser.Parse_TypeOrExpr(L"Name<A...>"); });
	});

	TEST_CASE(L"sizeof a()")
	{
		runParser(L"TypeOrExpr", L"SizeofA", [&]() { return parser.Parse_TypeOrExpr(L"sizeof a()"); });
	});
}

TEST_FILE
{
	TEST_CATEGORY(L"Traced Tests")
	{
		TracedTests();
	});

	cpp_parser::Parser parser;

	TEST_CATEGORY(L"Identifier")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Identifier.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppQualifiedName>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"PrimitiveExprs")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_PrimitiveExprs.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppExprOnly>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"PrimitiveTypes")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_PrimitiveTypes.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppTypeOnly>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"Types")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Types.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppConstType, CppVolatileType>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"Name")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Name.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppQualifiedName>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"Exprs")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Exprs.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppExprOnly>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"Declarators_Anonymous")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Declarators_Anonymous.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppDeclaratorType>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"LambdaExprs")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_LambdaExprs.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				//TEST_CASE(line)
				//{
				//	ParseTypeExpr<CppExprOnly>(parser, line);
				//});
			}
		}
	});

	TEST_CATEGORY(L"Ambiguous")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Ambiguous.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				//TEST_CASE(line)
				//{
				//	ParseTypeExpr<CppTypeOrExpr>(parser, line);
				//});
			}
		}
	});
}