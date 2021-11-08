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
				symbol->value = items.items.Count();
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

			bool AstClassPropSymbol::SetPropType(AstPropType _type, const WString& typeName)
			{
				propType = _type;
				if (_type == AstPropType::Token) return true;

				auto& symbols = parent->Owner()->Owner()->Symbols();
				vint index = symbols.Keys().IndexOf(typeName);
				if (index == -1)
				{
					ownerFile->Owner()->AddError(AstErrorType::FieldTypeNotExists, ownerFile->Name(), name);
					return false;
				}

				propSymbol = symbols.Values()[index];
				if (_type == AstPropType::Type) return true;

				if (!dynamic_cast<AstClassSymbol*>(propSymbol))
				{
					parent->Owner()->Owner()->AddError(
						AstErrorType::FieldTypeNotClass,
						parent->Owner()->Name(),
						parent->Name(),
						name
						);
					return false;
				}
				return true;
			}

/***********************************************************************
AstClassSymbol
***********************************************************************/

			AstClassSymbol::AstClassSymbol(AstDefFile* _file, const WString& _name)
				: AstSymbol(_file, _name)
			{
			}

			bool AstClassSymbol::SetBaseClass(const WString& typeName)
			{
				auto& symbols = ownerFile->Owner()->Symbols();
				vint index = symbols.Keys().IndexOf(typeName);
				if (index == -1)
				{
					ownerFile->Owner()->AddError(AstErrorType::BaseClassNotExists, ownerFile->Name(), name);
					return false;
				}

				baseClass = dynamic_cast<AstClassSymbol*>(symbols.Values()[index]);
				if (!baseClass)
				{
					ownerFile->Owner()->AddError(AstErrorType::BaseClassNotClass, ownerFile->Name(), name);
					return false;
				}

				baseClass->derivedClasses.Add(this);
				return true;
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

			bool AstDefFile::AddDependency(const WString& dependency)
			{
				if (dependencies.Contains(dependency)) return true;
				if (!ownerManager->Files().Keys().Contains(dependency))
				{
					ownerManager->AddError(
						AstErrorType::FileDependencyNotExists,
						name,
						dependency
						);
					return false;
				}

				List<WString> visited;
				visited.Add(dependency);
				for (vint i = 0; i < visited.Count(); i++)
				{
					auto currentName = visited[i];
					if (currentName == name)
					{
						ownerManager->AddError(
							AstErrorType::FileCyclicDependency,
							name,
							dependency
							);
						return false;
					}
					auto current = ownerManager->Files()[currentName];
					for (vint j = 0; j < current->dependencies.Count(); j++)
					{
						auto dep = current->dependencies[j];
						if (!visited.Contains(dep))
						{
							visited.Add(dep);
						}
					}
				}

				dependencies.Add(dependency);
				return true;
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