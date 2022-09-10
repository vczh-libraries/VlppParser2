#include "LogTrace.h"

extern WString GetTestOutputPath();
extern FilePath GetOutputDir(const WString& parserName);

/***********************************************************************
LogInstruction (AstIns)
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
	case AstInsType::DelayFieldAssignment:
		writer.WriteLine(L"DelayFieldAssignment()");
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
	case AstInsType::LriStore:
		writer.WriteLine(L"LriStore()");
		break;
	case AstInsType::LriFetch:
		writer.WriteLine(L"LriFetch()");
		break;
	case AstInsType::Field:
		writer.WriteLine(L"Field(" + fieldName(ins.param) + L")");
		break;
	case AstInsType::FieldIfUnassigned:
		writer.WriteLine(L"FieldIfUnassigned(" + fieldName(ins.param) + L")");
		break;
	case AstInsType::ResolveAmbiguity:
		writer.WriteLine(L"ResolveAmbiguity(" + typeName(ins.param) + L", " + itow(ins.count) + L")");
		break;
	case AstInsType::AccumulatedDfa:
		writer.WriteLine(L"AccumulatedDfa(" + itow(ins.count) + L")");
		break;
	case AstInsType::AccumulatedEoRo:
		writer.WriteLine(L"AccumulatedEoRo(" + itow(ins.count) + L")");
		break;
	default:
		writer.WriteLine(L"<UNKNOWN-INSTRUCTION>");
	}
}

/***********************************************************************
LogInstruction (SwitchIns)
***********************************************************************/

void LogInstruction(
	SwitchIns ins,
	const Func<WString(vint32_t)>& switchName,
	StreamWriter& writer
)
{
	switch (ins.type)
	{
	case SwitchInsType::SwitchPushFrame:
		writer.WriteLine(L"+switches");
		break;
	case SwitchInsType::SwitchWriteTrue:
		writer.WriteLine(switchName(ins.param) + L" <- TRUE");
		break;
	case SwitchInsType::SwitchWriteFalse:
		writer.WriteLine(switchName(ins.param) + L" <- FALSE");
		break;
	case SwitchInsType::SwitchPopFrame:
		writer.WriteLine(L"-switches");
		break;
	case SwitchInsType::ConditionRead:
		writer.WriteLine(L"<- " + switchName(ins.param));
		break;
	case SwitchInsType::ConditionNot:
		writer.WriteLine(L"!");
		break;
	case SwitchInsType::ConditionAnd:
		writer.WriteLine(L"&&");
		break;
	case SwitchInsType::ConditionOr:
		writer.WriteLine(L"||");
		break;
	case SwitchInsType::ConditionTest:
		writer.WriteLine(L"?");
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

	void Execute(AstIns instruction, const regex::RegexToken& token, vint32_t tokenIndex) override
	{
		writer.WriteString(L"<[");
		writer.WriteString(itow(tokenIndex));
		writer.WriteString(L"]");
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
	List<RegexToken>& tokens,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
)
{
	return LogTraceExecution(parserName, caseName, typeName, fieldName, tokenName, [&](IAstInsReceiver& receiver)
	{
		tm.ExecuteTrace(receiver, tokens);
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
	const Func<WString(vint32_t)>& stateLabel,
	const Func<WString(vint32_t)>& switchName
)
{
	StringReader reader(GenerateToStream([&](StreamWriter& writer)
	{
		auto logInsRefLink = [&tm, &writer](vint32_t first)
		{
			auto ref = first;
			while (ref != -1)
			{
				if (ref != first) writer.WriteString(L", ");
				auto link = tm.GetInsExec_InsRefLink(ref);
				writer.WriteString(itow(link->trace) + L"@" + itow(link->ins));
				ref = link->previous;
			}
		};

		auto logObjRefLink = [&tm, &writer](vint32_t first)
		{
			auto ref = first;
			while (ref != -1)
			{
				if (ref != first) writer.WriteString(L", ");
				auto link = tm.GetInsExec_ObjRefLink(ref);
				writer.WriteString(itow(link->id));
				ref = link->previous;
			}
		};

		auto logContext = [&tm, &writer, &logObjRefLink](InsExec_Context& context, const wchar_t* indentation)
		{
			writer.WriteString(indentation);
			if (context.createStack == -1)
			{
				writer.WriteLine(L"CSTop: []");
			}
			else
			{
				auto ieCSTop = tm.GetInsExec_CreateStack(context.createStack);
				writer.WriteString(L"CSTop: [");
				logObjRefLink(ieCSTop->objectIds);
				writer.WriteLine(
					L"] [" +
					itow(ieCSTop->allocatedIndex) +
					L" -> " +
					itow(ieCSTop->previous) +
					L"]");
			}

			writer.WriteString(indentation);
			if (context.objectStack == -1)
			{
				writer.WriteLine(L"OSTop: []");
			}
			else
			{
				auto ieOSTop = tm.GetInsExec_ObjectStack(context.objectStack);
				writer.WriteString(L"OSTop: [");
				logObjRefLink(ieOSTop->objectIds);
				writer.WriteLine(
					L"] [" +
					itow(ieOSTop->allocatedIndex) +
					L" -> " +
					itow(ieOSTop->previous) +
					L"]");
			}

			writer.WriteString(indentation);
			if (context.lriStoredObjects == -1)
			{
				writer.WriteLine(L"LriStored: []");
			}
			else
			{
				writer.WriteString(L"LriStored: [");
				logObjRefLink(context.lriStoredObjects);
				writer.WriteLine(L"]");
			}
		};

		/***********************************************************************
		Header
		***********************************************************************/

		writer.WriteString(L"[" + itow(trace->allocatedIndex) + L"]: ");
		if (trace->state == -1)
		{
			writer.WriteLine(L"<Merging>");
		}

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

		if (trace->traceExecRef != -1)
		{
			auto traceExec = tm.GetTraceExec(trace->traceExecRef);
			if (traceExec->insExecRefs.start == -1)
			{
				writer.WriteLine(L"  TraceExec: [" + itow(traceExec->allocatedIndex) + L"]");
			}
			else
			{
				writer.WriteLine(
					L"  TraceExec: [" + itow(traceExec->allocatedIndex) +
					L", " + itow(traceExec->insExecRefs.start) +
					L":" + itow(traceExec->insExecRefs.count) +
					L"]");
			}

			if (traceExec->branchData.forwardTrace != -1)
			{
				writer.WriteLine(
					(trace->state == -1 ? L"  SharedTrace: " : L"  BranchHead: ") +
					itow(traceExec->branchData.forwardTrace) +
					L" BranchDepth: " +
					itow(traceExec->branchData.branchDepth));
			}

			if (traceExec->ambiguity != -1)
			{
				auto ta = tm.GetTraceAmbiguity(traceExec->ambiguity);
				writer.WriteLine(L"[AMBIGUITY-RESOLVING]");
				writer.WriteString(L"  objs: [");
				logObjRefLink(ta->topObjectIds);
				writer.WriteString(L"] [");
				logObjRefLink(ta->bottomObjectIds);
				writer.WriteLine(L"]");

				writer.WriteLine(L"  first: " + itow(ta->firstTrace) + L" prefix: " + itow(ta->prefix));
				writer.WriteLine(L"  last: " + itow(ta->lastTrace) + L" postfix: " + itow(ta->postfix));
			}
		}

		if (trace->state == -1)
		{
			if (trace->traceExecRef != -1)
			{
				writer.WriteLine(L"[CONTEXT]");
				auto traceExec = tm.GetTraceExec(trace->traceExecRef);
				logContext(traceExec->context, L"  ");
			}
			return;
		}

		writer.WriteLine(stateLabel((vint32_t)trace->state));

		/***********************************************************************
		Holding Competition
		***********************************************************************/

		if (trace->competitionRouting.holdingCompetitions != -1)
		{
			writer.WriteLine(L"[HOLDING COMPETITION]:");
			vint32_t cid = trace->competitionRouting.holdingCompetitions;
			while (cid != -1)
			{
				auto competition = tm.GetCompetition(cid);
				writer.WriteString(L"  [" + itow(competition->allocatedIndex) + L"]");
				switch (competition->status)
				{
				case CompetitionStatus::Holding:
					writer.WriteString(L"[HOLDING]");
					break;
				case CompetitionStatus::HighPriorityWin:
					writer.WriteString(L"[HIGH PRIORITY WIN]");
					break;
				case CompetitionStatus::LowPriorityWin:
					writer.WriteString(L"[LOW PRIORITY WIN]");
					break;
				}
				writer.WriteLine(
					L"[RULE: " + itow(competition->ruleId) +
					L"][CLAUSE: " + itow(competition->clauseId) + L"]");
				cid = competition->nextHoldCompetition;
			}
		}

		/***********************************************************************
		Switch Instructions
		***********************************************************************/

		if (trace->byEdge != -1)
		{
			auto& edgeDesc = executable.edges[trace->byEdge];
			if (edgeDesc.insSwitch.start != -1)
			{
				writer.WriteLine(L"[SWITCH-INSTRUCTIONS]:");
				for (vint32_t i = 0; i < edgeDesc.insSwitch.count; i++)
				{
					auto ins = executable.switchInstructions[edgeDesc.insSwitch.start + i];
					writer.WriteString(L"    ");
					LogInstruction(ins, switchName, writer);
				}
			}
		}

		/***********************************************************************
		AST Instructions
		***********************************************************************/

		writer.WriteLine(L"[AST-INSTRUCTIONS]:");
		vint32_t c1 = 0, c2 = 0, c3 = 0;
		if (trace->byEdge != -1)
		{
			auto& edgeDesc = executable.edges[trace->byEdge];
			c1 = edgeDesc.insBeforeInput.count;
			c2 = c1 + edgeDesc.insAfterInput.count;
			c3 = c2;
		}
		if (trace->executedReturnStack != -1)
		{
			auto returnStack = tm.GetReturnStack(trace->executedReturnStack);
			auto& returnDesc = executable.returns[returnStack->returnIndex];
			c3 = c2 + returnDesc.insAfterInput.count;
		}

		for (vint32_t i = 0; i < c3; i++)
		{
			if (trace->traceExecRef != -1)
			{
				auto traceExec = tm.GetTraceExec(trace->traceExecRef);
				auto insExec = tm.GetInsExec(traceExec->insExecRefs.start + i);
				logContext(insExec->contextBeforeExecution, L"    ");
			}

			AstIns ins;
			if (i < c1)
			{
				auto& edgeDesc = executable.edges[trace->byEdge];
				ins = executable.astInstructions[edgeDesc.insBeforeInput.start + i];
				writer.WriteString(L"  - ");
			}
			else if (i < c2)
			{
				auto& edgeDesc = executable.edges[trace->byEdge];
				ins = executable.astInstructions[edgeDesc.insAfterInput.start + (i - c1)];
				writer.WriteString(L"  + ");
			}
			else
			{
				auto returnStack = tm.GetReturnStack(trace->executedReturnStack);
				auto& returnDesc = executable.returns[returnStack->returnIndex];
				ins = executable.astInstructions[returnDesc.insAfterInput.start + (i - c2)];
				writer.WriteString(L"  > ");
			}

			LogInstruction(ins, typeName, fieldName, writer);

			if (trace->traceExecRef != -1)
			{
				auto traceExec = tm.GetTraceExec(trace->traceExecRef);
				auto insExec = tm.GetInsExec(traceExec->insExecRefs.start + i);

				if (insExec->createdObjectId != -1)
				{
					auto ieObject = tm.GetInsExec_Object(insExec->createdObjectId);
					writer.WriteString(
						L"      obj:" + itow(ieObject->allocatedIndex) +
						L", new:" + itow(ieObject->bo_bolr_Trace) +
						L"@" + itow(ieObject->bo_bolr_Ins)
					);

					if (ieObject->lrObjectIds != -1)
					{
						writer.WriteString(L" lrs:[");
						logObjRefLink(ieObject->lrObjectIds);
						writer.WriteString(L"]");
					}

					if (ieObject->dfaInsRefs!= -1)
					{
						writer.WriteString(L" dfas:[");
						logInsRefLink(ieObject->dfaInsRefs);
						writer.WriteString(L"]");
					}
					writer.WriteLine(L"");
				}

				if (insExec->objRefs != -1)
				{
					writer.WriteString(L"      objRefs: ");
					logObjRefLink(insExec->objRefs);
					writer.WriteLine(L"");
				}

				if (insExec->eoInsRefs != -1)
				{
					writer.WriteString(L"      eoInsRefs: ");
					logInsRefLink(insExec->eoInsRefs);
					writer.WriteLine(L"");
				}

				writer.WriteLine(L"");
			}
		}

		if (trace->traceExecRef != -1)
		{
			auto traceExec = tm.GetTraceExec(trace->traceExecRef);
			logContext(traceExec->context, L"    ");
		}

		/***********************************************************************
		Rule Stack
		***********************************************************************/

		if (trace->returnStack != -1)
		{
			writer.WriteLine(L"[RETURN STACK]:");
			auto returnStack = tm.GetReturnStack(trace->returnStack);
			while (true)
			{
				auto&& returnDesc = executable.returns[returnStack->returnIndex];
				writer.WriteLine(L"  [" + itow(returnStack->allocatedIndex) + L"][" + itow(returnStack->fromTrace) + L"]: " + ruleName((vint32_t)returnDesc.consumedRule) + L" -> " + stateLabel((vint32_t)returnDesc.returnState));
				if (returnStack->previous == -1) break;
				returnStack = tm.GetReturnStack(returnStack->previous);
			}
		}

		/***********************************************************************
		Carried Competition
		***********************************************************************/

		if (trace->competitionRouting.carriedCompetitions != -1)
		{
			writer.WriteLine(L"[CARRIED COMPETITION]:");
			auto acId = trace->competitionRouting.carriedCompetitions;
			while (acId != -1)
			{
				auto ac = tm.GetAttendingCompetitions(acId);
				auto cpt = tm.GetCompetition(ac->competition);
				writer.WriteLine(
					L"  [" + itow(ac->allocatedIndex) +
					L"]: competition[" + itow(cpt->allocatedIndex) +
					L"][RS: " + itow(ac->returnStack) +
					L"] " + (ac->forHighPriority ? L"high" : L"low"));
				acId = ac->nextCarriedAC;
			}
		}

		/***********************************************************************
		Switches
		***********************************************************************/

		if (trace->switchValues != -1)
		{
			writer.WriteLine(L"[SWITCHES]:");
			auto sv = tm.GetSwitches(trace->switchValues);
			for (vint32_t i = 0; i < executable.switchDefaultValues.Count(); i++)
			{
				writer.WriteString(L"  " + switchName(i) + L" : ");
				vint32_t row = i / 8 * sizeof(vuint32_t);
				vint32_t column = i % 8 * sizeof(vuint32_t);
				vuint32_t& value = sv->values[row];
				bool read = (value >> column) % 2 == 1;
				writer.WriteLine(read ? L"true" : L"false");
			}
		}

		/***********************************************************************
		Predecessors and Successors
		***********************************************************************/

		if (trace->predecessors.first != -1)
		{
			writer.WriteString(L"[PREDECESSORS " + itow(trace->predecessors.first) + L"-" + itow(trace->predecessors.last) + L"]: ");
			auto tid = trace->predecessors.first;
			while (tid != -1)
			{
				auto t = tm.GetTrace(tid);
				writer.WriteString(L"[" + itow(t->allocatedIndex) + L"]");
				tid = t->predecessors.siblingNext;
			}
			writer.WriteLine(L"");
		}

		if (trace->successors.first != -1)
		{
			writer.WriteString(L"[SUCCESORS " + itow(trace->successors.first) + L"-" + itow(trace->successors.last) + L"]: ");
			auto tid = trace->successors.first;
			while (tid != -1)
			{
				auto t = tm.GetTrace(tid);
				writer.WriteString(L"[" + itow(t->allocatedIndex) + L"]");
				tid = t->successors.siblingNext;
			}
			writer.WriteLine(L"");
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
TraceCell
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

/***********************************************************************
TraceTree
***********************************************************************/

struct TraceTree
{
	bool					endTrace = false;
	Trace*					trace = nullptr;
	List<Ptr<TraceTree>>	children;

	vint					column = -1;
	vint					row = -1;

	void AddChildTrace(
		Trace* trace,
		TraceManager& tm,
		bool firstLevel,
		Dictionary<Trace*, Ptr<TraceTree>>& nonEndTraces,
		List<Trace*>& endTraces
	)
	{
		bool endTrace = !firstLevel && trace->byInput >= Executable::TokenBegin;
		if (!endTrace)
		{
			vint index = nonEndTraces.Keys().IndexOf(trace);
			if (index != -1)
			{
				auto startNode = nonEndTraces.Values()[index];
				children.Add(startNode);
				return;
			}
		}

		auto tree = MakePtr<TraceTree>();
		tree->endTrace = endTrace;
		tree->trace = trace;
		children.Add(tree);

		if (!endTrace)
		{
			nonEndTraces.Add(trace, tree);
		}

		if (tree->endTrace)
		{
			if (!endTraces.Contains(trace))
			{
				endTraces.Add(trace);
			}
			return;
		}

		auto successorId = trace->successors.first;
		while (successorId != -1)
		{
			auto successor = tm.GetTrace(successorId);
			tree->AddChildTrace(successor, tm, false, nonEndTraces, endTraces);
			successorId = successor->successors.siblingNext;
		}
	}

	vint SetColumns(vint start)
	{
		column = start;
		vint current = start;
		for (auto child : children)
		{
			if (child->column == -1 || child->column > current)
			{
				current += child->SetColumns(current);
			}
			else
			{
				current += 1;
			}
		}
		return current == start ? 1 : current - start;
	}

	vint SetRows(vint start)
	{
		if (endTrace) return 0;
		row = start;
		vint depth = 0;
		for (auto child : children)
		{
			if (child->row > start + 1) continue;
			vint childDepth = child->SetRows(start + 1);
			if (depth < childDepth) depth = childDepth;
		}
		return depth + 1;
	}

	void SetEndTraceRows(vint rows)
	{
		if (endTrace)
		{
			row = rows;
		}
		for (auto child : children)
		{
			child->SetEndTraceRows(rows);
		}
	}

	void FillBoard(Array<TraceCell>& board, vint depth, vint width)
	{
		if (trace && !endTrace)
		{
			auto&& cell = board[row * width + column];
			cell.content = TraceCellContent::Trace;
			cell.traceTree = this;
		}

		for (auto child : children)
		{
			child->FillBoard(board, depth, width);
			if (child->endTrace)
			{
				for (vint i = row + 1; i < child->row; i++)
				{
					board[i * width + child->column].content = TraceCellContent::LinkToToken;
				}
			}
		}
	}
};

/***********************************************************************
TraceBoardBuffer
***********************************************************************/

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

	void Set(vint row, vint column, const WString& s)
	{
		auto&& line = Prepare(row, column);
		for (vint i = 0; i < s.Length(); i++)
		{
			line[column + i] = s[i];
		}
	}

	void Draw(vint row, vint column, wchar_t c)
	{
		auto&& line = Prepare(row, column);
		switch (c)
		{
		case L'|':
			switch (line[column])
			{
			case 0:
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
			case 0:
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

/***********************************************************************
RenderTraceTree
***********************************************************************/

void RenderTraceTreeConnection(
	TraceTree* tree,
	Group<Trace*, WString>& traceLogs,
	Array<TraceCell>& board,
	vint depth,
	vint width,
	TraceBoardBuffer& buffer,
	vint connectionOffset,
	Array<vint>& rowStarts,
	Array<vint>& columnStarts,
	Group<Trace*, vint>& endTraceConnectionPositions,
	StreamWriter& writer
)
{
	if (tree->endTrace) return;
	if (tree->trace)
	{
		if (tree->children.Count() == 0) return;
		auto&& cell = board[tree->row * width + tree->column];
		vint startRow = rowStarts[tree->row] + cell.rows + connectionOffset;
		vint startColumn = columnStarts[tree->column];

		vint beginRow = rowStarts[tree->row] + traceLogs[tree->trace].Count() + connectionOffset;
		for (vint i = beginRow; i < startRow; i++)
		{
			buffer.Draw(i, startColumn, L'|');
		}

		vint maxColumn = startColumn;
		for (auto child : tree->children)
		{
			vint endColumn = columnStarts[child->column];
			if (maxColumn < endColumn) maxColumn = endColumn;

			vint endRow = rowStarts[child->row] - 1 + connectionOffset;
			for (vint i = startRow + 1; i < endRow; i++)
			{
				if (i >= buffer.lines.Count()) break;
				buffer.Draw(i, (startColumn > endColumn ? startColumn : endColumn), L'|');
			}

			if (endRow - 1 < buffer.lines.Count())
			{
				vint c1 = startColumn;
				vint c2 = endColumn;
				if (c1 > c2)
				{
					vint t = c1;
					c1 = c2;
					c2 = t;
				}
				for (vint i = c1; i <= c2; i++)
				{
					buffer.Draw(endRow - 1, i, L'-');
				}

				if (endRow < buffer.lines.Count())
				{
					buffer.Draw(endRow, endColumn, L'|');
				}

				if (child->endTrace)
				{
					endTraceConnectionPositions.Add(child->trace, startColumn);
				}
			}
			else
			{
				if (child->endTrace)
				{
					endTraceConnectionPositions.Add(child->trace, (startRow + 1 < buffer.lines.Count() ? endColumn : startColumn));
				}
			}
		}

		if (startRow < buffer.lines.Count() - 1)
		{
			buffer.Draw(startRow, startColumn, L'|');
			for (vint i = startColumn; i <= maxColumn; i++)
			{
				buffer.Draw(startRow + 1, i, L'-');
			}
		}
	}

	for (auto child : tree->children)
	{
		RenderTraceTreeConnection(
			child.Obj(),
			traceLogs,
			board,
			depth,
			width,
			buffer,
			connectionOffset,
			rowStarts,
			columnStarts,
			endTraceConnectionPositions,
			writer);
	}
}

void RenderTraceTree(
	Trace* rootTrace,
	TraceManager& tm,
	Group<Trace*, WString>& traceLogs,
	StreamWriter& writer
)
{
	SortedList<vint> sendPositions;
	Array<vint> receivePositions;
	Group<vint, vint> sendTos;
	List<Trace*> startTraces;
	startTraces.Add(rootTrace);

	while (startTraces.Count() > 0)
	{
		vint minWidthDueToConnection = 0;
		if (sendTos.Count() > 0)
		{
			for (auto p : sendPositions)
			{
				if (minWidthDueToConnection < p)
				{
					minWidthDueToConnection = p;
				}
			}
			for (auto p : receivePositions)
			{
				if (minWidthDueToConnection < p)
				{
					minWidthDueToConnection = p;
				}
			}
			minWidthDueToConnection += 5;
		}

		List<Trace*> endTraces;
		auto root = MakePtr<TraceTree>();
		{
			Dictionary<Trace*, Ptr<TraceTree>> nonEndTraces;
			for (auto trace : startTraces)
			{
				root->AddChildTrace(trace, tm, true, nonEndTraces, endTraces);
			}
		}
		vint width = root->SetColumns(0);
		vint depth = root->SetRows(-1) - 1;
		root->SetEndTraceRows(depth);

		Array<TraceCell> board(depth * width);
		root->FillBoard(board, depth, width);

		for (vint i = 0; i < board.Count(); i++)
		{
			auto&& cell = board[i];
			if (cell.traceTree && !cell.traceTree->endTrace)
			{
				auto trace = cell.traceTree->trace;
				auto&& lines = traceLogs[trace];
				cell.rows = lines.Count();
				cell.columns = lines[0].Length();
			}
		}

		vint columnPadding = 4;
		vint rowPadding = 3;
		vint bufferRows = 0;
		vint bufferColumns = 0;
		Array<vint> rowStarts(depth + 1);
		Array<vint> columnStarts(width);

		for (vint row = 0; row < depth; row++)
		{
			vint maxRows = 0;
			rowStarts[row] = bufferRows + row * rowPadding;
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
		rowStarts[depth] = bufferRows + depth * rowPadding;

		for (vint column = 0; column < width; column++)
		{
			vint maxColumns = 0;
			columnStarts[column] = bufferColumns + column * columnPadding;
			for (vint row = 0; row < depth; row++)
			{
				vint columns = board[row * width + column].columns;
				if (maxColumns < columns) maxColumns = columns;
			}
			for (vint row = 0; row < depth; row++)
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
		bufferRows += connectionOffset + (depth - 1) * rowPadding;
		bufferColumns += (width - 1) * columnPadding;
		TraceBoardBuffer buffer(
			bufferRows,
			(minWidthDueToConnection > bufferColumns ? minWidthDueToConnection : bufferColumns)
			);

		for (vint i = 0; i < board.Count(); i++)
		{
			auto&& cell = board[i];
			if (cell.traceTree && !cell.traceTree->endTrace)
			{
				auto trace = cell.traceTree->trace;
				auto&& lines = traceLogs[trace];
				vint x = rowStarts[cell.traceTree->row] + connectionOffset;
				vint y = columnStarts[cell.traceTree->column];

				for (vint u = 0; u < lines.Count(); u++)
				{
					buffer.Set(x + u, y, lines[u]);
				}
			}
		}

		Group<Trace*, vint> endTraceConnectionPositions;
		RenderTraceTreeConnection(
			root.Obj(),
			traceLogs,
			board,
			depth,
			width,
			buffer,
			connectionOffset,
			rowStarts,
			columnStarts,
			endTraceConnectionPositions,
			writer);

		receivePositions.Resize(startTraces.Count());
		for (auto [child, index] : indexed(root->children))
		{
			receivePositions[index] = columnStarts[child->column];
		}

		if (sendTos.Count() > 0)
		{
			for (auto [p, i] : indexed(sendPositions))
			{
				WString label = L"[";
				for (auto [r, ri] : indexed(sendTos[i]))
				{
					if (ri != 0) label += L"/";
					label += itow(r);
				}
				label += L"]";

				buffer.Set(0, p, label);
				buffer.Draw(1, p + 1, L'|');
				buffer.Draw(2, p + 1, L'|');
			}

			for (auto [p, i] : indexed(receivePositions))
			{
				buffer.Draw(2, p + 1, L'|');
				buffer.Draw(3, p + 1, L'|');
				buffer.Set(4, p, L"[" + itow(i) + L"]");
			}

			for (auto [is, ir] : sendTos)
			{
				auto p1 = sendPositions[is] + 1;
				auto p2 = receivePositions[ir] + 1;
				if (p1 > p2)
				{
					auto t = p1;
					p1 = p2;
					p2 = t;
				}
				for (vint i = p1; i <= p2; i++)
				{
					buffer.Draw(2, i, L'-');
				}
			}
		}

		sendPositions.Clear();
		sendTos.Clear();
		for (auto endTrace : endTraces)
		{
			for (vint position : endTraceConnectionPositions[endTrace])
			{
				if (!sendPositions.Contains(position))
				{
					sendPositions.Add(position);
				}
			}
		}

		for (auto [endTrace, index] : indexed(endTraces))
		{
			for (vint position : endTraceConnectionPositions[endTrace])
			{
				if (!sendPositions.Contains(position))
				{
					sendPositions.Add(position);
				}
				sendTos.Add(sendPositions.IndexOf(position), index);
			}
		}

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
	TraceProcessingPhase traceProcessingPhase,
	List<RegexToken>& tokens,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName,
	const Func<WString(vint32_t)>& ruleName,
	const Func<WString(vint32_t)>& stateLabel,
	const Func<WString(vint32_t)>& switchName
)
{
	CHECK_ERROR(tm.concurrentCount > 0, L"Cannot log failed traces!");
	Group<Trace*, WString> traceLogs;
	{
		SortedList<Trace*> logged;
		List<Trace*> visited;
		visited.Add(tm.GetInitialTrace());

		for (vint i = 0; i < visited.Count(); i++)
		{
			auto trace = visited[i];
			if (!logged.Contains(trace))
			{
				logged.Add(trace);
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
					stateLabel,
					switchName
					);

				auto successorId = trace->successors.first;
				while (successorId != -1)
				{
					auto successor = tm.GetTrace(successorId);
					visited.Add(successor);
					successorId = successor->successors.siblingNext;
				}
			}
		}
	}

	auto outputDir = GetOutputDir(parserName);
	auto outputFile = outputDir / (L"Trace-" + itow((vint)traceProcessingPhase + 1) + L"[" + caseName + L"].txt");
	auto content = GenerateToStream([&](StreamWriter& writer)
	{
		RenderTraceTree(tm.GetInitialTrace(), tm, traceLogs, writer);
	});
	File(outputFile).WriteAllText(content, true, BomEncoder::Utf8);
	return outputFile;
}