#include "LogParser.h"

extern WString GetExePath();

void LogSyntax(SyntaxSymbolManager& manager, const WString& parserName, const WString& phase)
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
}