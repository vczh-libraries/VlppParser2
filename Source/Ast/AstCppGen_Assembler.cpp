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
WriteAstAssemblerHeaderFile
***********************************************************************/

			void WriteAstAssemblerHeaderFile(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer)
			{
				WriteParserUtilityHeaderFile(manager, output, L"ASSEMBLER", writer, [&](const WString& prefix)
				{
					{
						vint index = 0;
						writer.WriteLine(prefix + L"enum class " + manager.name + L"Classes : vl::vint32_t");
						writer.WriteLine(prefix + L"{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								writer.WriteLine(prefix + L"\t" + classSymbol->Name() + L" = " + itow(index) + L",");
								index++;
							}
						}
						writer.WriteLine(prefix + L"};");
					}
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"enum class " + manager.name + L"Fields : vl::vint32_t");
						writer.WriteLine(prefix + L"{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									writer.WriteString(prefix + L"\t" + classSymbol->Name() + L"_" + propSymbol->Name());
									writer.WriteString(L" = ");
									writer.WriteString(L"(static_cast<vl::vint32_t>(" + manager.name + L"Classes::" + classSymbol->Name() + L") << 8) + " + itow(index));
									writer.WriteLine(L",");
								}
							}
						}
						writer.WriteLine(prefix + L"};");
					}
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"class " + manager.name + L"AstInsReceiver : public vl::glr::AstInsReceiverBase");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"protected:");
						writer.WriteLine(prefix + L"\tvl::Ptr<vl::glr::ParsingAstBase> CreateAstNode(vl::vint32_t type) override;");
						writer.WriteLine(prefix + L"\tvoid SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::Ptr<vl::glr::ParsingAstBase> value) override;");
						writer.WriteLine(prefix + L"\tvoid SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token) override;");
						writer.WriteLine(prefix + L"\tvl::Ptr<vl::glr::ParsingAstBase> ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates) override;");
						writer.WriteLine(prefix + L"};");
					}
				});
			}

/***********************************************************************
WriteAstAssemblerCppFile
***********************************************************************/

			void WriteAstAssemblerCppFile(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer)
			{
				WriteParserUtilityCppFile(manager, output->assemblyH, writer, [&](const WString& prefix)
				{
					writer.WriteLine(L"");
					writer.WriteLine(L"/***********************************************************************");
					writer.WriteLine(manager.name + L"AstInsReceiver : public vl::glr::AstInsReceiverBase");
					writer.WriteLine(L"***********************************************************************/");
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"vl::Ptr<vl::glr::ParsingAstBase> " + manager.name + L"AstInsReceiver::CreateAstNode(vl::vint32_t type)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tswitch((" + manager.name + L"Classes)type)");
						writer.WriteLine(prefix + L"\t{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								writer.WriteLine(prefix + L"\tcase " + manager.name + L"Classes::" + classSymbol->Name() + L":");
								if (classSymbol->derivedClasses.Count() > 0)
								{
									writer.WriteString(prefix + L"\t\tthrow vl::glr::AstInsException(L\"Unable to create abstract class \\\"");
									PrintCppType(nullptr, classSymbol, writer);
									writer.WriteLine(L"\\\".\", vl::glr::AstInsErrorType::UnknownType, type);");
								}
								else
								{
									writer.WriteString(prefix + L"\t\treturn new ");
									PrintCppType(nullptr, classSymbol, writer);
									writer.WriteLine(L"();");
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tthrow vl::glr::AstInsException(L\"The type id does not exist.\", vl::glr::AstInsErrorType::UnknownType, type);");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"}");
					}
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"void " + manager.name + L"AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::Ptr<vl::glr::ParsingAstBase> value)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tswitch((" + manager.name + L"Fields)field)");
						writer.WriteLine(prefix + L"\t{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									writer.WriteLine(prefix + L"\tcase " + manager.name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tthrow vl::glr::AstInsException(L\"The field id does not exist.\", vl::glr::AstInsErrorType::UnknownField, field);");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"}");
					}
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"void " + manager.name + L"AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tswitch((" + manager.name + L"Fields)field)");
						writer.WriteLine(prefix + L"\t{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									writer.WriteLine(prefix + L"\tcase " + manager.name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tthrow vl::glr::AstInsException(L\"The field id does not exist.\", vl::glr::AstInsErrorType::UnknownField, field);");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"}");
					}
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"vl::Ptr<vl::glr::ParsingAstBase> " + manager.name + L"AstInsReceiver::ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tswitch((" + manager.name + L"Classes)type)");
						writer.WriteLine(prefix + L"\t{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								writer.WriteLine(prefix + L"\tcase " + manager.name + L"Classes::" + classSymbol->Name() + L":");
							}
						}
						writer.WriteLine(prefix + L"\t\tthrow vl::glr::AstInsException(L\"The type is not configured to allow ambiguity.\", vl::glr::AstInsErrorType::UnsupportedAmbiguityType, type);");
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tthrow vl::glr::AstInsException(L\"The type id does not exist.\", vl::glr::AstInsErrorType::UnknownType, type);");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"}");
					}
				});
			}
		}
	}
}