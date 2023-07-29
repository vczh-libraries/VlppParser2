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
			}

			void Visit(GlrClass* node) override
			{
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