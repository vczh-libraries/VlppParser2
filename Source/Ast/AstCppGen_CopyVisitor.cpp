#include "AstCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;

			extern void PrintCppType(AstDefFile* fileContext, AstSymbol* propSymbol, stream::StreamWriter& writer);

/***********************************************************************
WriteCopyVisitorHeaderFile
***********************************************************************/

			void WriteCopyVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorHeaderFile(file, L"Copy", writer, [&](const WString& prefix)
				{
					for (auto name : file->SymbolOrder())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(file->Symbols()[name]))
						{
							if (classSymbol->derivedClasses.Count() > 0)
							{
								writer.WriteLine(prefix + L"/// <summary>A copy visitor, overriding all abstract methods with AST copying code.</summary>");
								writer.WriteString(prefix + L"class " + name + L"Visitor : public virtual vl::glr::CopyVisitorBase, public ");
								PrintCppType(file, classSymbol, writer);
								writer.WriteLine(L"::IVisitor");
								writer.WriteLine(prefix + L"{");

								writer.WriteLine(prefix + L"protected:");
								for (auto childSymbol : classSymbol->derivedClasses)
								{
									if (childSymbol->derivedClasses.Count() > 0)
									{
										writer.WriteString(prefix + L"\tvirtual void Dispatch(");
										PrintCppType(file, childSymbol, writer);
										writer.WriteLine(L"* node) = 0;");
									}
								}
								writer.WriteLine(L"");

								writer.WriteLine(prefix + L"public:");
								writer.WriteLine(prefix + L"\t// Visitor Members -----------------------------------");
								for (auto childSymbol : classSymbol->derivedClasses)
								{
									writer.WriteString(prefix + L"\tvoid Visit(");
									PrintCppType(file, childSymbol, writer);
									writer.WriteLine(L"* node) override;");
								}

								writer.WriteLine(prefix + L"};");
								writer.WriteLine(L"");
							}
						}
					}
				});
			}

/***********************************************************************
WriteCopyVisitorCppFile
***********************************************************************/

			void WriteCopyVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorCppFile(file, L"Copy", writer, [&](const WString& prefix)
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
		}
	}
}