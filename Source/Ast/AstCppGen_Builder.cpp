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
WriteAstBuilderHeaderFile
***********************************************************************/

			void WriteAstBuilderHeaderFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteUtilityHeaderFile(file, L"BUILDER", L"builder", writer, [&](const WString& prefix)
				{
					for(auto typeSymbol : file->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							if (classSymbol->Props().Count() > 0)
							{
								WString className = classSymbol->Name() + L"Builder";
								writer.WriteLine(prefix + L"class " + className);
								writer.WriteLine(prefix + L"{");
								writer.WriteLine(prefix + L"private:");
								writer.WriteString(prefix + L"\t");
								PrintCppType(file, classSymbol, writer);
								writer.WriteLine(L"* node;");
								writer.WriteLine(prefix + L"public:");
								writer.WriteString(prefix + L"\t" + classSymbol->Name() + L"Builder(");
								PrintCppType(file, classSymbol, writer);
								writer.WriteLine(L"* _node) : node(_node) {}");
								writer.WriteLine(L"");
								for (auto propSymbol : classSymbol->Props().Values())
								{
									switch (propSymbol->propType)
									{
									case AstPropType::Token:
										writer.WriteLine(prefix + L"\t" + className + L"& " + propSymbol->Name() + L"(const vl::WString& value);");
										break;
									case AstPropType::Type:
										if (dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
										{
											writer.WriteString(prefix + L"\t" + className + L"& " + propSymbol->Name() + L"(");
											PrintCppType(file, propSymbol->propSymbol, writer);
											writer.WriteLine(L" value);");
											break;
										}
									case AstPropType::Array:
										writer.WriteString(prefix + L"\t" + className + L"& " + propSymbol->Name() + L"(const vl::Ptr<");
										PrintCppType(file, propSymbol->propSymbol, writer);
										writer.WriteLine(L">& value);");
										break;
									}
								}
								writer.WriteLine(prefix + L"};");
								writer.WriteLine(L"");
							}
						}
					}
				});
			}

/***********************************************************************
WriteAstBuilderCppFile
***********************************************************************/

			void WriteAstBuilderCppFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteUtilityCppFile(file, L"Builder", L"builder", writer, [&](const WString& prefix)
				{
					for (auto typeSymbol : file->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							if (classSymbol->Props().Count() > 0)
							{
								WString className = classSymbol->Name() + L"Builder";
								writer.WriteLine(L"");
								writer.WriteLine(L"/***********************************************************************");
								writer.WriteLine(className);
								writer.WriteLine(L"***********************************************************************/");

								for (auto propSymbol : classSymbol->Props().Values())
								{
									writer.WriteLine(L"");
									switch (propSymbol->propType)
									{
									case AstPropType::Token:
										writer.WriteLine(prefix + className + L"& " + className + L"::" + propSymbol->Name() + L"(const vl::WString& value)");
										writer.WriteLine(prefix + L"{");
										writer.WriteLine(prefix + L"\tnode->" + propSymbol->Name() + L".value = value;");
										writer.WriteLine(prefix + L"\treturn *this;");
										writer.WriteLine(prefix + L"}");
										break;
									case AstPropType::Type:
										if (dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
										{
											writer.WriteString(prefix + className + L"& " + className + L"::" + propSymbol->Name() + L"(");
											PrintCppType(file, propSymbol->propSymbol, writer);
											writer.WriteLine(L" value)");
										}
										if (dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
										{
											writer.WriteString(prefix + className + L"& " + className + L"::" + propSymbol->Name() + L"(const vl::Ptr<");
											PrintCppType(file, propSymbol->propSymbol, writer);
											writer.WriteLine(L">& value)");
										}
										writer.WriteLine(prefix + L"{");
										writer.WriteLine(prefix + L"\tnode->" + propSymbol->Name() + L" = value;");
										writer.WriteLine(prefix + L"\treturn *this;");
										writer.WriteLine(prefix + L"}");
										break;
									case AstPropType::Array:
										writer.WriteString(prefix + className + L"& " + className + L"::" + propSymbol->Name() + L"(const vl::Ptr<");
										PrintCppType(file, propSymbol->propSymbol, writer);
										writer.WriteLine(L">& value)");
										writer.WriteLine(prefix + L"{");
										writer.WriteLine(prefix + L"\tnode->" + propSymbol->Name() + L".Add(value);");
										writer.WriteLine(prefix + L"\treturn *this;");
										writer.WriteLine(prefix + L"}");
										break;
									}
								}
							}
						}
					}
				});
			}
		}
	}
}