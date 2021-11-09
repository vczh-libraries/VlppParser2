#include "AstSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
CreateParserGenAst
***********************************************************************/

			AstDefFile* CreateParserGenTypeAst(AstSymbolManager& manager)
			{
				auto _ast = manager.CreateFile(L"TypeAst");
				Fill(_ast->includes, L"../AstBase.h");
				Fill(_ast->cppNss, L"vl", L"glr", L"parsergen");
				Fill(_ast->refNss, L"glr", L"parsergen");
				_ast->classPrefix = L"Glr";
				_ast->headerGuard = L"VCZH_PARSER2_PARSERGEN_TYPEAST";

				auto _type = _ast->CreateClass(L"Type");
				_type->CreateProp(L"name")->SetPropType(AstPropType::Token);

				auto _enumItem = _ast->CreateClass(L"EnumItem");
				_enumItem->CreateProp(L"name")->SetPropType(AstPropType::Token);

				auto _enum = _ast->CreateClass(L"Enum");
				_enum->SetBaseClass(L"Type");
				_enum->CreateProp(L"items")->SetPropType(AstPropType::Array, L"EnumItem");

				auto _propType = _ast->CreateEnum(L"PropType");
				_propType->CreateItem(L"Token");
				_propType->CreateItem(L"Type");
				_propType->CreateItem(L"Array");

				auto _classProp = _ast->CreateClass(L"ClassProp");
				_classProp->CreateProp(L"name")->SetPropType(AstPropType::Token);
				_classProp->CreateProp(L"propType")->SetPropType(AstPropType::Type, L"PropType");
				_classProp->CreateProp(L"propTypeName")->SetPropType(AstPropType::Token);

				auto _class = _ast->CreateClass(L"Class");
				_class->SetBaseClass(L"Type");
				_class->CreateProp(L"props")->SetPropType(AstPropType::Array, L"ClassProp");

				auto _file = _ast->CreateClass(L"File");
				_file->CreateProp(L"name")->SetPropType(AstPropType::Token);
				_file->CreateProp(L"types")->SetPropType(AstPropType::Array, L"Type");

				return _ast;
			}
		}
	}
}