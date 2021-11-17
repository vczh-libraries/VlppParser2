#include "LogParser.h"

extern WString GetExePath();

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
	List<StateSymbol*> order;
	{
		vint count = 0;
		List<StateSymbol*> visited;
		for (auto ruleName : manager.RuleOrder())
		{
			auto ruleSymbol = manager.Rules()[ruleName];
			for (auto startState : ruleSymbol->startStates)
			{
				if (!visited.Contains(startState))
				{
					vint startIndex = visited.Count();
					visited.Add(startState);
					for (vint i = startIndex; i < visited.Count(); i++)
					{
						auto state = visited[i];
						order.Add(state);
						labels.Add(state, L"[" + itow(count++) + L"] " + state->label);
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

	for (auto state : order)
	{
		writer.WriteLine(labels[state]);
	}
	writer.WriteLine(L"");
}