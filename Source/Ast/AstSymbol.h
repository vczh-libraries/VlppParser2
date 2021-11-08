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
			class AstClassSymbol;
			class AstDefFile;
			class AstSymbolManager;

/***********************************************************************
AstSymbol
***********************************************************************/

			class AstSymbol : public Object
			{
				friend class AstDefFile;
			protected:
				AstDefFile*					ownerFile = nullptr;
				WString						name;

			public:
			};

			class AstEnumItemSymbol : public AstSymbol
			{
			protected:
				vint						value = 0;

			public:
			};

			class AstEnumSymbol : public AstSymbol
			{
				using ItemList = collections::List<Ptr<AstEnumItemSymbol>>;
				using ItemMap = collections::Dictionary<WString, AstEnumItemSymbol*>;
			protected:
				ItemList					items;
				ItemMap						itemMap;

			public:
				const ItemMap&				Items() { return itemMap; }
			};

			enum class AstPropType
			{
				Token,
				Class,
				ClassArray,
			};

			class AstClassPropSymbol : public AstSymbol
			{
			protected:
				AstPropType					propType = AstPropType::Token;
				AstClassSymbol*				classSymbol = nullptr;

			public:
			};

			class AstClassSymbol : public AstSymbol
			{
				using PropList = collections::List<Ptr<AstClassPropSymbol>>;
				using PropMap = collections::Dictionary<WString, AstClassPropSymbol*>;
			protected:
				PropList					props;
				PropMap						propMap;

			public:
				const PropMap&				Props() { return propMap; }
			};

/***********************************************************************
AstDefFile
***********************************************************************/

			class AstDefFile : public Object
			{
				friend class AstSymbolManager;

				using DependenciesList = collections::List<WString>;
				using NamespaceItems = collections::List<WString>;
				using SymbolList = collections::List<Ptr<AstSymbol>>;
				using SymbolMap = collections::Dictionary<WString, AstSymbol*>;
			protected:
				AstSymbolManager*			ownerManager = nullptr;
				SymbolList					symbols;
				SymbolMap					symbolMap;

				template<typename T>
				T*							CreateSymbol(const WString& symbolName);

				AstDefFile(AstSymbolManager* _ownerManager, const WString& _name);
			public:
				WString						name;
				DependenciesList			dependencies;
				NamespaceItems				nss;

				AstEnumSymbol*				CreateEnum(const WString& symbolName);
				AstClassSymbol*				CreateClass(const WString& symbolName);
				const SymbolMap&			Symbols() { return symbolMap; }
			};

/***********************************************************************
AstSymbolManager
***********************************************************************/

			enum class AstErrorType
			{
				DuplicatedFile,				// (fileName)
				DuplicatedSymbol,			// (fileName, symbolName)
				DuplicatedSymbolGlobally,	// (fileName, symbolName, anotherFileName)
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
				using FileList = collections::List<Ptr<AstDefFile>>;
				using ErrorList = collections::List<AstError>;
				using FileMap = collections::Dictionary<WString, AstDefFile*>;
				using SymbolMap = collections::Dictionary<WString, AstSymbol*>;

				friend class AstDefFile;
			protected:
				FileList					files;
				ErrorList					errors;
				FileMap						fileMap;
				SymbolMap					symbolMap;

			public:
				template<typename ...TArgs>
				void						AddError(AstErrorType type, TArgs&& ...args);

				AstDefFile*					CreateFile(const WString& name);
				const FileMap&				Files() { return fileMap; }
				const SymbolMap&			Symbols() { return symbolMap; }
			};
		}
	}
}

#endif