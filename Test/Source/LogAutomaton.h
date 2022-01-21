#ifndef VCZH_VLPPPARSER2_UNITTEST_LOGAUTOMATON
#define VCZH_VLPPPARSER2_UNITTEST_LOGAUTOMATON

#include "LogTrace.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::regex;
using namespace vl::glr;
using namespace vl::glr::automaton;

extern FilePath LogAutomatonWithPath(
	const FilePath& outputFile,
	Executable& executable,
	Metadata& metadata,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName,
	const Func<WString(vint32_t)>& switchName
	);

extern FilePath LogAutomaton(
	const WString& parserName,
	Executable& executable,
	Metadata& metadata,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName,
	const Func<WString(vint32_t)>& switchName
	);

#endif