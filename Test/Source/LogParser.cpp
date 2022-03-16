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
	const Func<WString(vint32_t)>& tokenName,
	const Func<WString(vint32_t)>& switchName
)
{
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
		List<EdgeSymbol*> orderedEdges;
		state->GetOutEdgesInStableOrder(order, orderedEdges);
		writer.WriteLine(labels[state]);
		writer.WriteLine(L"[RULE: " + itow(manager.RuleOrder().IndexOf(state->Rule()->Name())) + L"][CLAUSE: " + itow(state->ClauseId()) + L"]");
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
			case EdgeInputType::LrPlaceholder:
				writer.WriteString(L"\tlr-placeholder: " + manager.lrpFlags[edge->input.token]);
				break;
			case EdgeInputType::LrInject:
				writer.WriteString(L"\tlr-inject: " + edge->input.rule->Name());
				break;
			}
			writer.WriteLine(L" -> " + labels[edge->To()]);

			for (auto&& ins : edge->insSwitch)
			{
				writer.WriteString(L"\t\t? ");
				LogInstruction(ins, switchName, writer);
			}

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

FilePath LogSyntax(
	SyntaxSymbolManager& manager,
	const WString& parserName,
	const WString& phase,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName,
	const Func<WString(vint32_t)>& switchName
)
{
	auto outputDir = GetOutputDir(parserName);
	auto outputFile = outputDir / (phase + L".txt");
	return LogSyntaxWithPath(manager, outputFile, typeName, fieldName, tokenName, switchName);
}