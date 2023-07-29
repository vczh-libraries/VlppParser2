/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_AST_ASTSYMBOL
#define VCZH_PARSER2_AST_ASTSYMBOL

#include "../ParserGen_Global/ParserSymbol.h"

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
				bool								isPublic = false;
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
				AstEnumItemSymbol*					CreateItem(const WString& itemName, ParsingTextRange codeRange = {});
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
				bool								SetPropType(AstPropType _type, const WString& typeName = WString::Empty, ParsingTextRange codeRange = {});
			};

			enum class AstClassType
			{
				Defined,
				Generated_ToResolve,
				Generated_Common,
			};

			class AstClassSymbol : public AstSymbol
			{
				friend class AstDefFile;
			protected:
				MappedOwning<AstClassPropSymbol>	props;

				AstClassSymbol(AstDefFile* _file, const WString& _name);
			public:
				AstClassType						classType = AstClassType::Defined;
				AstClassSymbol*						baseClass = nullptr;
				collections::List<AstClassSymbol*>	derivedClasses;
				
				AstClassSymbol*						derivedClass_ToResolve = nullptr;
				AstClassSymbol*						derivedClass_Common = nullptr;

				bool								SetBaseClass(const WString& typeName, ParsingTextRange codeRange = {});
				AstClassSymbol*						CreateDerivedClass_ToResolve(ParsingTextRange codeRange);
				AstClassSymbol*						CreateDerivedClass_Common(ParsingTextRange codeRange);
				AstClassPropSymbol*					CreateProp(const WString& propName, ParsingTextRange codeRange = {});
				const auto&							Props() { return props.map; }
				const auto&							PropOrder() { return props.order; }
			};

			extern AstClassSymbol*					FindCommonBaseClass(AstClassSymbol* c1, AstClassSymbol* c2);
			extern AstClassPropSymbol*				FindPropSymbol(AstClassSymbol*& type, const WString& name);

/***********************************************************************
AstDefFile
***********************************************************************/

			class AstDefFile : public Object
			{
				friend class AstSymbolManager;

				using DependenciesList = collections::List<WString>;
				using StringItems = collections::List<WString>;
			protected:
				ParserSymbolManager*		global = nullptr;
				AstSymbolManager*			ownerManager = nullptr;
				WString						name;
				MappedOwning<AstSymbol>		symbols;

				template<typename T>
				T*							CreateSymbol(const WString& symbolName, ParsingTextRange codeRange);

				AstDefFile(ParserSymbolManager* _global, AstSymbolManager* _ownerManager, const WString& _name);
			public:
				DependenciesList			dependencies;
				StringItems					cppNss;
				StringItems					refNss;
				WString						classPrefix;

				AstSymbolManager*			Owner() { return ownerManager; }
				const WString&				Name() { return name; }
				bool						AddDependency(const WString& dependency, ParsingTextRange codeRange = {});
				AstEnumSymbol*				CreateEnum(const WString& symbolName, ParsingTextRange codeRange = {});
				AstClassSymbol*				CreateClass(const WString& symbolName, ParsingTextRange codeRange = {});
				const auto&					Symbols() { return symbols.map; }
				const auto&					SymbolOrder() { return symbols.order; }

				template<typename ...TArgs>
				void AddError(ParserErrorType type, ParsingTextRange codeRange, TArgs&&... args)
				{
					global->AddError(type, { ParserDefFileType::Ast,name,codeRange }, std::forward<TArgs&&>(args)...);
				}
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

				const ParserSymbolManager&	Global() const { return global; }
				const auto&					Files() const { return files.map; }
				const auto&					FileOrder() const { return files.order; }
				const auto&					Symbols() const { return symbolMap; }
			};

			extern AstDefFile*				CreateParserGenTypeAst(AstSymbolManager& manager);
			extern AstDefFile*				CreateParserGenRuleAst(AstSymbolManager& manager);
		}
	}
}

#endif