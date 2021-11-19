#include "LogParser.h"

extern WString GetExePath();

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

void LogSyntax(
	SyntaxSymbolManager& manager,
	const WString& parserName,
	const WString& phase,
	WString(*typeName)(vint32_t),
	WString(*fieldName)(vint32_t),
	WString(*tokenName)(vint32_t)
)
{
	auto outputDir = FilePath(GetExePath()) / L"../../Output" / parserName;
	{
		Folder folder = outputDir;
		if (!folder.Exists())
		{
			folder.Create(true);
		}
	}
	auto outputFile = outputDir / (phase + L".txt");
	FileStream fileStream(outputFile.GetFullPath(), FileStream::WriteOnly);
	BomEncoder encoder(BomEncoder::Utf8);
	EncoderStream encoderStream(fileStream, encoder);
	StreamWriter writer(encoderStream);

	Dictionary<StateSymbol*, WString> labels;
	Group<RuleSymbol*, StateSymbol*> groupedStates;
	List<StateSymbol*> order;
	{
		List<StateSymbol*> visited;
		for (auto ruleName : manager.RuleOrder())
		{
			auto ruleSymbol = manager.Rules()[ruleName];
			for (auto startState : ruleSymbol->startStates)
			{
				if (!visited.Contains(startState))
				{
					vint startIndex = visited.Add(startState);
					for (vint i = startIndex; i < visited.Count(); i++)
					{
						auto state = visited[i];
						groupedStates.Add(state->Rule(), state);
						for (auto edge : state->OutEdges())
						{
							auto target = edge->To();
							if (!visited.Contains(target))
							{
								visited.Add(target);
							}
						}
					}
				}
			}
		}
	}
	{
		vint counter = 0;
		for (auto ruleName : manager.RuleOrder())
		{
			auto ruleSymbol = manager.Rules()[ruleName];
			auto orderedStates = From(groupedStates[ruleSymbol])
				.OrderBy([](StateSymbol* s1, StateSymbol* s2)
				{
					return WString::Compare(s1->label, s2->label);
				});
			for (auto state : orderedStates)
			{
				auto label = L"[" + itow(counter++) + L"][" + ruleSymbol->Name() + L"]" + state->label + (state->endingState ? L"[ENDING]" : L"");
				order.Add(state);
				labels.Add(state, label);
			}
		}
	}

	for (auto state : order)
	{
		writer.WriteLine(labels[state]);
		auto orderedEdges = From(state->OutEdges())
			.OrderBy([&](EdgeSymbol* e1, EdgeSymbol* e2)
			{
				return order.IndexOf(e1->To()) - order.IndexOf(e2->To());
			});
		for (auto edge : state->OutEdges())
		{
			switch (edge->input.type)
			{
			case EdgeInputType::Epsilon:
				writer.WriteString(L"\tepsilon");
				break;
			case EdgeInputType::Ending:
				writer.WriteString(L"\tending");
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

		}
		writer.WriteLine(L"");
	}
}