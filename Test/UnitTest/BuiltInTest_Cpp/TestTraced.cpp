#include "TestCppHelper.h"

TEST_FILE
{
	WString indexName;
	WString caseName;
	FilePath dirOutput = GetOutputDir(L"BuiltIn-Cpp");

	auto handlerOnTraceProcessing = GetCppParser().OnTraceProcessing.Add(
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

	auto handlerOnReadyToExecute = GetCppParser().OnReadyToExecute.Add(
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

	//TEST_CASE(L"int*")
	//{
	//	runParser(L"TypeOrExpr", L"PointerOfInt", [&]() { return GetCppParser().Parse_TypeOrExpr(L"int*"); });
	//});
	//
	//TEST_CASE(L"Name<int>")
	//{
	//	runParser(L"TypeOrExpr", L"NameOfInt", [&]() { return GetCppParser().Parse_TypeOrExpr(L"Name<int>"); });
	//});
	//
	//TEST_CASE(L"Name<A...>")
	//{
	//	runParser(L"TypeOrExpr", L"NameOfAs", [&]() { return GetCppParser().Parse_TypeOrExpr(L"Name<A...>"); });
	//});
	//
	//TEST_CASE(L"sizeof a()")
	//{
	//	runParser(L"TypeOrExpr", L"SizeofA", [&]() { return GetCppParser().Parse_TypeOrExpr(L"sizeof a()"); });
	//});
	//
	//TEST_CASE(L"[]<typename T, class ...U = int>{}")
	//{
	//	runParser(L"TypeOrExpr", L"LambdaGeneric", [&]() { return GetCppParser().Parse_TypeOrExpr(L"[]<typename T, class ...U = int>{}"); });
	//});
	//
	//TEST_CASE(L"(T)(a)")
	//{
	//	runParser(L"TypeOrExpr", L"CallOrCast", [&]() { return GetCppParser().Parse_TypeOrExpr(L"(T)(a)"); });
	//});
	//
	//TEST_CASE(L"T*{a}")
	//{
	//	runParser(L"TypeOrExpr", L"MuliplyOrInit", [&]() { return GetCppParser().Parse_TypeOrExpr(L"T*{a}"); });
	//});

	TEST_CASE(L"A<B>::C")
	{
		runParser(L"TypeOrExpr", L"BExprOrQName", [&]() { return GetCppParser().Parse_TypeOrExpr(L"A<B>::C"); });
	});

	GetCppParser().OnTraceProcessing.Remove(handlerOnTraceProcessing);
	GetCppParser().OnReadyToExecute.Remove(handlerOnReadyToExecute);
}