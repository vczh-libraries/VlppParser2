#include "SyntaxSymbolWriter.h"
#include "../../Source/Ast/AstSymbol.h"
#include "../../Source/ParserGen_Generated/ParserGen_Assembler.h"
#include "../../Source/ParserGen_Generated/ParserGen_Lexer.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace syntax_writer;

			extern syntax_writer::Token		tok(ParserGenTokens id);
			extern syntax_writer::Token		tok(ParserGenTokens id, ParserGenFields field);

/***********************************************************************
CreateParserGenTypeSyntax
***********************************************************************/

			void CreateParserGenTypeSyntax(AstSymbolManager& ast, SyntaxSymbolManager& manager)
			{
				auto createRule = [&](const wchar_t* ruleName)
				{
					return manager.CreateRule(WString::Unmanaged(ruleName), -1, false, false);
				};

				manager.name = L"TypeParser";

				auto _enumItem = createRule(L"EnumItem");
				auto _enum = createRule(L"Enum");
				auto _classPropType = createRule(L"ClassPropType");
				auto _classProp = createRule(L"classProp");
				auto _classBody = createRule(L"ClassBody");
				auto _class = createRule(L"Class");
				auto _type = createRule(L"Type");
				auto _file = createRule(L"File");

				_classPropType->isPartial = true;
				_classBody->isPartial = true;

				_file->isParser = true;
				_file->ruleType = dynamic_cast<AstClassSymbol*>(ast.Symbols()[L"AstFile"]);

				using T = ParserGenTokens;
				using C = ParserGenClasses;
				using F = ParserGenFields;

				// ID:name "," as EnumItem
				Clause{ _enumItem } = create(tok(T::ID, F::EnumItem_name) + tok(T::COMMA), C::EnumItem);

				// ["@public"] "enum" ID:name "{" {EnumItem} "}" as Enum
				Clause{ _enum } = create(opt(tok(T::ATT_PUBLIC, F::Type_attPublic)) + tok(T::ENUM) + tok(T::ID, F::Type_name) + tok(T::OPEN_CURLY) + loop(rule(_enumItem, F::Enum_items)) + tok(T::CLOSE_CURLY), C::Enum);

				// "token" as partial ClassProp {propType = "Token"}
				Clause{ _classPropType } = partial(tok(T::TOKEN)).with(F::ClassProp_propType, GlrPropType::Token);

				// ID:propTypeName as partial ClassProp {propType = "Type"}
				Clause{ _classPropType } = partial(tok(T::ID, F::ClassProp_propTypeName)).with(F::ClassProp_propType, GlrPropType::Type);

				// ID:propTypeName "[" "]" as partial ClassProp {propType = "Array"}
				Clause{ _classPropType } = partial(tok(T::ID, F::ClassProp_propTypeName) + tok(T::OPEN_SQUARE) + tok(T::CLOSE_SQUARE)).with(F::ClassProp_propType, GlrPropType::Array);

				// "var" ID:name ":" ClassPropType ";" as ClassProp
				Clause{ _classProp } = create(tok(T::VAR) + tok(T::ID, F::ClassProp_name) + tok(T::COLON) + prule(_classPropType) + tok(T::SEMICOLON), C::ClassProp);

				// ID:name [":" ID:baseClass] "{" {ClassProp} "}" as partial Class
				Clause{ _classBody } = partial(tok(T::ID, F::Type_name) + opt(tok(T::COLON) + tok(T::ID, F::Class_baseClass)) + tok(T::OPEN_CURLY) + loop(rule(_classProp, F::Class_props)) + tok(T::CLOSE_CURLY));

				// ["@public"] ["@ambiguous"] "class" ClassBody {ambiguity = Yes}
				Clause{ _class } = create(opt(tok(T::ATT_PUBLIC, F::Type_attPublic)) + opt(tok(T::ATT_AMBIGUOUS, F::Class_attAmbiguous)) + tok(T::CLASS) + prule(_classBody), C::Class);

				// !Class | !Enum
				Clause{ _type } = use(_enum) | use(_class);

				// type:types {type:types} as AstFile
				Clause{ _file } = create(rule(_type, F::AstFile_types) + loop(rule(_type, F::AstFile_types)), C::AstFile);
			}
		}
	}
}