/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:IfElseAmbiguity
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "IfElseAmbiguity_Assembler.h"

namespace ifelseambiguity
{

/***********************************************************************
IfElseAmbiguityAstInsReceiver : public vl::glr::AstInsReceiverBase
***********************************************************************/

	vl::Ptr<vl::glr::ParsingAstBase> IfElseAmbiguityAstInsReceiver::CreateAstNode(vl::vint32_t type)
	{
		switch((IfElseAmbiguityClasses)type)
		{
		case IfElseAmbiguityClasses::BlockStat:
			return new ifelseambiguity::BlockStat();
		case IfElseAmbiguityClasses::DoStat:
			return new ifelseambiguity::DoStat();
		case IfElseAmbiguityClasses::IfStat:
			return new ifelseambiguity::IfStat();
		case IfElseAmbiguityClasses::Module:
			return new ifelseambiguity::Module();
		case IfElseAmbiguityClasses::Stat:
			throw vl::glr::AstInsException(L"Unable to create abstract class \"ifelseambiguity::Stat\".", vl::glr::AstInsErrorType::UnknownType, type);
		default:
			throw vl::glr::AstInsException(L"The type id does not exist.", vl::glr::AstInsErrorType::UnknownType, type);
		}
	}

	void IfElseAmbiguityAstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::Ptr<vl::glr::ParsingAstBase> value)
	{
		switch((IfElseAmbiguityFields)field)
		{
		case IfElseAmbiguityFields::BlockStat_stats:
			{
				auto typedObject = dynamic_cast<ifelseambiguity::BlockStat*>(object);
				if (!typedObject) throw vl::glr::AstInsException(L"Field \"ifelseambiguity::BlockStat::stats\" does not exist in the current object.", vl::glr::AstInsErrorType::FieldNotExistsInType, field);
				auto typedValue = value.Cast<ifelseambiguity::Stat>();
				if (!typedValue) throw vl::glr::AstInsException(L"Field \"ifelseambiguity::BlockStat::stats\" cannot be assigned with an uncompatible value.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
				typedObject->stats.Add(typedValue);
			}
			break;
		case IfElseAmbiguityFields::IfStat_elseBranch:
			{
				auto typedObject = dynamic_cast<ifelseambiguity::IfStat*>(object);
				if (!typedObject) throw vl::glr::AstInsException(L"Field \"ifelseambiguity::IfStat::elseBranch\" does not exist in the current object.", vl::glr::AstInsErrorType::FieldNotExistsInType, field);
				if (typedObject->elseBranch) throw vl::glr::AstInsException(L"Field \"ifelseambiguity::IfStat::elseBranch\" has already been assigned.", vl::glr::AstInsErrorType::FieldReassigned, field);
				auto typedValue = value.Cast<ifelseambiguity::Stat>();
				if (!typedValue) throw vl::glr::AstInsException(L"Field \"ifelseambiguity::IfStat::elseBranch\" cannot be assigned with an uncompatible value.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
				typedObject->elseBranch = typedValue;
			}
			break;
		case IfElseAmbiguityFields::IfStat_thenBranch:
			{
				auto typedObject = dynamic_cast<ifelseambiguity::IfStat*>(object);
				if (!typedObject) throw vl::glr::AstInsException(L"Field \"ifelseambiguity::IfStat::thenBranch\" does not exist in the current object.", vl::glr::AstInsErrorType::FieldNotExistsInType, field);
				if (typedObject->thenBranch) throw vl::glr::AstInsException(L"Field \"ifelseambiguity::IfStat::thenBranch\" has already been assigned.", vl::glr::AstInsErrorType::FieldReassigned, field);
				auto typedValue = value.Cast<ifelseambiguity::Stat>();
				if (!typedValue) throw vl::glr::AstInsException(L"Field \"ifelseambiguity::IfStat::thenBranch\" cannot be assigned with an uncompatible value.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
				typedObject->thenBranch = typedValue;
			}
			break;
		case IfElseAmbiguityFields::Module_stat:
			{
				auto typedObject = dynamic_cast<ifelseambiguity::Module*>(object);
				if (!typedObject) throw vl::glr::AstInsException(L"Field \"ifelseambiguity::Module::stat\" does not exist in the current object.", vl::glr::AstInsErrorType::FieldNotExistsInType, field);
				if (typedObject->stat) throw vl::glr::AstInsException(L"Field \"ifelseambiguity::Module::stat\" has already been assigned.", vl::glr::AstInsErrorType::FieldReassigned, field);
				auto typedValue = value.Cast<ifelseambiguity::Stat>();
				if (!typedValue) throw vl::glr::AstInsException(L"Field \"ifelseambiguity::Module::stat\" cannot be assigned with an uncompatible value.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
				typedObject->stat = typedValue;
			}
			break;
		default:
			throw vl::glr::AstInsException(L"The field id does not exist.", vl::glr::AstInsErrorType::UnknownField, field);
		}
	}

	void IfElseAmbiguityAstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token)
	{
		switch((IfElseAmbiguityFields)field)
		{
		case IfElseAmbiguityFields::BlockStat_stats:
			throw vl::glr::AstInsException(L"Field \"ifelseambiguity::BlockStat::stats\" is not a token.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
		case IfElseAmbiguityFields::IfStat_elseBranch:
			throw vl::glr::AstInsException(L"Field \"ifelseambiguity::IfStat::elseBranch\" is not a token.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
		case IfElseAmbiguityFields::IfStat_thenBranch:
			throw vl::glr::AstInsException(L"Field \"ifelseambiguity::IfStat::thenBranch\" is not a token.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
		case IfElseAmbiguityFields::Module_stat:
			throw vl::glr::AstInsException(L"Field \"ifelseambiguity::Module::stat\" is not a token.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
		default:
			throw vl::glr::AstInsException(L"The field id does not exist.", vl::glr::AstInsErrorType::UnknownField, field);
		}
	}

	void IfElseAmbiguityAstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::vint32_t enumItem)
	{
		switch((IfElseAmbiguityFields)field)
		{
		case IfElseAmbiguityFields::BlockStat_stats:
			throw vl::glr::AstInsException(L"Field \"ifelseambiguity::BlockStat::stats\" is not an enum item.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
		case IfElseAmbiguityFields::IfStat_elseBranch:
			throw vl::glr::AstInsException(L"Field \"ifelseambiguity::IfStat::elseBranch\" is not an enum item.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
		case IfElseAmbiguityFields::IfStat_thenBranch:
			throw vl::glr::AstInsException(L"Field \"ifelseambiguity::IfStat::thenBranch\" is not an enum item.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
		case IfElseAmbiguityFields::Module_stat:
			throw vl::glr::AstInsException(L"Field \"ifelseambiguity::Module::stat\" is not an enum item.", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
		default:
			throw vl::glr::AstInsException(L"The field id does not exist.", vl::glr::AstInsErrorType::UnknownField, field);
		}
	}

	const wchar_t* IfElseAmbiguityTypeName(IfElseAmbiguityClasses type)
	{
		const wchar_t* results[] = {
			L"BlockStat",
			L"DoStat",
			L"IfStat",
			L"Module",
			L"Stat",
		};
		vl::vint index = (vl::vint)type;
		return 0 <= index && index < 5 ? results[index] : nullptr;
	}

	const wchar_t* IfElseAmbiguityFieldName(IfElseAmbiguityFields field)
	{
		switch(field)
		{
		case IfElseAmbiguityFields::BlockStat_stats:
			return L"BlockStat::stats";
		case IfElseAmbiguityFields::IfStat_elseBranch:
			return L"IfStat::elseBranch";
		case IfElseAmbiguityFields::IfStat_thenBranch:
			return L"IfStat::thenBranch";
		case IfElseAmbiguityFields::Module_stat:
			return L"Module::stat";
		default:
			return nullptr;
		}
	}

	vl::Ptr<vl::glr::ParsingAstBase> IfElseAmbiguityAstInsReceiver::ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates)
	{
		switch((IfElseAmbiguityClasses)type)
		{
		case IfElseAmbiguityClasses::BlockStat:
		case IfElseAmbiguityClasses::DoStat:
		case IfElseAmbiguityClasses::IfStat:
		case IfElseAmbiguityClasses::Module:
		case IfElseAmbiguityClasses::Stat:
			throw vl::glr::AstInsException(L"The type is not configured to allow ambiguity.", vl::glr::AstInsErrorType::UnsupportedAmbiguityType, type);
		default:
			throw vl::glr::AstInsException(L"The type id does not exist.", vl::glr::AstInsErrorType::UnknownType, type);
		}
	}
}