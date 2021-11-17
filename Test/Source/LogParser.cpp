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

	for (auto ruleName : manager.RuleOrder())
	{
		auto ruleSymbol = manager.Rules()[ruleName];
	}
}