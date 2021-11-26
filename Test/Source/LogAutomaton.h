#ifndef VCZH_VLPPPARSER2_UNITTEST_LOGAUTOMATON
#define VCZH_VLPPPARSER2_UNITTEST_LOGAUTOMATON

#include "../../../Source/SyntaxBase.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::regex;
using namespace vl::glr;
using namespace vl::glr::automaton;

void LogInstruction(
	AstIns ins,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	StreamWriter& writer
	);

FilePath LogAutomaton(
	const WString& parserName,
	Executable& executable,
	Metadata& metadata,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
	);

FilePath LogTrace(
	const WString& parserName,
	const WString& caseName,
	Executable& executable,
	Metadata& metadata,
	TraceManager& tm,
	Trace* trace,
	List<RegexToken>& tokens,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
	);

template<typename TVisitor, typename T>
WString PrintAstJson(Ptr<T> ast)
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
		return reader.ReadToEnd();
	}
}

inline void AssertLines(const WString& expectedString, const WString& actualString)
{
	List<WString> expected, actual;
	{
		StringReader reader(expectedString);
		while (!reader.IsEnd())
		{
			expected.Add(reader.ReadLine());
		}
	}
	{
		StringReader reader(actualString);
		while (!reader.IsEnd())
		{
			actual.Add(reader.ReadLine());
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

template<typename TVisitor, typename T>
void AssertAst(Ptr<T> ast, const wchar_t* output)
{
	auto actual = PrintAstJson<TVisitor>(ast);
	AssertLines(WString::Unmanaged(output), actual);
}

#endif