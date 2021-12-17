#include "../../Source/LogTrace.h"

extern WString GetSourcePath();
extern FilePath GetOutputDir(const WString& parserName);

TEST_FILE
{
	FilePath dirWorkflow = FilePath(GetSourcePath()) / L"../../Workflow/Test/Resources";
	List<File> indexFiles;
	Folder(dirWorkflow).GetFiles(indexFiles);

	for (auto indexFile : indexFiles)
	{
		WString indexName = indexFile.GetFilePath().GetName();
		indexName = indexName.Sub(5, indexName.Length() - 9);

		TEST_CATEGORY(L"Test Workflow on Index: " + indexName)
		{
			List<WString> caseNames;
			indexFile.ReadAllLinesByBom(caseNames);
			for (auto caseName : caseNames)
			{
				{
					vint eq = caseName.IndexOf(L'=');
					if (eq != -1)
					{
						caseName = caseName.Left(eq);
					}
				}

				TEST_CASE(caseName)
				{
				});
			}
		});
	}
}