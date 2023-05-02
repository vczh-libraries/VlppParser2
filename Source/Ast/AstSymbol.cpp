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

			AstEnumItemSymbol* AstEnumSymbol::CreateItem(const WString& itemName, ParsingTextRange codeRange)
			{
				auto symbol = new AstEnumItemSymbol(this, itemName);
				symbol->value = items.map.Count();
				if (!items.Add(itemName, symbol))
				{
					ownerFile->AddError(
						ParserErrorType::DuplicatedEnumItem,
						codeRange,
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

			bool AstClassPropSymbol::SetPropType(AstPropType _type, const WString& typeName, ParsingTextRange codeRange)
			{
				propType = _type;
				if (_type == AstPropType::Token) return true;

				auto& symbols = parent->Owner()->Symbols();
				vint index = symbols.Keys().IndexOf(typeName);
				if (index == -1)
				{
					ownerFile->AddError(
						ParserErrorType::FieldTypeNotExists,
						codeRange,
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
					ownerFile->AddError(
						ParserErrorType::FieldTypeNotClass,
						codeRange,
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

			bool AstClassSymbol::SetBaseClass(const WString& typeName, ParsingTextRange codeRange)
			{
				auto& symbols = ownerFile->Symbols();
				vint index = symbols.Keys().IndexOf(typeName);
				if (index == -1)
				{
					ownerFile->AddError(
						ParserErrorType::BaseClassNotExists,
						codeRange,
						ownerFile->Name(),
						name,
						typeName
						);
					return false;
				}

				auto newBaseClass = dynamic_cast<AstClassSymbol*>(symbols.Values()[index]);
				if (!newBaseClass)
				{
					ownerFile->AddError(
						ParserErrorType::BaseClassNotClass,
						codeRange,
						ownerFile->Name(),
						name,
						typeName
						);
					return false;
				}

				List<AstClassSymbol*> visited;
				visited.Add(newBaseClass);
				// TODO: (enumerable) foreach:alterable
				for (vint i = 0; i < visited.Count(); i++)
				{
					auto currentSymbol = visited[i];
					if (currentSymbol == this)
					{
						ownerFile->AddError(
							ParserErrorType::BaseClassCyclicDependency,
							codeRange,
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

			AstClassSymbol* AstClassSymbol::CreateAmbiguousDerivedClass(ParsingTextRange codeRange)
			{
				if (!ambiguousDerivedClass)
				{
					auto derived = ownerFile->CreateClass(name + L"ToResolve", codeRange);
					derived->baseClass = this;
					derivedClasses.Add(derived);

					auto prop = derived->CreateProp(L"candidates", codeRange);
					prop->propType = AstPropType::Array;
					prop->propSymbol = this;

					ambiguousDerivedClass = derived;
				}
				return ambiguousDerivedClass;
			}

			AstClassPropSymbol* AstClassSymbol::CreateProp(const WString& propName, ParsingTextRange codeRange)
			{
				auto symbol = new AstClassPropSymbol(this, propName);
				if (!props.Add(propName, symbol))
				{
					ownerFile->AddError(
						ParserErrorType::DuplicatedClassProp,
						codeRange,
						ownerFile->Name(),
						name,
						propName
						);
				}
				return symbol;
			}

/***********************************************************************
FindCommonBaseClass
***********************************************************************/

			AstClassSymbol* FindCommonBaseClass(AstClassSymbol* c1, AstClassSymbol* c2)
			{
				if (c1 == c2) return c1;
				if (!c1) return c2;
				if (!c2) return c1;

				// find common base classes
				vint n1 = 0, n2 = 0;
				{
					auto c = c1;
					while (c)
					{
						n1++;
						c = c->baseClass;
					}
				}
				{
					auto c = c2;
					while (c)
					{
						n2++;
						c = c->baseClass;
					}
				}

				while (n1 > n2)
				{
					n1--;
					c1 = c1->baseClass;
				}
				while (n2 > n1)
				{
					n2--;
					c2 = c2->baseClass;
				}

				while (c1 && c2)
				{
					if (c1 == c2) return c1;
					c1 = c1->baseClass;
					c2 = c2->baseClass;
				}
				return nullptr;
			}

/***********************************************************************
FindPropSymbol
***********************************************************************/
				
			AstClassPropSymbol* FindPropSymbol(AstClassSymbol*& type, const WString& name)
			{
				auto currentType = type;
				while (currentType)
				{
					vint index = currentType->Props().Keys().IndexOf(name);
					if (index != -1)
					{
						return currentType->Props().Values()[index];
					}
					currentType = currentType->baseClass;
				}
				return nullptr;
			}

/***********************************************************************
AstDefFile
***********************************************************************/

			template<typename T>
			T* AstDefFile::CreateSymbol(const WString& symbolName, ParsingTextRange codeRange)
			{
				auto symbol = new T(this, symbolName);
				if (!symbols.Add(symbolName, symbol))
				{
					AddError(
						ParserErrorType::DuplicatedSymbol,
						codeRange,
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
					AddError(
						ParserErrorType::DuplicatedSymbolGlobally,
						codeRange,
						name,
						symbolName,
						ownerManager->symbolMap[symbolName]->Owner()->name
						);
				}

				return symbol;
			}

			AstDefFile::AstDefFile(ParserSymbolManager* _global, AstSymbolManager* _ownerManager, const WString& _name)
				: global(_global)
				, ownerManager(_ownerManager)
				, name(_name)
			{
			}

			bool AstDefFile::AddDependency(const WString& dependency, ParsingTextRange codeRange)
			{
				if (dependencies.Contains(dependency)) return true;
				if (!ownerManager->Files().Keys().Contains(dependency))
				{
					AddError(
						ParserErrorType::FileDependencyNotExists,
						codeRange,
						name,
						dependency
						);
					return false;
				}

				List<WString> visited;
				visited.Add(dependency);
				// TODO: (enumerable) foreach
				for (vint i = 0; i < visited.Count(); i++)
				{
					auto currentName = visited[i];
					if (currentName == name)
					{
						AddError(
							ParserErrorType::FileCyclicDependency,
							codeRange,
							name,
							dependency
							);
						return false;
					}
					auto current = ownerManager->Files()[currentName];
					// TODO: (enumerable) foreach
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

			AstEnumSymbol* AstDefFile::CreateEnum(const WString& symbolName, ParsingTextRange codeRange)
			{
				return CreateSymbol<AstEnumSymbol>(symbolName, codeRange);
			}

			AstClassSymbol* AstDefFile::CreateClass(const WString& symbolName, ParsingTextRange codeRange)
			{
				return CreateSymbol<AstClassSymbol>(symbolName, codeRange);
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
				auto file = new AstDefFile(&global, this, name);
				if (!files.Add(name, file))
				{
					file->AddError(
						ParserErrorType::DuplicatedFile,
						{},
						name
						);
				}
				return file;
			}
		}
	}
}