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

				auto& symbols = parent->Owner()->Owner()->Symbols();
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

				if (parent->Owner() != propSymbol->Owner() && !propSymbol->isPublic)
				{
					ownerFile->AddError(
						ParserErrorType::FieldTypeNotPublic,
						codeRange,
						ownerFile->Name(),
						parent->Name(),
						name
						);
					return false;
				}

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
				auto& symbols = ownerFile->Owner()->Symbols();
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
				else if (ownerFile != newBaseClass->Owner() && !newBaseClass->isPublic)
				{
					ownerFile->AddError(
						ParserErrorType::BaseClassNotPublic,
						codeRange,
						ownerFile->Name(),
						name,
						typeName
						);
					return false;
				}

				if (newBaseClass->derivedClass_Common)
				{
					newBaseClass = newBaseClass->derivedClass_Common;
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

			AstClassSymbol* AstClassSymbol::CreateDerivedClass_ToResolve(ParsingTextRange codeRange)
			{
				if (!derivedClass_ToResolve)
				{
					auto derived = ownerFile->CreateClass(name + L"ToResolve", false, codeRange);
					derived->classType = AstClassType::Generated_ToResolve;
					derived->baseClass = this;
					derivedClasses.Add(derived);

					auto prop = derived->CreateProp(L"candidates", codeRange);
					prop->propType = AstPropType::Array;
					prop->propSymbol = this;

					derivedClass_ToResolve = derived;
				}
				return derivedClass_ToResolve;
			}

			AstClassSymbol* AstClassSymbol::CreateDerivedClass_Common(ParsingTextRange codeRange)
			{
				if (!derivedClass_Common)
				{
					auto derived = ownerFile->CreateClass(name + L"Common", isPublic, codeRange);
					derived->classType = AstClassType::Generated_Common;
					derived->baseClass = this;
					derivedClasses.Add(derived);

					derivedClass_Common = derived;
				}
				return derivedClass_Common;
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
						ParserErrorType::DuplicatedSymbolInFile,
						codeRange,
						name,
						symbolName
						);
				}
				else if (ownerGroup->symbolMap.Keys().Contains(symbolName))
				{
					AddError(
						ParserErrorType::DuplicatedSymbolInFileGroup,
						codeRange,
						name,
						symbolName,
						ownerGroup->symbolMap[symbolName]->Owner()->name
						);
				}
				else
				{
					ownerGroup->symbolMap.Add(symbolName, symbol);
					ownerGroup->ownerManager->symbolGroup.Add(symbolName, symbol);
				}

				return symbol;
			}

			AstDefFile::AstDefFile(AstDefFileGroup* _ownerGroup, const WString& _name)
				: ownerGroup(_ownerGroup)
				, name(_name)
			{
			}

			AstEnumSymbol* AstDefFile::CreateEnum(const WString& symbolName, bool isPublic, ParsingTextRange codeRange)
			{
				auto symbol = CreateSymbol<AstEnumSymbol>(symbolName, codeRange);
				symbol->isPublic = isPublic;
				return symbol;
			}

			AstClassSymbol* AstDefFile::CreateClass(const WString& symbolName, bool isPublic, ParsingTextRange codeRange)
			{
				auto symbol = CreateSymbol<AstClassSymbol>(symbolName, codeRange);
				symbol->isPublic = isPublic;
				return symbol;
			}

/***********************************************************************
AstDefFileGroup
***********************************************************************/

			AstDefFileGroup::AstDefFileGroup(AstSymbolManager* _ownerManager, const WString& _name)
				: ownerManager(_ownerManager)
				, name(_name)
			{
			}

			bool AstDefFileGroup::AddDependency(const WString& dependency, ParsingTextRange codeRange)
			{
				if (dependencies.Contains(dependency)) return true;
				if (!ownerManager->FileGroups().Keys().Contains(dependency))
				{
					AddError(
						ParserErrorType::FileGroupDependencyNotExists,
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
							ParserErrorType::FileGroupCyclicDependency,
							codeRange,
							name,
							dependency
							);
						return false;
					}
					auto current = ownerManager->FileGroups()[currentName];
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

			AstDefFile* AstDefFileGroup::CreateFile(const WString& name)
			{
				auto file = new AstDefFile(this, name);
				if (!files.Add(name, file))
				{
					AddError(
						ParserErrorType::DuplicatedFile,
						{},
						name
						);
				}
				return file;
			}

/***********************************************************************
AstSymbolManager
***********************************************************************/

			AstSymbolManager::AstSymbolManager(ParserSymbolManager& _global)
				: global(_global)
			{
			}

			AstDefFileGroup* AstSymbolManager::CreateFileGroup(const WString& name)
			{
				auto fileGroup = new AstDefFileGroup(this, name);
				if (!fileGroups.Add(name, fileGroup))
				{
					global.AddError(
						ParserErrorType::DuplicatedFileGroup,
						{ ParserDefFileType::AstGroup,name },
						name
						);
				}
				return fileGroup;
			}
		}
	}
}