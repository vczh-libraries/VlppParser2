#include "AstToCode.h"

namespace vl::glr::parsergen
{
	using namespace collections;
	using namespace stream;
	
	namespace ast_printer
	{
		class TypeAstToStringVisitor
			: public Object
			, protected virtual GlrType::IVisitor
		{
		protected:
			TextWriter&					writer;
		
		public:
			TypeAstToStringVisitor(
				TextWriter& _writer
			)
				: writer(_writer)
			{
			}
		
			void VisitType(Ptr<GlrType> clause)
			{
				clause->Accept(this);
			}
		protected:

			////////////////////////////////////////////////////////////////////////
			// GlrType::IVisitor
			////////////////////////////////////////////////////////////////////////

			void Visit(GlrEnum* node) override
			{
				if (node->attPublic)
				{
					writer.WriteString(L"@public ");
				}
				writer.WriteString(L"enum ");
				writer.WriteString(node->name.value);
				writer.WriteLine(L"");
				writer.WriteLine(L"{");
				for (auto item : node->items)
				{
					writer.WriteString(L"  ");
					writer.WriteString(item->name.value);
					writer.WriteLine(L",");
				}
				writer.WriteLine(L"}");
			}

			void Visit(GlrClass* node) override
			{
				if (node->attPublic)
				{
					writer.WriteString(L"@public ");
				}
				if (node->attAmbiguous)
				{
					writer.WriteString(L"@ambiguous ");
				}
				writer.WriteString(L"class ");
				writer.WriteString(node->name.value);
				if (node->baseClass)
				{
					writer.WriteString(L" : ");
					writer.WriteString(node->baseClass.value);
				}
				writer.WriteLine(L"");
				writer.WriteLine(L"{");
				for (auto prop : node->props)
				{
					writer.WriteString(L"  var ");
					writer.WriteString(prop->name.value);
					writer.WriteString(L" : ");
					switch (prop->propType)
					{
					case GlrPropType::Token:
						writer.WriteString(L"token");
						break;
					case GlrPropType::Type:
						writer.WriteString(prop->propTypeName.value);
						break;
					case GlrPropType::Array:
						writer.WriteString(prop->propTypeName.value);
						writer.WriteString(L"[]");
						break;
					default:;
					}
					writer.WriteLine(L";");
				}
				writer.WriteLine(L"}");
			}
		};
	}
	
	/***********************************************************************
	TypeAstToCode
	***********************************************************************/
	
	void TypeAstToCode(
		Ptr<GlrAstFile> file,
		TextWriter& writer
	)
	{
		ast_printer::TypeAstToStringVisitor visitor(writer);
		for (auto type : file->types)
		{
			visitor.VisitType(type);
			writer.WriteLine(L"");
		}
	}
}