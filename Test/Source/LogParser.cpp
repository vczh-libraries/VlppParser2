#include "LogParser.h"

extern WString GetExePath();

FilePath GetOutputDir(const WString& parserName)
{
	auto outputDir = FilePath(GetExePath()) / L"../../Output" / parserName;
	{
		Folder folder = outputDir;
		if (!folder.Exists())
		{
			folder.Create(true);
		}
	}
	return outputDir;
}

/***********************************************************************
LogInstruction
***********************************************************************/

void LogInstruction(
	AstIns ins,
	WString(*typeName)(vint32_t),
	WString(*fieldName)(vint32_t),
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
LogSyntax
***********************************************************************/

void LogSyntax(
	SyntaxSymbolManager& manager,
	const WString& parserName,
	const WString& phase,
	WString(*typeName)(vint32_t),
	WString(*fieldName)(vint32_t),
	WString(*tokenName)(vint32_t)
)
{
	auto outputDir = GetOutputDir(parserName);
	auto outputFile = outputDir / (phase + L".txt");
	FileStream fileStream(outputFile.GetFullPath(), FileStream::WriteOnly);
	BomEncoder encoder(BomEncoder::Utf8);
	EncoderStream encoderStream(fileStream, encoder);
	StreamWriter writer(encoderStream);

	Dictionary<StateSymbol*, WString> labels;
	List<StateSymbol*> order;
	manager.GetStatesInStableOrder(order);
	for (auto [state, index] : indexed(order))
	{
		auto label = L"[" + itow(index) + L"][" + state->Rule()->Name() + L"]" + state->label + (state->endingState ? L"[ENDING]" : L"");
		labels.Add(state, label);
	}

	for (auto state : order)
	{
		writer.WriteLine(labels[state]);
		auto orderedEdges = From(state->OutEdges())
			.OrderBy([&](EdgeSymbol* e1, EdgeSymbol* e2)
			{
				return order.IndexOf(e1->To()) - order.IndexOf(e2->To());
			});
		for (auto edge : orderedEdges)
		{
			switch (edge->input.type)
			{
			case EdgeInputType::Epsilon:
				writer.WriteString(L"\tepsilon");
				break;
			case EdgeInputType::Ending:
				writer.WriteString(L"\tending");
				break;
			case EdgeInputType::LeftRec :
				writer.WriteString(L"\tleftrec");
				break;
			case EdgeInputType::Token:
				writer.WriteString(L"\ttoken: " + tokenName(edge->input.token));
				break;
			case EdgeInputType::Rule:
				writer.WriteString(L"\trule: " + edge->input.rule->Name());
				break;
			}
			writer.WriteLine(L" -> " + labels[edge->To()]);

			for (auto&& ins : edge->insBeforeInput)
			{
				writer.WriteString(L"\t\t- ");
				LogInstruction(ins, typeName, fieldName, writer);
			}

			for (auto&& ins : edge->insAfterInput)
			{
				writer.WriteString(L"\t\t+ ");
				LogInstruction(ins, typeName, fieldName, writer);
			}

			for (auto returnEdge : edge->returnEdges)
			{
				writer.WriteLine(L"\t\t> rule: " + returnEdge->input.rule->Name() + L" -> " + labels[returnEdge->To()]);
			}

		}
		writer.WriteLine(L"");
	}
}

/***********************************************************************
LogAutomaton
***********************************************************************/

void LogAutomaton(
	const WString& parserName,
	Executable& executable,
	Metadata& metadata,
	WString(*typeName)(vint32_t),
	WString(*fieldName)(vint32_t),
	WString(*tokenName)(vint32_t)
)
{
	auto outputDir = GetOutputDir(parserName);
	auto outputFile = outputDir / L"Automaton.txt";
	FileStream fileStream(outputFile.GetFullPath(), FileStream::WriteOnly);
	BomEncoder encoder(BomEncoder::Utf8);
	EncoderStream encoderStream(fileStream, encoder);
	StreamWriter writer(encoderStream);

	for (auto&& [state, stateIndex] : indexed(executable.states))
	{
		writer.WriteLine(metadata.stateLabels[stateIndex]);
		for (vint input = 0; input < Executable::TokenBegin + executable.tokenCount; input++)
		{
			auto&& transition = executable.transitions[stateIndex * (Executable::TokenBegin + executable.tokenCount) + input];
			for (vint edgeRef = 0; edgeRef < transition.count; edgeRef++)
			{
				auto&& edge = executable.edges[transition.start + edgeRef];
				switch (input)
				{
				case Executable::EndingInput:
					writer.WriteString(L"\tending");
					break;
				case Executable::LeftrecInput:
					writer.WriteString(L"\tleftrec");
					break;
				default:
					writer.WriteString(L"\ttoken:" + tokenName(input - Executable::TokenBegin));
					break;
				}
				writer.WriteLine(L" -> " + metadata.stateLabels[edge.toState]);

				for (vint insRef = 0; insRef < edge.insBeforeInput.count; insRef++)
				{
					writer.WriteString(L"\t\t- ");
					LogInstruction(executable.instructions[edge.insBeforeInput.start + insRef], typeName, fieldName, writer);
				}

				for (vint insRef = 0; insRef < edge.insAfterInput.count; insRef++)
				{
					writer.WriteString(L"\t\t- ");
					LogInstruction(executable.instructions[edge.insAfterInput.start + insRef], typeName, fieldName, writer);
				}

				for (vint returnRef = 0; returnRef < edge.returns.count; returnRef++)
				{
					auto&& returnDesc = executable.returns[edge.returns.start + returnRef];
					writer.WriteLine(L"\t\t> rule: " + metadata.ruleNames[executable.states[returnDesc.returnState].rule] + L" -> " + metadata.stateLabels[returnDesc.returnState]);
					for (vint insRef = 0; insRef < returnDesc.insAfterInput.count; insRef++)
					{
						writer.WriteString(L"\t\t- ");
						LogInstruction(executable.instructions[returnDesc.insAfterInput.start + insRef], typeName, fieldName, writer);
					}
				}
			}
		}
		writer.WriteLine(L"");
	}
}