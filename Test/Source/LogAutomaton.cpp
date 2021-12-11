#include "LogAutomaton.h"

extern WString GetTestOutputPath();
extern FilePath GetOutputDir(const WString& parserName);

/***********************************************************************
LogAutomaton
***********************************************************************/

FilePath LogAutomatonWithPath(
	const FilePath& outputFile,
	Executable& executable,
	Metadata& metadata,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
)
{
	FileStream fileStream(outputFile.GetFullPath(), FileStream::WriteOnly);
	BomEncoder encoder(BomEncoder::Utf8);
	EncoderStream encoderStream(fileStream, encoder);
	StreamWriter writer(encoderStream);

	for (auto&& [state, stateIndex] : indexed(executable.states))
	{
		writer.WriteLine(metadata.stateLabels[stateIndex]);
		writer.WriteLine(L"[ROW: " + itow(state.rule) + L"][CLAUSE: " + itow(state.clause) + L"]");
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
					writer.WriteString(L"\ttoken: " + tokenName((vint32_t)(input - Executable::TokenBegin)));
					break;
				}
				switch (edge.priority)
				{
				case EdgePriority::HighPriority:
					writer.WriteString(L" [HIGH PRIORITY]");
					break;
				case EdgePriority::LowPriority:
					writer.WriteString(L" [LOW PRIORITY]");
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
					writer.WriteString(L"\t\t> rule");
					switch (returnDesc.priority)
					{
					case EdgePriority::HighPriority:
						writer.WriteString(L" [HIGH PRIORITY]");
						break;
					case EdgePriority::LowPriority:
						writer.WriteString(L" [LOW PRIORITY]");
						break;
					}
					writer.WriteLine(L": " + metadata.ruleNames[returnDesc.consumedRule] + L" -> " + metadata.stateLabels[returnDesc.returnState]);
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

FilePath LogAutomaton(
	const WString& parserName,
	Executable& executable,
	Metadata& metadata,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
)
{
	return LogAutomatonWithPath(GetOutputDir(parserName) / L"Automaton.txt", executable, metadata, typeName, fieldName, tokenName);
}