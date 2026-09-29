#include <VlppOS.h>
#if defined VCZH_MSVC
#define _WINSOCKAPI_
#include <Windows.h>
#endif

using namespace vl;
using namespace vl::console;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;

#if defined VCZH_MSVC
WString GetExePath()
{
	wchar_t buffer[65536];
	GetModuleFileName(NULL, buffer, sizeof(buffer) / sizeof(*buffer));
	vint pos = -1;
	vint index = 0;
	while (buffer[index])
	{
		if (buffer[index] == L'\\' || buffer[index] == L'/')
		{
			pos = index;
		}
		index++;
	}
	return WString::CopyFrom(buffer, pos + 1);
}
#endif

WString GetSourcePath()
{
#if defined VCZH_GCC
	return L"../../../Source/";
#elif defined VCZH_WASM
	return L"/Source/";
#elif defined VCZH_MSVC && defined _WIN64
	return GetExePath() + L"../../../../Source/";
#elif defined VCZH_MSVC
	return GetExePath() + L"../../../Source/";
#endif
}

WString GetTestParserInputPath(const WString& parserName)
{
#if defined VCZH_GCC
	return L"../../Source/" + parserName + L"/";
#elif defined VCZH_WASM
	return L"/Test/Source/" + parserName + L"/";
#elif defined VCZH_MSVC && defined _WIN64
	return GetExePath() + L"../../../Source/" + parserName + L"/";
#elif defined VCZH_MSVC
	return GetExePath() + L"../../Source/" + parserName + L"/";
#endif
}

WString GetTestOutputPath()
{
#if defined VCZH_GCC
	return L"../../ParserLog/";
#elif defined VCZH_WASM
	return L"/Test/ParserLog/";
#elif defined VCZH_MSVC && defined _WIN64
	return GetExePath() + L"../../../ParserLog/";
#elif defined VCZH_MSVC
	return GetExePath() + L"../../ParserLog/";
#endif
}

WString GetParserGenGeneratedOutputPath()
{
	return GetTestOutputPath() + L"../../Source/ParserGen_Generated/";
}

FilePath GetOutputDir(const WString& parserName)
{
	auto outputDir = FilePath(GetTestOutputPath()) / parserName;
	{
		Folder folder = outputDir;
		if (!folder.Exists())
		{
			folder.Create(true);
		}
	}
	return outputDir;
}

void WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files)
{
#if defined VCZH_MSVC
	for (auto [key, index] : indexed(files.Keys()))
	{
		File outputFile = outputDir / key;
		auto content = files.Values()[index];
		if (outputFile.Exists())
		{
			auto existing = outputFile.ReadAllTextByBom();
			if (content == existing)
			{
				continue;
			}
		}
		outputFile.WriteAllText(content, false, BomEncoder::Utf8);
	}
#elif defined VCZH_GCC
	Console::WriteLine(L"**** Skipped updating C++ files in Linux ****");
#endif
}

TEST_FILE
{
	TEST_CASE_ASSERT(Folder(GetTestOutputPath()).Exists());
}

using TExecutor = void(*)(void);

TExecutor runBeforeTests = nullptr;
TExecutor runAfterTests = nullptr;

void SetRunBeforeTests(TExecutor value)
{
	runBeforeTests = value;
}

void SetRunAfterTests(TExecutor value)
{
	runAfterTests = value;
}

template<typename T>
int UnitTestMain(int argc, T* argv[])
{
	{
		Folder folder(GetTestOutputPath());
		if (!folder.Exists())
		{
			folder.Create(true);
		}
	}
	if (runBeforeTests) runBeforeTests();
	int result = vl::unittest::UnitTest::RunAndDisposeTests(argc, argv);
	if (runAfterTests) runAfterTests();
	FinalizeGlobalStorage();
	vl::unittest::UnitTest::DumpMemoryLeak(argc, argv);
	return result;
}

#if defined VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
{
	return UnitTestMain(argc, argv);
}
#elif defined VCZH_GCC
int main(int argc, char* argv[])
{
	return UnitTestMain(argc, argv);
}
#endif

#if defined VCZH_WASM
#include <emscripten.h>
#include <emscripten/bind.h>

EM_JS(int, WasmReportFailure, (const char16_t* text, vint length), {
	return globalThis["vlConsoleFailure"](HEAPU16, text, length);
});

vint WasmMain()
{
	wchar_t name[] = L"UnitTest";
	wchar_t mode[] = L"/D";
	wchar_t* arguments[] = { name, mode };
	return UnitTestMain(2, arguments);
}

vint wasm_main()
{
	try
	{
		WString message;
		try
		{
			return WasmMain();
		}
		catch (const vl::unittest::UnitTestAssertError& error)
		{
			message = error.message;
		}
		catch (const vl::unittest::UnitTestConfigError& error)
		{
			message = error.message;
		}
		catch (const vl::unittest::UnitTestJustCrashError&)
		{
			message = L"The unit test framework stopped after a failure.";
		}
		catch (const Error& error)
		{
			message = error.Description();
		}
		catch (const Exception& error)
		{
			message = error.Message();
		}
		catch (const std::exception& error)
		{
			message = atow(error.what());
		}
		catch (...)
		{
			message = L"Unknown C++ exception.";
		}
		auto text = wtou16(message);
		WasmReportFailure(text.Buffer(), text.Length());
	}
	catch (...)
	{
		// Diagnostics can allocate too; no C++ exception may cross this boundary.
		constexpr char16_t text[] = u"Unable to format the C++ failure diagnostic.";
		WasmReportFailure(text, sizeof(text) / sizeof(*text) - 1);
	}
	return 1;
}

EMSCRIPTEN_BINDINGS(CppApplication)
{
	emscripten::function("wasm_main", &wasm_main);
}
#endif
