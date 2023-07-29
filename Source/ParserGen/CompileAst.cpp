#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
CompileAst
***********************************************************************/

			class CreateAstSymbolVisitor :public Object, public virtual GlrType::IVisitor
			{
			protected:
				AstDefFile*				astDefFile;
			public:
				CreateAstSymbolVisitor(AstDefFile* _astDefFile)
					: astDefFile(_astDefFile)
				{
				}

				void Visit(GlrEnum* node) override
				{
					auto symbol = astDefFile->CreateEnum(node->name.value, node->name.codeRange);
					symbol->isPublic = node->attPublic;
				}

				void Visit(GlrClass* node) override
				{
					auto symbol = astDefFile->CreateClass(node->name.value, node->name.codeRange);
					symbol->isPublic = node->attPublic;

					if (node->attAmbiguous)
					{
						symbol->CreateDerivedClass_ToResolve(node->name.codeRange);
						if (node->props.Count() > 0)
						{
							symbol->CreateDerivedClass_Common(node->name.codeRange);
						}
					}
				}
			};

			class FillAstSymbolVisitor :public Object, public virtual GlrType::IVisitor
			{
			protected:
				AstDefFile*				astDefFile;
			public:
				FillAstSymbolVisitor(AstDefFile* _astDefFile)
					: astDefFile(_astDefFile)
				{
				}

				void Visit(GlrEnum* node) override
				{
					auto enumSymbol = dynamic_cast<AstEnumSymbol*>(astDefFile->Symbols()[node->name.value]);
					for (auto item : node->items)
					{
						enumSymbol->CreateItem(item->name.value, item->name.codeRange);
					}
				}

				void FillClassSymbolBaseClass(GlrClass* node, AstClassSymbol* classSymbol)
				{
					if (node->baseClass)
					{
						classSymbol->SetBaseClass(node->baseClass.value, node->baseClass.codeRange);
					}
				}

				void FillClassSymbolProps(GlrClass* node, AstClassSymbol* classSymbol)
				{
					for (auto prop : node->props)
					{
						auto propSymbol = classSymbol->CreateProp(prop->name.value, prop->name.codeRange);
						switch (prop->propType)
						{
						case GlrPropType::Token:
							propSymbol->SetPropType(AstPropType::Token);
							break;
						case GlrPropType::Type:
							propSymbol->SetPropType(AstPropType::Type, prop->propTypeName.value, prop->propTypeName.codeRange);
							break;
						case GlrPropType::Array:
							propSymbol->SetPropType(AstPropType::Array, prop->propTypeName.value, prop->propTypeName.codeRange);
							break;
						default:;
						}
					}
				}

				void Visit(GlrClass* node) override
				{
					auto classSymbol = dynamic_cast<AstClassSymbol*>(astDefFile->Symbols()[node->name.value]);
					FillClassSymbolBaseClass(node, classSymbol);

					if (node->attAmbiguous && classSymbol->derivedClass_Common)
					{
						classSymbol = classSymbol->derivedClass_Common;
					}
					FillClassSymbolProps(node, classSymbol);
				}
			};

			void CompileAst(AstSymbolManager& astManager, collections::List<collections::Pair<AstDefFile*, Ptr<GlrAstFile>>>& files)
			{
				for (auto [astDefFile, file] : files)
				{
					CreateAstSymbolVisitor visitor(astDefFile);
					for (auto type : file->types)
					{
						type->Accept(&visitor);
					}
				}
				if (astManager.Global().Errors().Count() == 0)
				{
					for (auto [astDefFile, file] : files)
					{
						FillAstSymbolVisitor visitor(astDefFile);
						for (auto type : file->types)
						{
							type->Accept(&visitor);
						}
					}
				}
			}

			void CompileAst(AstSymbolManager& astManager, AstDefFile* astDefFile, Ptr<GlrAstFile> file)
			{
				List<Pair<AstDefFile*, Ptr<GlrAstFile>>> files;
				files.Add({ astDefFile,file });
				CompileAst(astManager, files);
			}
		}
	}
}