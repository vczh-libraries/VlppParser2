/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_AST_ASTSYMBOL
#define VCZH_PARSER2_AST_ASTSYMBOL

#include "../ParserGen/ParserSymbol.h"

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

			class AstSymbolManager : public Object
			{
				using SymbolMap = collections::Dictionary<WString, AstSymbol*>;

				friend class AstDefFile;
			protected:
				MappedOwning<AstDefFile>	files;
				SymbolMap					symbolMap;
				ParserSymbolManager&		global;

			public:
				AstSymbolManager(ParserSymbolManager& _global);

				AstDefFile*					CreateFile(const WString& name);

				ParserSymbolManager&		Global() { return global; }
				const auto&					Files() { return files.map; }
				const auto&					FileOrder() { return files.order; }
				const auto&					Symbols() { return symbolMap; }
			};

			extern AstDefFile*				CreateParserGenTypeAst(AstSymbolManager& manager);
		}
	}
}

#endif