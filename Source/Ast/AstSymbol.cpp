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
					ownerFile->Owner()->Global().AddError(
						ParserErrorType::DuplicatedEnumItem,
						ownerFile->Name(),
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

				auto& symbols = parent->Owner()->Symbols();
				vint index = symbols.Keys().IndexOf(typeName);
				if (index == -1)
				{
					ownerFile->Owner()->Global().AddError(
						ParserErrorType::FieldTypeNotExists,
						ownerFile->Name(),
						parent->Name(),
						name
						);
					return false;
				}

				propSymbol = symbols.Values()[index];
				if (_type == AstPropType::Type) return true;

				if (!dynamic_cast<AstClassSymbol*>(propSymbol))
				{
					parent->Owner()->Owner()->Global().AddError(
						ParserErrorType::FieldTypeNotClass,
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
				auto& symbols = ownerFile->Symbols();
				vint index = symbols.Keys().IndexOf(typeName);
				if (index == -1)
				{
					ownerFile->Owner()->Global().AddError(ParserErrorType::BaseClassNotExists, ownerFile->Name(), name, typeName);
					return false;
				}

				auto newBaseClass = dynamic_cast<AstClassSymbol*>(symbols.Values()[index]);
				if (!newBaseClass)
				{
					ownerFile->Owner()->Global().AddError(ParserErrorType::BaseClassNotClass, ownerFile->Name(), name, typeName);
					return false;
				}

				List<AstClassSymbol*> visited;
				visited.Add(newBaseClass);
				for (vint i = 0; i < visited.Count(); i++)
				{
					auto currentSymbol = visited[i];
					if (currentSymbol == this)
					{
						ownerFile->Owner()->Global().AddError(
							ParserErrorType::BaseClassCyclicDependency,
							ownerFile->Name(),
							name
						);
						return false;
					}

					if (currentSymbol->baseClass)
					{
						visited.Add(currentSymbol->baseClass);
					}
				}

				baseClass = newBaseClass;
				newBaseClass->derivedClasses.Add(this);
				return true;
			}

			AstClassSymbol* AstClassSymbol::CreateAmbiguousDerivedClass()
			{
				if (!ambiguousDerivedClass)
				{
					auto derived = ownerFile->CreateClass(name + L"ToResolve");
					derived->baseClass = this;

					auto prop = derived->CreateProp(L"candidates");
					prop->propType = AstPropType::Array;
					prop->propSymbol = this;

					ambiguousDerivedClass = derived;
				}
				return ambiguousDerivedClass;
			}

			AstClassPropSymbol* AstClassSymbol::CreateProp(const WString& propName)
			{
				auto symbol = new AstClassPropSymbol(this, propName);
				if (!props.Add(propName, symbol))
				{
					ownerFile->Owner()->Global().AddError(
						ParserErrorType::DuplicatedClassProp,
						ownerFile->Name(),
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
					ownerManager->Global().AddError(
						ParserErrorType::DuplicatedSymbol,
						name,
						symbolName
						);
				}
				else if (!ownerManager->symbolMap.Keys().Contains(symbolName))
				{
					ownerManager->symbolMap.Add(symbolName, symbol);
				}
				else
				{
					ownerManager->Global().AddError(
						ParserErrorType::DuplicatedSymbolGlobally,
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
					ownerManager->Global().AddError(
						ParserErrorType::FileDependencyNotExists,
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
						ownerManager->Global().AddError(
							ParserErrorType::FileCyclicDependency,
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

			AstSymbolManager::AstSymbolManager(ParserSymbolManager& _global)
				: global(_global)
			{
			}

			AstDefFile* AstSymbolManager::CreateFile(const WString& name)
			{
				auto file = new AstDefFile(this, name);
				if (!files.Add(name, file))
				{
					global.AddError(
						ParserErrorType::DuplicatedFile,
						name
						);
				}
				return file;
			}
		}
	}
}