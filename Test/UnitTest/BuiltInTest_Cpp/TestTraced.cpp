#include "TestCppHelper.h"

TEST_FILE
{
	WString indexName;
	WString caseName;
	FilePath dirOutput = GetOutputDir(L"BuiltIn-Cpp");

	auto handlerOnError = GetCppParser().OnError.Add(
		[&](ErrorArgs& args)
		{
			args.throwError = true;

			TraceProcessingArgs tpArgs(args.tokens, args.executable, args.executor, false, TraceProcessingPhase::EndOfInput);
			GetCppParser().OnTraceProcessing(tpArgs);
		});

	auto handlerOnTraceProcessing = GetCppParser().OnTraceProcessing.Add(
		[&](TraceProcessingArgs& args)
		{
			TEST_PRINT(L"Printing Trace-" + itow((vint)args.phase + 1) + L"[" + caseName + L"].txt ...");
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
			TEST_PRINT(L"Finished");
		});

	auto handlerOnReadyToExecute = GetCppParser().OnReadyToExecute.Add(
		[&](ReadyToExecuteArgs& args)
		{
			TEST_PRINT(L"Printing Instructions[" + caseName + L"].txt ...");
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
			TEST_PRINT(L"Finished");
		});

	auto runParser = [&]<typename T>(const wchar_t* _indexName, const wchar_t* _caseName, auto parse)
	{
		indexName = WString::Unmanaged(_indexName);
		caseName = WString::Unmanaged(_caseName);
		auto ast = parse();
		auto astJson = PrintAstJson<json_visitor::AstVisitor>(ast);
		File(dirOutput / (L"Output[" + indexName + L"_" + caseName + L"].json")).WriteAllText(astJson, true, BomEncoder::Utf8);
		TEST_ASSERT(ast.Cast<T>());
	};

	TEST_CASE(L"true")
	{
		runParser.operator()<CppExprOnly>(L"TypeOrExpr", L"TrueExpr", [&]() { return GetCppParser().Parse_TypeOrExpr(L"true"); });
	});

	TEST_CASE(L"int")
	{
		runParser.operator()<CppTypeOnly>(L"TypeOrExpr", L"IntType", [&]() { return GetCppParser().Parse_TypeOrExpr(L"int"); });
	});

	TEST_CASE(L"int*")
	{
		runParser.operator()<CppDeclaratorType>(L"TypeOrExpr", L"PointerOfInt", [&]() { return GetCppParser().Parse_TypeOrExpr(L"int*"); });
	});

	TEST_CASE(L"Name")
	{
		runParser.operator() < CppQualifiedName > (L"TypeOrExpr", L"Name", [&]() { return GetCppParser().Parse_TypeOrExpr(L"Name"); });
	});
	
	TEST_CASE(L"Name<int>")
	{
		runParser.operator()<CppQualifiedName>(L"TypeOrExpr", L"NameOfInt", [&]() { return GetCppParser().Parse_TypeOrExpr(L"Name<int>"); });
	});
	
	TEST_CASE(L"Name<A...>")
	{
		runParser.operator()<CppQualifiedName>(L"TypeOrExpr", L"NameOfAs", [&]() { return GetCppParser().Parse_TypeOrExpr(L"Name<A...>"); });
	});
	
	TEST_CASE(L"sizeof a()")
	{
		runParser.operator()<CppExprOnly>(L"TypeOrExpr", L"SizeofA", [&]() { return GetCppParser().Parse_TypeOrExpr(L"sizeof a()"); });
	});
	
	TEST_CASE(L"[]<typename T, class ...U = int>{}")
	{
		runParser.operator()<CppLambdaExpr>(L"TypeOrExpr", L"LambdaGeneric", [&]() { return GetCppParser().Parse_TypeOrExpr(L"[]<typename T, class ...U = int>{}"); });
	});
	
	TEST_CASE(L"(T)(a)")
	{
		runParser.operator()<CppTypeOrExprToResolve>(L"TypeOrExpr", L"CallOrCast", [&]() { return GetCppParser().Parse_TypeOrExpr(L"(T)(a)"); });
	});
	
	TEST_CASE(L"T*{a}")
	{
		runParser.operator()<CppExprOnly>(L"TypeOrExpr", L"Multiply", [&]() { return GetCppParser().Parse_TypeOrExpr(L"T*{a}"); });
	});
	
	TEST_CASE(L"A<B>::C")
	{
		runParser.operator()<CppTypeOrExprToResolve>(L"TypeOrExpr", L"BExprOrQName", [&]() { return GetCppParser().Parse_TypeOrExpr(L"A<B>::C"); });
	});

	TEST_CASE(L"Name<a < b>")
	{
		runParser.operator()<CppTypeOrExprToResolve>(L"TypeOrExpr", L"BExprOrQName2", [&]() { return GetCppParser().Parse_TypeOrExpr(L"Name<a < b>"); });
	});

	TEST_CASE(L"void(int(...))")
	{
		runParser.operator()<CppTypeOnly>(L"TypeOrExpr", L"AmbiguousArgument", [&]() { return GetCppParser().Parse_TypeOrExpr(L"void(int(...))"); });
	});

	TEST_CASE(L"A<B>C;")
	{
		runParser.operator()<CppStatement>(L"Stat", L"AmbiguousStat", [&]() { return GetCppParser().Parse_Stat(L"A<B>C;"); });
	});

	TEST_CASE(L"while(int a = 0);")
	{
		runParser.operator() < CppStatement > (L"Stat", L"WhileStatement", [&]() { return GetCppParser().Parse_Stat(L"while(int a = 0);"); });
	});

	TEST_CASE(L"class X{};")
	{
		runParser.operator()<CppFile>(L"File", L"SimpleClass", [&]() { return GetCppParser().Parse_File(L"class X{};"); });
	});

	TEST_CASE(L"A X();")
	{
		runParser.operator()<CppFile>(L"File", L"AmbiguousDecl", [&]() { return GetCppParser().Parse_File(L"A X();"); });
	});

	TEST_CASE(L"A::B::X(){}")
	{
		runParser.operator()<CppFile>(L"File", L"AmbiguousDecl2", [&]() { return GetCppParser().Parse_File(L"A::B::X(){}"); });
	});

	TEST_CASE(L"namespace {A::B::X(){}}")
	{
		runParser.operator()<CppFile>(L"File", L"AmbiguousDecl3", [&]() { return GetCppParser().Parse_File(L"namespace {A::B::X(){}}"); });
	});

	TEST_CASE(L"int main() {A X();}")
	{
		runParser.operator()<CppFile>(L"File", L"AmbiguousStat", [&]() { return GetCppParser().Parse_File(L"int main() {A X();}"); });
	});

	TEST_CASE(L"template<typename T = X()> struct S{};")
	{
		runParser.operator()<CppFile>(L"File", L"AmbiguousGenericParameter", [&]() { return GetCppParser().Parse_File(L"template<typename T = X()> struct S{};"); });
	});

	TEST_CASE(L"struct S<X()>;")
	{
		runParser.operator()<CppFile>(L"File", L"AmbiguousGenericArgument", [&]() { return GetCppParser().Parse_File(L"struct S<X()>;"); });
	});

	GetCppParser().OnError.Remove(handlerOnError);
	GetCppParser().OnTraceProcessing.Remove(handlerOnTraceProcessing);
	GetCppParser().OnReadyToExecute.Remove(handlerOnReadyToExecute);
}