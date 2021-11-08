#include "AstSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
AstSymbol
***********************************************************************/

			AstSymbol::AstSymbol(AstDefFile* _file, const WString& _name)
				: ownerFile(_file)
				, name(_name)
			{
			}

/***********************************************************************
AstEnumItemSymbol
***********************************************************************/

			AstEnumItemSymbol::AstEnumItemSymbol(AstEnumSymbol* _parent, const WString& name)
				: AstSymbol(_parent->Owner(), name)
				, parent(_parent)
			{
			}

/***********************************************************************
AstEnumSymbol
***********************************************************************/

			AstEnumSymbol::AstEnumSymbol(AstDefFile* _file, const WString& _name)
				: AstSymbol(_file, _name)
			{
			}

			AstEnumItemSymbol* AstEnumSymbol::CreateItem(const WString& itemName)
			{
				auto symbol = new AstEnumItemSymbol(this, itemName);
				if (!items.Add(itemName, symbol))
				{
					ownerFile->Owner()->AddError(
						AstErrorType::DuplicatedEnumItem,
						name,
						itemName
						);
				}
				return symbol;
			}

/***********************************************************************
AstClassPropSymbol
***********************************************************************/

			AstClassPropSymbol::AstClassPropSymbol(AstClassSymbol* _parent, const WString& name)
				: AstSymbol(_parent->Owner(), name)
				, parent(_parent)
			{
			}

/***********************************************************************
AstClassSymbol
***********************************************************************/

			AstClassSymbol::AstClassSymbol(AstDefFile* _file, const WString& _name)
				: AstSymbol(_file, _name)
			{
			}

			AstClassPropSymbol* AstClassSymbol::CreateProp(const WString& propName)
			{
				auto symbol = new AstClassPropSymbol(this, propName);
				if (!props.Add(propName, symbol))
				{
					ownerFile->Owner()->AddError(
						AstErrorType::DuplicatedClassProp,
						name,
						propName
						);
				}
				return symbol;
			}

/***********************************************************************
AstDefFile
***********************************************************************/

			template<typename T>
			T* AstDefFile::CreateSymbol(const WString& symbolName)
			{
				auto symbol = new T(this, symbolName);
				if (!symbols.Add(symbolName, symbol))
				{
					ownerManager->AddError(
						AstErrorType::DuplicatedSymbol,
						name,
						symbolName
						);
				}

				if (!ownerManager->symbolMap.Keys().Contains(symbolName))
				{
					ownerManager->symbolMap.Add(symbolName, symbol);
				}
				else
				{
					ownerManager->AddError(
						AstErrorType::DuplicatedSymbolGlobally,
						name,
						symbolName,
						ownerManager->symbolMap[symbolName]->Owner()->name
						);
				}

				return symbol;
			}

			AstDefFile::AstDefFile(AstSymbolManager* _ownerManager, const WString& _name)
				: ownerManager(_ownerManager)
				, name(_name)
			{
			}

			AstEnumSymbol* AstDefFile::CreateEnum(const WString& symbolName)
			{
				return CreateSymbol<AstEnumSymbol>(symbolName);
			}

			AstClassSymbol* AstDefFile::CreateClass(const WString& symbolName)
			{
				return CreateSymbol<AstClassSymbol>(symbolName);
			}

/***********************************************************************
AstSymbolManager
***********************************************************************/

			template<typename ...TArgs>
			void AstSymbolManager::AddError(AstErrorType type, TArgs&& ...args)
			{
				AstError error;
				error.type = type;

				WString sargs[] = { WString(args)... };
				WString* dargs[] = { &error.arg1,&error.arg2,&error.arg3 };
				constexpr vint sl = sizeof(sargs) / sizeof(*sargs);
				constexpr vint dl = sizeof(dargs) / sizeof(*dargs);
				constexpr vint ml = sl < dl ? sl : dl;
				for (vint i = 0; i < ml; i++)
				{
					*dargs[i] = sargs[i];
				}

				errors.Add(std::move(error));
			}

			AstDefFile* AstSymbolManager::CreateFile(const WString& name)
			{
				auto file = new AstDefFile(this, name);
				if (!files.Add(name, file))
				{
					AddError(
						AstErrorType::DuplicatedFile,
						name
						);
				}
				return file;
			}
		}
	}
}