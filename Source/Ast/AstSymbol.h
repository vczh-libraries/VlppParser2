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
			class AstDefFileGroup;
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
				AstDefFile*							Owner() const { return ownerFile; }
				const WString&						Name() const { return name; }
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

				AstEnumSymbol*						Parent() const { return parent; }
			};

			class AstEnumSymbol : public AstSymbol
			{
				friend class AstDefFile;
			protected:
				MappedOwning<AstEnumItemSymbol>		items;

				AstEnumSymbol(AstDefFile* _file, const WString& _name);
			public:
				AstEnumItemSymbol*					CreateItem(const WString& itemName, ParsingTextRange codeRange = {});

				const auto&							Items() const { return items.map; }
				const auto&							ItemOrder() const { return items.order; }
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

				AstClassSymbol*						Parent() const { return parent; }
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

				const auto&							Props() const { return props.map; }
				const auto&							PropOrder() const { return props.order; }
			};

			extern AstClassSymbol*					FindCommonBaseClass(AstClassSymbol* c1, AstClassSymbol* c2);
			extern AstClassPropSymbol*				FindPropSymbol(AstClassSymbol*& type, const WString& name);

/***********************************************************************
AstDefFile
***********************************************************************/

			class AstDefFile : public Object
			{
				friend class AstDefFileGroup;
				using DependenciesList = collections::List<WString>;
				using StringItems = collections::List<WString>;
			protected:
				AstDefFileGroup*			ownerGroup = nullptr;
				WString						name;
				MappedOwning<AstSymbol>		symbols;

				template<typename T>
				T*							CreateSymbol(const WString& symbolName, ParsingTextRange codeRange);

				AstDefFile(AstDefFileGroup* _ownerGroup, const WString& _name);
			public:
				DependenciesList			dependencies;
				StringItems					cppNss;
				StringItems					refNss;
				WString						classPrefix;

				AstDefFileGroup*			Owner() const { return ownerGroup; }
				const WString&				Name() const { return name; }
				AstEnumSymbol*				CreateEnum(const WString& symbolName, bool isPublic = false, ParsingTextRange codeRange = {});
				AstClassSymbol*				CreateClass(const WString& symbolName, bool isPublic = false, ParsingTextRange codeRange = {});

				const auto&					Symbols() const { return symbols.map; }
				const auto&					SymbolOrder() const { return symbols.order; }

				template<typename ...TArgs>
				void AddError(ParserErrorType type, ParsingTextRange codeRange, TArgs&&... args);
			};

/***********************************************************************
AstDefFileGroup
***********************************************************************/

			class AstDefFileGroup : public Object
			{
				friend class AstDefFile;
				friend class AstSymbolManager;
				using DependenciesList = collections::List<WString>;
				using StringItems = collections::List<WString>;
				using SymbolMap = collections::Dictionary<WString, AstSymbol*>;
			protected:
				AstSymbolManager*			ownerManager = nullptr;
				WString						name;
				MappedOwning<AstDefFile>	files;
				SymbolMap					symbolMap;

				AstDefFileGroup(AstSymbolManager* _ownerManager, const WString& _name);
			public:
				DependenciesList			dependencies;
				StringItems					cppNss;
				StringItems					refNss;
				WString						classPrefix;

				AstSymbolManager*			Owner() const { return ownerManager; }
				const WString&				Name() const { return name; }
				bool						AddDependency(const WString& dependency, ParsingTextRange codeRange = {});
				AstDefFile*					CreateFile(const WString& name);

				const auto&					Files() const { return files.map; }
				const auto&					FileOrder() const { return files.order; }
				const auto&					Symbols() const { return symbolMap; }

				template<typename ...TArgs>
				void AddError(ParserErrorType type, ParsingTextRange codeRange, TArgs&&... args);
			};

/***********************************************************************
AstSymbolManager
***********************************************************************/

			class AstSymbolManager : public Object
			{
				friend class AstDefFile;
				using SymbolGroup = collections::Group<WString, AstSymbol*>;
				using FileMap = collections::Dictionary<WString, AstDefFile*>;
			protected:
				ParserSymbolManager&			global;
				MappedOwning<AstDefFileGroup>	fileGroups;
				SymbolGroup						symbolGroup;
				FileMap							fileMap;

			public:
				AstSymbolManager(ParserSymbolManager& _global);

				AstDefFileGroup*				CreateFileGroup(const WString& name);

				const ParserSymbolManager&		Global() const { return global; }
				const auto&						FileGroups() const { return fileGroups.map; }
				const auto&						FileGroupOrder() const { return fileGroups.order; }
				const auto&						Symbols() const { return symbolGroup; }
				const auto&						Files() const { return fileMap; }
			};

			extern AstDefFile*					CreateParserGenTypeAst(AstSymbolManager& manager);
			extern AstDefFile*					CreateParserGenRuleAst(AstSymbolManager& manager);

/***********************************************************************
AddError
***********************************************************************/

			template<typename ...TArgs>
			void AstDefFile::AddError(ParserErrorType type, ParsingTextRange codeRange, TArgs&&... args)
			{
				auto&& global = const_cast<ParserSymbolManager&>(ownerGroup->Owner()->Global());
				global.AddError(type, { ParserDefFileType::Ast,name,codeRange }, std::forward<TArgs&&>(args)...);
			}

			template<typename ...TArgs>
			void AstDefFileGroup::AddError(ParserErrorType type, ParsingTextRange codeRange, TArgs&&... args)
			{
				auto&& global = const_cast<ParserSymbolManager&>(ownerManager->Global());
				global.AddError(type, { ParserDefFileType::Ast,name,codeRange }, std::forward<TArgs&&>(args)...);
			}
		}
	}
}

#endif