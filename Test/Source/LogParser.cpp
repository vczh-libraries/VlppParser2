#include "LogParser.h"

extern WString GetTestOutputPath();

FilePath GetOutputDir(const WString& parserName)
{
	auto outputDir = FilePath(GetTestOutputPath()) / parserName;
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
LogSyntax
***********************************************************************/

FilePath LogSyntax(
	SyntaxSymbolManager& manager,
	const WString& parserName,
	const WString& phase,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
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
		labels.Add(state, manager.GetStateGlobalLabel(state, index));
	}

	for (auto state : order)
	{
		writer.WriteLine(labels[state]);
		auto orderedEdges = From(state->OutEdges())
			.OrderBy([&](EdgeSymbol* e1, EdgeSymbol* e2)
			{
				vint result = 0;
				if (e1->input.type != e2->input.type)
				{
					result = (vint)e1->input.type - (vint)e2->input.type;
				}
				else
				{
					switch (e1->input.type)
					{
					case EdgeInputType::Token:
						result = e1->input.token - e2->input.token;
						break;
					case EdgeInputType::Rule:
						result = manager.RuleOrder().IndexOf(e1->input.rule->Name()) - manager.RuleOrder().IndexOf(e2->input.rule->Name());
						break;
					default:;
					}
				}

				if (result != 0) return result;
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
				if (manager.Phase() == SyntaxPhase::CrossReferencedNFA)
				{
					continue;
				}
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
				for (auto&& ins : returnEdge->insAfterInput)
				{
					writer.WriteString(L"\t\t\t+ ");
					LogInstruction(ins, typeName, fieldName, writer);
				}
			}

		}
		writer.WriteLine(L"");
	}
	return outputFile;
}

/***********************************************************************
LogAutomaton
***********************************************************************/

FilePath LogAutomaton(
	const WString& parserName,
	Executable& executable,
	Metadata& metadata,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
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
					writer.WriteString(L"\ttoken: " + tokenName(input - Executable::TokenBegin));
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
					writer.WriteString(L"\t\t+ ");
					LogInstruction(executable.instructions[edge.insAfterInput.start + insRef], typeName, fieldName, writer);
				}

				for (vint returnRef = 0; returnRef < edge.returnIndices.count; returnRef++)
				{
					auto&& returnDesc = executable.returns[executable.returnIndices[edge.returnIndices.start + returnRef]];
					writer.WriteLine(L"\t\t> rule: " + metadata.ruleNames[returnDesc.consumedRule] + L" -> " + metadata.stateLabels[returnDesc.returnState]);
					for (vint insRef = 0; insRef < returnDesc.insAfterInput.count; insRef++)
					{
						writer.WriteString(L"\t\t\t+ ");
						LogInstruction(executable.instructions[returnDesc.insAfterInput.start + insRef], typeName, fieldName, writer);
					}
				}
			}
		}
		writer.WriteLine(L"");
	}
	return outputFile;
}