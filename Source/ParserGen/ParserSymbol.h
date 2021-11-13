/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_PARSERGEN_PARSERSYMBOl
#define VCZH_PARSER2_PARSERGEN_PARSERSYMBOl

#include <Vlpp.h>

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
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

			template<typename ...TArgs>
			void Fill(collections::List<WString>& ss, TArgs&& ...args)
			{
				WString items[] = { args... };
				for (auto& item : items)
				{
					ss.Add(item);
				}
			}

/***********************************************************************
ParserSymbolManager
***********************************************************************/

			enum class ParserErrorType
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

				DuplicatedToken,			// (tokenName)
			};

			struct ParserError
			{
				ParserErrorType				type;
				WString						arg1;
				WString						arg2;
				WString						arg3;
			};

			class ParserSymbolManager : public Object
			{
				using ErrorList = collections::List<ParserError>;
				using StringItems = collections::List<WString>;
			protected:

				ErrorList					errors;
			public:
				WString						name;
				StringItems					cppNss;
				WString						headerGuard;

				const auto&					Errors() { return errors; }

				template<typename ...TArgs>
				void AddError(ParserErrorType type, TArgs&& ...args)
				{
					ParserError error;
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
			};

/***********************************************************************
Utility
***********************************************************************/

			extern void						InitializeParserSymbolManager(ParserSymbolManager& manager);
		}
	}
}

#endif