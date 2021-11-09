/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_AST_ASTSYMBOL
#define VCZH_PARSER2_AST_ASTSYMBOL

#include <Vlpp.h>

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			class AstEnumSymbol;
			class AstClassSymbol;
			class AstDefFile;
			class AstSymbolManager;

			template<typename T>
			struct MappedOwning
			{
				collections::List<Ptr<T>>				items;
				collections::List<WString>				order;
				collections::Dictionary<WString, T*>	map;

				bool Add(const WString& name, T* item)
				{
					items.Add(item);
					if (map.Keys().Contains(name)) return false;
					order.Add(name);
					map.Add(name, item);
					return true;
				}
			};

/***********************************************************************
AstSymbol
***********************************************************************/

			class AstSymbol : public Object
			{
			protected:
				AstDefFile*							ownerFile = nullptr;
				WString								name;

				AstSymbol(AstDefFile* _file, const WString& _name);
			public:
				AstDefFile*							Owner() { return ownerFile; }
				const WString&						Name() { return name; }
			};

/***********************************************************************
AstEnumSymbol
***********************************************************************/

			class AstEnumItemSymbol : public AstSymbol
			{
				friend class AstEnumSymbol;
			protected:
				AstEnumSymbol*						parent = nullptr;

				AstEnumItemSymbol(AstEnumSymbol* _parent, const WString& name);
			public:
				vint								value = 0;

				AstEnumSymbol*						Parent() { return parent; }
			};

			class AstEnumSymbol : public AstSymbol
			{
				friend class AstDefFile;
			protected:
				MappedOwning<AstEnumItemSymbol>		items;

				AstEnumSymbol(AstDefFile* _file, const WString& _name);
			public:
				AstEnumItemSymbol*					CreateItem(const WString& itemName);
				const auto&							Items() { return items.map; }
				const auto&							ItemOrder() { return items.order; }
			};

/***********************************************************************
AstClassSymbol
***********************************************************************/

			enum class AstPropType
			{
				Token,
				Type,
				Array,
			};

			class AstClassPropSymbol : public AstSymbol
			{
				friend class AstClassSymbol;
			protected:
				AstClassSymbol*						parent = nullptr;

				AstClassPropSymbol(AstClassSymbol* _parent, const WString& name);
			public:
				AstPropType							propType = AstPropType::Token;
				AstSymbol*							propSymbol = nullptr;

				AstClassSymbol*						Parent() { return parent; }
				bool								SetPropType(AstPropType _type, const WString& typeName = WString::Empty);
			};

			class AstClassSymbol : public AstSymbol
			{
				friend class AstDefFile;
			protected:
				MappedOwning<AstClassPropSymbol>	props;

				AstClassSymbol(AstDefFile* _file, const WString& _name);
			public:
				AstClassSymbol*						baseClass = nullptr;
				collections::List<AstClassSymbol*>	derivedClasses;

				bool								SetBaseClass(const WString& typeName);
				AstClassPropSymbol*					CreateProp(const WString& propName);
				const auto&							Props() { return props.map; }
				const auto&							PropOrder() { return props.order; }
			};

/***********************************************************************
AstDefFile
***********************************************************************/

			class AstDefFile : public Object
			{
				friend class AstSymbolManager;

				using DependenciesList = collections::List<WString>;
				using StringItems = collections::List<WString>;
			protected:
				AstSymbolManager*			ownerManager = nullptr;
				WString						name;
				MappedOwning<AstSymbol>		symbols;

				template<typename T>
				T*							CreateSymbol(const WString& symbolName);

				AstDefFile(AstSymbolManager* _ownerManager, const WString& _name);
			public:
				DependenciesList			dependencies;
				StringItems					includes;
				StringItems					cppNss;
				StringItems					refNss;
				WString						classPrefix;
				WString						headerGuard;

				AstSymbolManager*			Owner() { return ownerManager; }
				const WString&				Name() { return name; }
				bool						AddDependency(const WString& dependency);
				AstEnumSymbol*				CreateEnum(const WString& symbolName);
				AstClassSymbol*				CreateClass(const WString& symbolName);
				const auto&					Symbols() { return symbols.map; }
				const auto&					SymbolOrder() { return symbols.order; }
			};

/***********************************************************************
AstSymbolManager
***********************************************************************/

			enum class AstErrorType
			{
				DuplicatedFile,				// (fileName)
				FileDependencyNotExists,	// (fileName, dependency)
				FileCyclicDependency,		// (fileName, dependency)
				DuplicatedSymbol,			// (fileName, symbolName)
				DuplicatedSymbolGlobally,	// (fileName, symbolName, anotherFileName)
				DuplicatedClassProp,		// (fileName, className, propName)
				DuplicatedEnumItem,			// (fileName, enumName, propName
				BaseClassNotExists,			// (fileName, className)
				BaseClassNotClass,			// (fileName, className)
				BaseClassCyclicDependency,	// (fileName, className)
				FieldTypeNotExists,			// (fileName, className, propName)
				FieldTypeNotClass,			// (fileName, className, propName)
			};

			struct AstError
			{
				AstErrorType				type;
				WString						arg1;
				WString						arg2;
				WString						arg3;
			};

			class AstSymbolManager : public Object
			{
				using ErrorList = collections::List<AstError>;
				using SymbolMap = collections::Dictionary<WString, AstSymbol*>;
				using StringItems = collections::List<WString>;

				friend class AstDefFile;
			protected:
				MappedOwning<AstDefFile>	files;
				ErrorList					errors;
				SymbolMap					symbolMap;

			public:
				WString						name;
				StringItems					cppNss;
				WString						headerGuard;

				template<typename ...TArgs>
				void						AddError(AstErrorType type, TArgs&& ...args);

				AstDefFile*					CreateFile(const WString& name);
				const auto&					Files() { return files.map; }
				const auto&					FileOrder() { return files.order; }
				const auto&					Symbols() { return symbolMap; }
				const auto&					Errors() { return errors; }
			};


			template<typename ...TArgs>
			void Fill(collections::List<WString>& ss, TArgs&& ...args)
			{
				WString items[] = { args... };
				for (auto& item : items)
				{
					ss.Add(item);
				}
			}

			extern AstDefFile*				CreateParserGenTypeAst(AstSymbolManager& manager);
		}
	}
}

#endif