#include "LogAutomaton.h"

extern WString GetTestOutputPath();
extern FilePath GetOutputDir(const WString& parserName);

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
LogAutomaton
***********************************************************************/

FilePath LogAutomaton(
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

FilePath LogAutomaton(
	const WString& parserName,
	Executable& executable,
	Metadata& metadata,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
)
{
	return LogAutomaton(GetOutputDir(parserName) / L"Automaton.txt", executable, metadata, typeName, fieldName, tokenName);
}

/***********************************************************************
LogTrace
***********************************************************************/

void LogTraceInstruction(
	AstIns& ins,
	RegexToken& token,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName,
	StreamWriter& writer
	)
{
	writer.WriteString(L"<");
	writer.WriteString(tokenName(token.token));
	writer.WriteString(L":");
	writer.WriteString(token.reading, token.length);
	writer.WriteString(L"> ");
	LogInstruction(ins, typeName, fieldName, writer);
}

class LogTraceInsReceiver : public Object, public virtual IAstInsReceiver
{
protected:
	const Func<WString(vint32_t)>&		typeName;
	const Func<WString(vint32_t)>&		fieldName;
	const Func<WString(vint32_t)>&		tokenName;
	StreamWriter&						writer;
public:
	LogTraceInsReceiver(
		const Func<WString(vint32_t)>& _typeName,
		const Func<WString(vint32_t)>& _fieldName,
		const Func<WString(vint32_t)>& _tokenName,
		StreamWriter& _writer)
		: typeName(_typeName)
		, fieldName(_fieldName)
		, tokenName(_tokenName)
		, writer(_writer)
	{
	}

	void Execute(AstIns instruction, const regex::RegexToken& token) override
	{
		writer.WriteString(L"<");
		writer.WriteString(tokenName(token.token));
		writer.WriteString(L":");
		writer.WriteString(token.reading, token.length);
		writer.WriteString(L"> ");
		LogInstruction(instruction, typeName, fieldName, writer);
	}

	Ptr<ParsingAstBase> Finished() override
	{
		return {};
	}
};

FilePath LogTrace(
	const WString& parserName,
	const WString& caseName,
	Executable& executable,
	Metadata& metadata,
	TraceManager& tm,
	Trace* trace,
	List<RegexToken>& tokens,
	const Func<WString(vint32_t)>& typeName,
	const Func<WString(vint32_t)>& fieldName,
	const Func<WString(vint32_t)>& tokenName
)
{
	auto outputDir = GetOutputDir(parserName);
	auto outputFile = outputDir / (L"Instructions[" + caseName + L"].txt");
	FileStream fileStream(outputFile.GetFullPath(), FileStream::WriteOnly);
	BomEncoder encoder(BomEncoder::Utf8);
	EncoderStream encoderStream(fileStream, encoder);
	StreamWriter writer(encoderStream);
	LogTraceInsReceiver receiver(typeName, fieldName, tokenName, writer);
	tm.ExecuteTrace(trace, receiver, tokens);
	return outputFile;
}