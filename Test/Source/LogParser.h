#ifndef VCZH_VLPPPARSER2_UNITTEST_LOGPARSER
#define VCZH_VLPPPARSER2_UNITTEST_LOGPARSER

#include "../../../Source/Syntax/SyntaxSymbol.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr;
using namespace vl::glr::automaton;
using namespace vl::glr::parsergen;

FilePath LogSyntax(
	SyntaxSymbolManager& manager,
	const WString& parserName,
	const WString& phase,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
	);

FilePath LogAutomaton(
	const WString& parserName,
	Executable& executable,
	Metadata& metadata,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
	);


template<typename TVisitor, typename T>
void AssertAst(Ptr<T> ast, const wchar_t* output)
{
	List<WString> expected, actual;
	{
		StringReader reader(output);
		while (!reader.IsEnd())
		{
			expected.Add(reader.ReadLine());
		}
	}
	{
		MemoryStream actualStream;
		{
			StreamWriter writer(actualStream);
			TVisitor visitor(writer);
			visitor.printAstCodeRange = false;
			visitor.printTokenCodeRange = false;
			visitor.Print(ast.Obj());
		}
		actualStream.SeekFromBegin(0);
		{
			StreamReader reader(actualStream);
			while (!reader.IsEnd())
			{
				actual.Add(reader.ReadLine());
			}
		}
	}

	TEST_ASSERT(expected.Count() == actual.Count());
	for (vint i = 0; i < expected.Count(); i++)
	{
		auto se = expected[i];
		auto sa = actual[i];
		TEST_ASSERT(se == sa);
	}
}

#endif