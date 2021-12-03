#include "LogTrace.h"

extern WString GetTestOutputPath();
extern FilePath GetOutputDir(const WString& parserName);

/***********************************************************************
LogInstruction
***********************************************************************/

void LogInstruction(
	AstIns ins,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	StreamWriter& writer
)
{
	switch (ins.type)
	{
	case AstInsType::Token:
		writer.WriteLine(L"Token()");
		break;
	case AstInsType::EnumItem:
		writer.WriteLine(L"EnumItem(" + itow(ins.param) + L")");
		break;
	case AstInsType::BeginObject:
		writer.WriteLine(L"BeginObject(" + typeName(ins.param) + L")");
		break;
	case AstInsType::BeginObjectLeftRecursive:
		writer.WriteLine(L"BeginObjectLeftRecursive(" + typeName(ins.param) + L")");
		break;
	case AstInsType::ReopenObject:
		writer.WriteLine(L"ReopenObject()");
		break;
	case AstInsType::EndObject:
		writer.WriteLine(L"EndObject()");
		break;
	case AstInsType::DiscardValue:
		writer.WriteLine(L"DiscardValue()");
		break;
	case AstInsType::Field:
		writer.WriteLine(L"Field(" + fieldName(ins.param) + L")");
		break;
	case AstInsType::ResolveAmbiguity:
		writer.WriteLine(L"ResolveAmbiguity(" + typeName(ins.param) + L", " + itow(ins.count) + L")");
		break;
	default:
		writer.WriteLine(L"<UNKNOWN-INSTRUCTION>");
	}
}

/***********************************************************************
LogTraceInsReceiver
***********************************************************************/

class LogTraceInsReceiver : public Object, public virtual IAstInsReceiver
{
protected:
	const Func<WString(vint32_t)>&		typeName;
	const Func<WString(vint32_t)>&		fieldName;
	const Func<WString(vint32_t)>&		tokenName;
	StreamWriter&						writer;
public:
	LogTraceInsReceiver(
		const Func<WString(vint32_t)>& _typeName,
		const Func<WString(vint32_t)>& _fieldName,
		const Func<WString(vint32_t)>& _tokenName,
		StreamWriter& _writer)
		: typeName(_typeName)
		, fieldName(_fieldName)
		, tokenName(_tokenName)
		, writer(_writer)
	{
	}

	void Execute(AstIns instruction, const regex::RegexToken& token) override
	{
		writer.WriteString(L"<");
		writer.WriteString(tokenName((vint32_t)token.token));
		writer.WriteString(L":");
		writer.WriteString(token.reading, token.length);
		writer.WriteString(L"> ");
		LogInstruction(instruction, typeName, fieldName, writer);
	}

	Ptr<ParsingAstBase> Finished() override
	{
		return {};
	}
};

/***********************************************************************
LogTraceExecution
***********************************************************************/

FilePath LogTraceExecution(
	const WString& parserName,
	const WString& caseName,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName,
	const Func<void(IAstInsReceiver&)>& callback
)
{
	auto outputDir = GetOutputDir(parserName);
	auto outputFile = outputDir / (L"Instructions[" + caseName + L"].txt");
	auto content = GenerateToStream([&](StreamWriter& writer)
	{
		LogTraceInsReceiver receiver(typeName, fieldName, tokenName, writer);
		callback(receiver);
	});
	File(outputFile).WriteAllText(content, true, BomEncoder::Utf8);
	return outputFile;
}

FilePath LogTraceExecution(
	const WString& parserName,
	const WString& caseName,
	TraceManager& tm,
	Trace* trace,
	List<RegexToken>& tokens,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
)
{
	return LogTraceExecution(parserName, caseName, typeName, fieldName, tokenName, [&](IAstInsReceiver& receiver)
	{
		tm.ExecuteTrace(trace, receiver, tokens);
	});
}

/***********************************************************************
RenderTrace
***********************************************************************/

void RenderTrace(
	Group<Trace*, WString>& traceLogs,
	Trace* trace,
	Executable& executable,
	List<RegexToken>& tokens,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
)
{
	StringReader reader(GenerateToStream([&](StreamWriter& writer)
	{
		writer.WriteString(L"[" + itow(trace->allocatedIndex) + L"]: ");
		switch (trace->byInput)
		{
		case -1:
			writer.WriteString(L"<Start>");
			break;
		case Executable::EndingInput:
			writer.WriteString(L"<Ending>");
			break;
		case Executable::LeftrecInput:
			writer.WriteString(L"<Leftrec>");
			break;
		default:
			writer.WriteString(L"{" + tokenName((vint32_t)(trace->byInput - Executable::TokenBegin)) + L"}");
		}

		if (trace->currentTokenIndex != -1)
		{
			auto&& token = tokens[trace->currentTokenIndex];
			writer.WriteString(L" ");
			writer.WriteLine(token.reading, token.length);
		}
		else
		{
			writer.WriteLine(L"");
		}

		if (trace->byEdge != -1)
		{
			auto& edgeDesc = executable.edges[trace->byEdge];
			for (vint insRef = 0; insRef < edgeDesc.insBeforeInput.count; insRef++)
			{
				vint insIndex = edgeDesc.insBeforeInput.start + insRef;
				auto& ins = executable.instructions[insIndex];
				writer.WriteString(L"  > ");
				LogInstruction(ins, typeName, fieldName, writer);
			}
			for (vint insRef = 0; insRef < edgeDesc.insAfterInput.count; insRef++)
			{
				vint insIndex = edgeDesc.insAfterInput.start + insRef;
				auto& ins = executable.instructions[insIndex];
				writer.WriteString(L"  > ");
				LogInstruction(ins, typeName, fieldName, writer);
			}
		}

		if (trace->executedReturn != -1)
		{
			auto& returnDesc = executable.returns[trace->executedReturn];
			for (vint insRef = 0; insRef < returnDesc.insAfterInput.count; insRef++)
			{
				vint insIndex = returnDesc.insAfterInput.start + insRef;
				auto& ins = executable.instructions[insIndex];
				writer.WriteString(L"  > ");
				LogInstruction(ins, typeName, fieldName, writer);
			}
		}
	}));
	while (!reader.IsEnd())
	{
		traceLogs.Add(trace, reader.ReadLine());
	}
	{
		auto&& logs = const_cast<List<WString>&>(traceLogs[trace]);
		logs.RemoveAt(logs.Count() - 1);

		vint length = 0;
		for (auto&& line : logs)
		{
			if (length < line.Length()) length = line.Length();
		}
		for (vint i = 0; i < logs.Count(); i++)
		{
			auto&& line = logs[i];
			Array<wchar_t> newLine(length + 2);
			newLine[0] = L'|';
			memcpy(&newLine[1], line.Buffer(), sizeof(wchar_t) * line.Length());
			for (vint j = line.Length() + 1; j < length + 1; j++)
			{
				newLine[j] = L' ';
			}
			newLine[length + 1] = L'|';
			logs[i] = WString::CopyFrom(&newLine[0], length + 2);
		}

		Array<wchar_t> newLine(length + 2);
		newLine[0] = L'+';
		for (vint i = 1; i < length + 1; i++)
		{
			newLine[i] = L'-';
		}
		newLine[length + 1] = L'+';
		auto border = WString::CopyFrom(&newLine[0], length + 2);
		logs.Insert(0, border);
		logs.Add(border);
	}
}

/***********************************************************************
RenderTraceTree
***********************************************************************/

void RenderTraceTree(
	Trace* rootTrace,
	Group<Trace*, Trace*>& nexts,
	Group<Trace*, WString>& traceLogs,
	StreamWriter& writer
)
{
	while (true)
	{
		auto&& logs = traceLogs[rootTrace];
		for (auto&& line : logs)
		{
			writer.WriteLine(line);
		}
		writer.WriteLine(L"");

		vint index = nexts.Keys().IndexOf(rootTrace);
		if (index == -1) break;
		rootTrace = nexts.GetByIndex(index)[0];
	}
}

/***********************************************************************
LogTraceManager
***********************************************************************/

FilePath LogTraceManager(
	const WString& parserName,
	const WString& caseName,
	Executable& executable,
	TraceManager& tm,
	List<RegexToken>& tokens,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName,
	const Func<WString(vint32_t)>& ruleName,
	const Func<WString(vint32_t)>& stateLabel
)
{
	Trace* rootTrace = nullptr;
	Group<Trace*, Trace*> nexts;
	Group<Trace*, WString> traceLogs;
	{
		SortedList<Trace*> availables;
		List<Trace*> visited;

		for (vint i = 0; i < tm.concurrentCount; i++)
		{
			visited.Add(tm.concurrentTraces->Get(i));
		}

		for (vint i = 0; i < visited.Count(); i++)
		{
			auto trace = visited[i];
			RenderTrace(traceLogs, trace, executable, tokens, typeName, fieldName, tokenName);

			if (!availables.Contains(trace))
			{
				availables.Add(trace);

				if (trace->previous == -1)
				{
					CHECK_ERROR(rootTrace == nullptr, L"Impossible to have multiple root traces!");
					rootTrace = trace;
				}
				else
				{
					auto previous = tm.GetTrace(trace->previous);
					visited.Add(previous);
					nexts.Add(previous, trace);
				}
			}
		}
	}

	auto outputDir = GetOutputDir(parserName);
	auto outputFile = outputDir / (L"Trace[" + caseName + L"].txt");
	auto content = GenerateToStream([&](StreamWriter& writer)
	{
		RenderTraceTree(rootTrace, nexts, traceLogs, writer);
	});
	File(outputFile).WriteAllText(content, true, BomEncoder::Utf8);
	return outputFile;
}