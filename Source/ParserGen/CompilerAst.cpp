#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{

/***********************************************************************
CompileAst
***********************************************************************/

			class CompileAstVisitor :public Object, public virtual GlrType::IVisitor
			{
			protected:
				AstDefFile*				astDefFile;
			public:
				CompileAstVisitor(AstDefFile* _astDefFile)
					: astDefFile(_astDefFile)
				{
				}

				void Visit(GlrEnum* node) override
				{
					auto enumSymbol = astDefFile->CreateEnum(node->name.value);
					for (auto item : node->items)
					{
						enumSymbol->CreateItem(item->name.value);
					}
				}

				void Visit(GlrClass* node) override
				{
					auto classSymbol = astDefFile->CreateClass(node->name.value);
					if (node->baseClass)
					{
						classSymbol->SetBaseClass(node->baseClass.value);
					}
					for (auto prop : node->props)
					{
						auto propSymbol = classSymbol->CreateProp(prop->name.value);
						switch (prop->propType)
						{
						case GlrPropType::Token:
							propSymbol->SetPropType(AstPropType::Token);
							break;
						case GlrPropType::Type:
							propSymbol->SetPropType(AstPropType::Type, prop->propTypeName.value);
							break;
						case GlrPropType::Array:
							propSymbol->SetPropType(AstPropType::Array, prop->propTypeName.value);
							break;
						}
					}
				}
			};

			void CompileAst(AstSymbolManager& astManager, AstDefFile* astDefFile, Ptr<GlrAstFile> file)
			{
				CompileAstVisitor visitor(astDefFile);
				for (auto type : file->types)
				{
					type->Accept(&visitor);
				}
			}
		}
	}
}