#include "AstCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;

/***********************************************************************
WriteTraverseVisitorHeaderFile
***********************************************************************/

			void WriteTraverseVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorHeaderFile(file, L"Traverse", writer, [&](const WString& prefix)
				{
					for (auto name : file->SymbolOrder())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(file->Symbols()[name]))
						{
							if (classSymbol->derivedClasses.Count() > 0)
							{
							}
						}
					}
				});
			}

/***********************************************************************
WriteTraverseVisitorCppFile
***********************************************************************/

			void WriteTraverseVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorCppFile(file, L"Traverse", writer, [&](const WString& prefix)
				{
					for (auto name : file->SymbolOrder())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(file->Symbols()[name]))
						{
							if (classSymbol->derivedClasses.Count() > 0)
							{
							}
						}
					}
				});
			}

/***********************************************************************
WriteRootTraverseVisitorHeaderFile
***********************************************************************/

			void WriteRootTraverseVisitorHeaderFile(AstSymbolManager& manager, stream::StreamWriter& writer)
			{
				WriteRootVisitorHeaderFile(manager, L"Traverse", writer, [&](const WString& prefix)
				{
				});
			}

/***********************************************************************
WriteRootTraverseVisitorCppFile
***********************************************************************/

			void WriteRootTraverseVisitorCppFile(AstSymbolManager& manager, stream::StreamWriter& writer)
			{
				WriteRootVisitorCppFile(manager, L"Traverse", writer, [&](const WString& prefix)
				{
				});
			}
		}
	}
}