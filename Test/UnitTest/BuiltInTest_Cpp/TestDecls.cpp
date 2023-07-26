#include "TestCppHelper.h"

TEST_FILE
{
	const wchar_t* categories[] = {
		L"Decls",
		L"Members_Decls",
		L"Generic_Decls",
		// L"GenericMember_Decls",
		// L"GenericPS_Decls",
	};

	for (auto category : categories)
	{
		TEST_CATEGORY(WString::Unmanaged(category))
		{
			Folder caseFolder(
				FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
				/ L"Input"
				/ L"Declarations"
				/ category
				);
			TEST_CASE_ASSERT(caseFolder.Exists());
			List<File> caseFiles;
			TEST_CASE_ASSERT(caseFolder.GetFiles(caseFiles));

			for (auto caseFile : caseFiles)
			{
				TEST_DECL_PARSER(category, caseFile.GetFilePath().GetName().Buffer())
				{
					ParseFile<CppFile>(parser, lines);
				});
			}
		});
	}
}