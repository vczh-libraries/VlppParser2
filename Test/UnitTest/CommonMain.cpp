#include <VlppOS.h>
#include <Windows.h>

using namespace vl;
using namespace vl::filesystem;

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

WString GetTestParserInputPath(const WString& parserName)
{
#ifdef _WIN64
	return GetExePath() + L"../../../Source/" + parserName + L"/";
#else
	return GetExePath() + L"../../Source/" + parserName + L"/";
#endif
}

WString GetTestOutputPath()
{
#ifdef _WIN64
	return GetExePath() + L"../../../Output/";
#else
	return GetExePath() + L"../../Output/";
#endif
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

TEST_FILE
{
	TEST_CASE_ASSERT(Folder(GetTestOutputPath()).Exists());
}

int wmain(int argc, wchar_t* argv[])
{
	{
		Folder folder(GetTestOutputPath());
		if (!folder.Exists())
		{
			folder.Create(false);
		}
	}
	int result = unittest::UnitTest::RunAndDisposeTests(argc, argv);
	FinalizeGlobalStorage();
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}