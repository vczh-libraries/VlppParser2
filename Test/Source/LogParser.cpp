#include "LogParser.h"

extern WString GetTestOutputPath();
extern FilePath GetOutputDir(const WString& parserName);

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