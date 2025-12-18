#include "LogParser.h"

extern WString GetTestOutputPath();
extern FilePath GetOutputDir(const WString& parserName);

/***********************************************************************
LogSyntax
***********************************************************************/

FilePath LogSyntaxWithPath(
	SyntaxSymbolManager& manager,
	const FilePath& outputFile,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
)
{
	FileStream fileStream(outputFile.GetFullPath(), FileStream::WriteOnly);
	BomEncoder encoder(BomEncoder::Utf8);
	EncoderStream encoderStream(fileStream, encoder);
	StreamWriter writer(encoderStream);

	if (manager.prefixMergeSolutions.Count() > 0)
	{
		writer.WriteLine(L"[PREFIX MERGE SOLUTIONS]");
		for (auto key : From(manager.prefixMergeSolutions.Keys())
			.OrderByKey([](auto k) {return Tuple(k.get<0>()->Name(), k.get<1>()->label); })
			)
		{
			auto value = manager.prefixMergeSolutions[key];
			writer.WriteLine(L"  " + key.get<0>()->Name() + L": " + key.get<1>()->label);
			writer.WriteString(L"    prefixes: ");
			for (auto [rule, index] : indexed(From(value->prefixRules).OrderByKey([](auto k) { return k->Name(); })))
			{
				if (index > 0) writer.WriteString(L", ");
				writer.WriteString(rule->Name());
			}
			writer.WriteLine(L"");
		}
		writer.WriteLine(L"");

		writer.WriteLine(L"[PREFIX MERGE APPLICATIONS]");
		for (auto key : From(manager.prefixMergeSolutions.Keys())
			.OrderByKey([](auto k) {return Tuple(k.get<0>()->Name(), k.get<1>()->label); })
			)
		{
			auto value = manager.prefixMergeSolutions[key];
			if (value->applications.Count() == 0) continue;

			writer.WriteLine(L"  " + key.get<0>()->Name() + L": " + key.get<1>()->label);
			for (auto application : From(value->applications)
				.OrderByKey([](auto a)
				{
					return From(a->edgesToMerge)
						.Select([](auto e) { return e->input.rule->Name(); })
						.OrderBySelf()
						.First();
				}))
			{
				writer.WriteString(L"    [applies: ");
				for (auto [rule, index] : indexed(From(application->prefixRules).OrderByKey([](auto k) { return k->Name(); })))
				{
					if (index > 0) writer.WriteString(L", ");
					writer.WriteString(rule->Name());
				}
				writer.WriteString(L"] [on: ");
				for (auto [edge, index] : indexed(From(application->edgesToMerge).OrderByKey([](auto k) { return k->input.rule->Name(); })))
				{
					if (index > 0) writer.WriteString(L", ");
					writer.WriteString(edge->input.rule->Name());
				}
				writer.WriteLine(L"]");
			}
		}
		writer.WriteLine(L"");
	}

	Dictionary<StateSymbol*, WString> labels;
	List<StateSymbol*> order;
	manager.GetStatesInStableOrder(order);
	for (auto [state, index] : indexed(order))
	{
		labels.Add(state, manager.GetStateGlobalLabel(state, index));
	}

	for (auto state : order)
	{
		List<EdgeSymbol*> orderedEdges;
		state->GetOutEdgesInStableOrder(order, orderedEdges);
		writer.WriteLine(labels[state]);
		writer.WriteLine(L"[RULE: " + itow(manager.RuleOrder().IndexOf(state->Rule()->Name())) + L"]");
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
				if (edge->input.condition)
				{
					writer.WriteString(L"=\"" + edge->input.condition.Value() + L"\"");
				}
				break;
			case EdgeInputType::Rule:
				if (manager.Phase() == SyntaxPhase::CrossReferencedNFA)
				{
					continue;
				}
				writer.WriteString(L"\trule: " + edge->input.rule->Name());
				break;
			default:;
			}

			for (auto comp : edge->competitions)
			{
				writer.WriteChar(L'[');
				writer.WriteChar(comp.highPriority ? L'H' : L'L');
				writer.WriteString(itow(comp.competitionId));
				writer.WriteChar(L']');
			}
			writer.WriteLine(L" -> " + labels[edge->To()]);

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
	return LogSyntaxWithPath(manager, outputFile, typeName, fieldName, tokenName);
}