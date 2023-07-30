#include "AstToCode.h"

namespace vl::glr::parsergen
{
	/***********************************************************************
	TypeAstToCode
	***********************************************************************/
	
	Ptr<GlrAstFile> TypeSymbolToAst(
		const AstSymbolManager& manager,
		bool createGeneratedTypes
	)
	{
		auto generated = Ptr(new GlrAstFile);
		for (auto groupName : manager.FileGroupOrder())
		{
			auto group = manager.FileGroups()[groupName];
			for (auto typeName : group->SymbolOrder())
			{
				auto symbol = group->Symbols()[typeName];

				if (auto enumSymbol = dynamic_cast<AstEnumSymbol*>(symbol))
				{
					auto enumType = Ptr(new GlrEnum);
					generated->types.Add(enumType);

					if (enumSymbol->isPublic) enumType->attPublic.value = L"@public";
					enumType->name.value = enumSymbol->Name();

					for (auto itemName : enumSymbol->ItemOrder())
					{
						auto itemSymbol = enumSymbol->Items()[itemName];

						auto enumItem = Ptr(new GlrEnumItem);
						enumType->items.Add(enumItem);

						enumItem->name.value = itemSymbol->Name();
					}
				}

				if (auto classSymbol = dynamic_cast<AstClassSymbol*>(symbol))
				{
					if (!createGeneratedTypes && classSymbol->classType != AstClassType::Defined)
					{
						continue;
					}

					auto classType = Ptr(new GlrClass);
					generated->types.Add(classType);

					if (classSymbol->isPublic) classType->attPublic.value = L"@public";
					if (classSymbol->derivedClass_ToResolve) classType->attAmbiguous.value = L"@ambiguous";
					classType->name.value = classSymbol->Name();

					if (classSymbol->baseClass)
					{
						if (!createGeneratedTypes && classSymbol->baseClass->classType == AstClassType::Generated_Common)
						{
							classType->baseClass.value = classSymbol->baseClass->baseClass->Name();
						}
						else
						{
							classType->baseClass.value = classSymbol->baseClass->Name();
						}
					}

					if (!createGeneratedTypes && classSymbol->derivedClass_Common)
					{
						classSymbol = classSymbol->derivedClass_Common;
					}
					for (auto propName : classSymbol->PropOrder())
					{
						auto propSymbol = classSymbol->Props()[propName];

						auto classProp = Ptr(new GlrClassProp);
						classType->props.Add(classProp);

						classProp->name.value = propSymbol->Name();
						switch (propSymbol->propType)
						{
						case AstPropType::Token:
							classProp->propType = GlrPropType::Token;
							break;
						case AstPropType::Type:
							classProp->propType = GlrPropType::Type;
							break;
						case AstPropType::Array:
							classProp->propType = GlrPropType::Array;
							break;
						}
						if (propSymbol->propSymbol) classProp->propTypeName.value = propSymbol->propSymbol->Name();
					}
				}
			}
		}
		return generated;
	}
}