#include "AstPrint.h"

namespace vl
{
	namespace glr
	{

/***********************************************************************
ParsingEmptyPrintNodeRecorder
***********************************************************************/

		ParsingEmptyPrintNodeRecorder::ParsingEmptyPrintNodeRecorder()
		{
		}

		ParsingEmptyPrintNodeRecorder::~ParsingEmptyPrintNodeRecorder()
		{
		}

		void ParsingEmptyPrintNodeRecorder::Record(ParsingAstBase* node, const ParsingTextRange& range)
		{
		}

/***********************************************************************
ParsingMultiplePrintNodeRecorder
***********************************************************************/

		ParsingMultiplePrintNodeRecorder::ParsingMultiplePrintNodeRecorder()
		{
		}

		ParsingMultiplePrintNodeRecorder::~ParsingMultiplePrintNodeRecorder()
		{
		}

		void ParsingMultiplePrintNodeRecorder::AddRecorder(Ptr<IParsingPrintNodeRecorder> recorder)
		{
			recorders.Add(recorder);
		}

		void ParsingMultiplePrintNodeRecorder::Record(ParsingAstBase* node, const ParsingTextRange& range)
		{
			for (auto recorder : recorders)
			{
				recorder->Record(node, range);
			}
		}

/***********************************************************************
ParsingOriginalLocationRecorder
***********************************************************************/

		ParsingOriginalLocationRecorder::ParsingOriginalLocationRecorder(Ptr<IParsingPrintNodeRecorder> _recorder)
			:recorder(_recorder)
		{
		}

		ParsingOriginalLocationRecorder::~ParsingOriginalLocationRecorder()
		{
		}

		void ParsingOriginalLocationRecorder::Record(ParsingAstBase* node, const ParsingTextRange& range)
		{
			auto codeRange = node->codeRange;
			codeRange.codeIndex = range.codeIndex;
			recorder->Record(node, codeRange);
		}

/***********************************************************************
ParsingGeneratedLocationRecorder
***********************************************************************/

		ParsingGeneratedLocationRecorder::ParsingGeneratedLocationRecorder(RangeMap& _rangeMap)
			:rangeMap(_rangeMap)
		{
		}

		ParsingGeneratedLocationRecorder::~ParsingGeneratedLocationRecorder()
		{
		}

		void ParsingGeneratedLocationRecorder::Record(ParsingAstBase* node, const ParsingTextRange& range)
		{
			rangeMap.Add(node, range);
		}

/***********************************************************************
ParsingUpdateLocationRecorder
***********************************************************************/

		ParsingUpdateLocationRecorder::ParsingUpdateLocationRecorder()
		{
		}

		ParsingUpdateLocationRecorder::~ParsingUpdateLocationRecorder()
		{
		}

		void ParsingUpdateLocationRecorder::Record(ParsingAstBase* node, const ParsingTextRange& range)
		{
			node->codeRange = range;
		}

/***********************************************************************
ParsingWriter
***********************************************************************/

		void ParsingWriter::HandleChar(wchar_t c)
		{
			lastPos = currentPos;
			switch (c)
			{
			case L'\n':
				currentPos.index++;
				currentPos.row++;
				currentPos.column = 0;
				break;
			default:
				currentPos.index++;
				currentPos.column++;
			}
		}

		ParsingWriter::ParsingWriter(stream::TextWriter& _writer, Ptr<IParsingPrintNodeRecorder> _recorder, vint _codeIndex)
			:writer(_writer)
			, recorder(_recorder)
			, codeIndex(_codeIndex)
			, lastPos(-1, 0, -1)
			, currentPos(0, 0, 0)
		{
		}

		ParsingWriter::~ParsingWriter()
		{
		}

		void ParsingWriter::WriteChar(wchar_t c)
		{
			writer.WriteChar(c);
			if (!recorder) return;
			HandleChar(c);
		}

		void ParsingWriter::WriteString(const wchar_t* string, vint charCount)
		{
			writer.WriteString(string, charCount);
			if (!recorder) return;
			for (vint i = 0; i < charCount; i++)
			{
				HandleChar(string[i]);
			}
		}

		void ParsingWriter::BeforePrint(ParsingAstBase* node)
		{
			if (!recorder) return;
			nodePositions.Add(NodePosPair(node, currentPos));
		}

		void ParsingWriter::AfterPrint(ParsingAstBase* node)
		{
			if (!recorder) return;

			auto pair = nodePositions[nodePositions.Count() - 1];
			nodePositions.RemoveAt(nodePositions.Count() - 1);
			CHECK_ERROR(pair.key == node, L"vl::parsing::ParsingWriter::AfterPrint(ParsingTreeNode*)#BeforePrint and AfterPrint should be call in pairs.");

			ParsingTextRange range(pair.value, lastPos, codeIndex);
			recorder->Record(node, range);
		}
	}
}