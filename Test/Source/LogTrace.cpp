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
	bool bold,
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
	StringReader reader(GenerateToStream([&](StreamWriter& writer)
	{
		writer.WriteString(L"[" + itow(trace->allocatedIndex) + L"]: ");
		switch (trace->byInput)
		{
		case -1:
			writer.WriteLine(L"<Start>");
			break;
		case Executable::EndingInput:
			writer.WriteLine(L"<Ending>");
			break;
		case Executable::LeftrecInput:
			writer.WriteLine(L"<Leftrec>");
			break;
		default:
			{
				auto&& token = tokens[trace->currentTokenIndex];
				writer.WriteString(L"{" + tokenName((vint32_t)(trace->byInput - Executable::TokenBegin)) + L"} ");
				writer.WriteLine(token.reading, token.length);
			}
		}

		writer.WriteLine(stateLabel((vint32_t)trace->state));

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

		if (trace->returnStack != -1)
		{
			auto returnStack = tm.GetReturnStack(trace->returnStack);
			while (true)
			{
				auto&& returnDesc = executable.returns[returnStack->returnIndex];
				writer.WriteLine(ruleName((vint32_t)returnDesc.consumedRule) + L" -> " + stateLabel((vint32_t)returnDesc.returnState));
				if (returnStack->previous == -1) break;
				returnStack = tm.GetReturnStack(returnStack->previous);
			}
		}
	}));

	List<WString> lines;
	while (!reader.IsEnd())
	{
		lines.Add(reader.ReadLine());
	}
	lines.RemoveAt(lines.Count() - 1);

	{
		vint thickness = bold ? 2 : 1;
		vint length = 0;
		for (auto&& line : lines)
		{
			if (length < line.Length()) length = line.Length();
		}

		vint blockLength = length + (thickness + 1) * 2;
		Array<wchar_t> buffer(blockLength);

		for (vint i = 0; i < thickness; i++)
		{
			buffer[i] = L'+';
			buffer[blockLength - i - 1] = L'+';
		}
		for (vint i = 0; i < length + 2; i++)
		{
			buffer[i + thickness] = bold ? L'=' : L'-';
		}
		auto border = WString::CopyFrom(&buffer[0], blockLength);

		for (vint i = 0; i < thickness; i++)
		{
			buffer[i] = L'|';
			buffer[blockLength - i - 1] = L'|';
		}
		for (vint i = 0; i < length + 2; i++)
		{
			buffer[i + thickness] = L' ';
		}

		traceLogs.Add(trace, border);

		for (auto&& line : lines)
		{
			for (vint i = 0; i < length; i++)
			{
				buffer[i + thickness + 1] = L' ';
			}
			memcpy(&buffer[thickness + 1], line.Buffer(), sizeof(wchar_t)* line.Length());
			traceLogs.Add(trace, WString::CopyFrom(&buffer[0], blockLength));
		}

		traceLogs.Add(trace, border);
	}
}

/***********************************************************************
RenderTraceTree
***********************************************************************/

struct TraceTree;

enum class TraceCellContent
{
	Empty,
	Trace,
	LinkToToken,
};

struct TraceCell
{
	TraceCellContent		content = TraceCellContent::Empty;
	TraceTree*				traceTree = nullptr;
	vint					rows = 1;
	vint					columns = 1;
};

struct TraceTree
{
	bool					tokenTrace = false;
	Trace*					trace = nullptr;
	List<Ptr<TraceTree>>	children;

	vint					column = -1;
	vint					row = -1;

	void AddChildTrace(Trace* trace, bool firstLevel, Group<Trace*, Trace*>& nexts, List<Trace*>& tokenTraces)
	{
		auto tree = MakePtr<TraceTree>();
		tree->tokenTrace = trace->byInput >= Executable::TokenBegin;
		tree->trace = trace;
		children.Add(tree);

		if (tree->tokenTrace)
		{
			if (!firstLevel && !tokenTraces.Contains(trace))
			{
				tokenTraces.Add(trace);
			}
		}
		else
		{
			vint index = nexts.Keys().IndexOf(trace);
			if (index != -1)
			{
				for (auto childTrace : nexts.GetByIndex(index))
				{
					tree->AddChildTrace(childTrace, false, nexts, tokenTraces);
				}
			}
		}
	}

	vint SetColumns(vint start)
	{
		column = start;
		vint current = start;
		for (auto child : children)
		{
			current += child->SetColumns(current);
		}
		return current == start ? 1 : current - start;
	}

	vint SetRows(vint start)
	{
		if (tokenTrace) return 0;
		row = start;
		vint depth = 0;
		for (auto child : children)
		{
			vint childDepth = child->SetRows(start + 1);
			if (depth < childDepth) depth = childDepth;
		}
		return depth + 1;
	}

	void SetTokenTraceRows(vint rows)
	{
		if (tokenTrace)
		{
			row = rows;
		}
		for (auto child : children)
		{
			child->SetTokenTraceRows(rows);
		}
	}

	void FillBoard(Array<TraceCell>& board, vint depth, vint width)
	{
		if (trace)
		{
			auto&& cell = board[row * width + column];
			cell.content = TraceCellContent::Trace;
			cell.traceTree = this;
		}

		for (auto child : children)
		{
			child->FillBoard(board, depth, width);
			if (child->tokenTrace)
			{
				for (vint i = row + 1; i < child->row; i++)
				{
					board[i * width + child->column].content = TraceCellContent::LinkToToken;
				}
			}
		}
	}
};

struct TraceBoardBuffer
{
	Array<Array<wchar_t>>	lines;

	TraceBoardBuffer(vint rows, vint columns)
	{
		lines.Resize(rows);
		for (vint i = 0; i < rows; i++)
		{
			auto&& line = lines[i];
			line.Resize(columns + 1);
			for (vint j = 0; j < columns + 1; j++)
			{
				line[j] = 0;
			}
		}
	}

	Array<wchar_t>& Prepare(vint row, vint column)
	{
		auto&& line = lines[row];
		for (vint i = 0; i < column; i++)
		{
			if (line[i] == 0) line[i] = L' ';
		}
		return line;
	}

	void Set(vint row, vint column, wchar_t c)
	{
		auto&& line = Prepare(row, column);
		line[column] = c;
	}

	void Draw(vint row, vint column, wchar_t c)
	{
		auto&& line = Prepare(row, column);
		switch (c)
		{
		case L'|':
			switch (line[column])
			{
			case L' ':
				line[column] = L'|';
				break;
			case L'-':
				line[column] = L'+';
				break;
			}
			break;
		case L'-':
			switch (line[column])
			{
			case L' ':
				line[column] = L'-';
				break;
			case L'|':
				line[column] = L'+';
				break;
			}
			break;
		default:
			line[column] = c;
		}
	}
};

void RenderTraceTree(
	Trace* rootTrace,
	Group<Trace*, Trace*>& nexts,
	Group<Trace*, WString>& traceLogs,
	StreamWriter& writer
)
{
	List<vint> sendPositions;
	List<vint> receivePositions;
	Group<vint, vint> sendTos;
	List<Trace*> startTraces;
	startTraces.Add(rootTrace);

	while (startTraces.Count() > 0)
	{
		List<Trace*> endTraces;
		auto root = MakePtr<TraceTree>();

		for (auto trace : startTraces)
		{
			root->AddChildTrace(trace, true, nexts, endTraces);
		}
		vint width = root->SetColumns(0);
		vint depth = root->SetRows(-1);
		root->SetTokenTraceRows(depth);

		Array<TraceCell> board((depth + 1) * width);
		root->FillBoard(board, depth, width);

		for (vint i = 0; i < board.Count(); i++)
		{
			auto&& cell = board[i];
			if (cell.traceTree && cell.traceTree->row != depth)
			{
				auto trace = cell.traceTree->trace;
				auto&& lines = traceLogs[trace];
				cell.rows = lines.Count();
				cell.columns = lines[0].Length();
			}
		}

		vint bufferRows = 0;
		vint bufferColumns = 0;

		for (vint row = 0; row < depth + 1; row++)
		{
			vint maxRows = 0;
			for (vint column = 0; column < width; column++)
			{
				vint rows = board[row * width + column].rows;
				if (maxRows < rows) maxRows = rows;
			}
			for (vint column = 0; column < width; column++)
			{
				board[row * width + column].rows = maxRows;
			}
			bufferRows += maxRows;
		}

		for (vint column = 0; column < width; column++)
		{
			vint maxColumns = 0;
			for (vint row = 0; row < depth + 1; row++)
			{
				vint columns = board[row * width + column].columns;
				if (maxColumns < columns) maxColumns = columns;
			}
			for (vint row = 0; row < depth + 1; row++)
			{
				board[row * width + column].columns = maxColumns;
			}
			bufferColumns += maxColumns;
		}

		vint connectionOffset = 0;
		if (sendTos.Count() > 0)
		{
			connectionOffset = 5;
		}
		bufferRows += connectionOffset + depth - 1;
		bufferColumns += width - 1;
		TraceBoardBuffer buffer(bufferRows, bufferColumns);

		for (auto&& line : buffer.lines)
		{
			writer.WriteLine(&line[0]);
		}

		CopyFrom(startTraces, endTraces);
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
	CHECK_ERROR(tm.concurrentCount > 0, L"Cannot log failed traces!");
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
			RenderTrace(
				traceLogs,
				trace,
				trace->byInput >= Executable::TokenBegin,
				executable,
				tm,
				tokens,
				typeName,
				fieldName,
				tokenName,
				ruleName,
				stateLabel
			);

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