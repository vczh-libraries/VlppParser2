#include "AstSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
AstDefFile
***********************************************************************/

			template<typename T>
			T* AstDefFile::CreateSymbol(const WString& symbolName)
			{
				auto symbol = new T();
				static_cast<AstSymbol*>(symbol)->ownerFile = this;
				static_cast<AstSymbol*>(symbol)->name = symbolName;
				symbols.Add(symbol);

				if (!symbolMap.Keys().Contains(symbolName))
				{
					symbolMap.Add(symbolName, symbol);
				}
				else
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
						ownerManager->symbolMap[symbolName]->ownerFile->name
						);
				}

				return symbol;
			}

			AstDefFile::AstDefFile(AstSymbolManager* _ownerManager, const WString& _name)
				:ownerManager(_ownerManager)
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
				files.Add(file);
				if (!fileMap.Keys().Contains(name))
				{
					fileMap.Add(name, file);
				}
				else
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