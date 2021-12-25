/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_ASTPRINT
#define VCZH_PARSER2_ASTPRINT

#include "AstBase.h"

namespace vl
{
	namespace glr
	{
/***********************************************************************
IParsingPrintNodeRecorder
***********************************************************************/

		class IParsingPrintNodeRecorder : public virtual Interface
		{
		public:
			virtual void						Record(ParsingAstBase* node, const ParsingTextRange& range) = 0;
		};

		class ParsingEmptyPrintNodeRecorder : public Object, public virtual IParsingPrintNodeRecorder
		{
		public:
			ParsingEmptyPrintNodeRecorder();
			~ParsingEmptyPrintNodeRecorder();

			void								Record(ParsingAstBase* node, const ParsingTextRange& range)override;
		};

		class ParsingMultiplePrintNodeRecorder : public Object, public virtual IParsingPrintNodeRecorder
		{
			typedef collections::List<Ptr<IParsingPrintNodeRecorder>>				RecorderList;
		protected:
			RecorderList						recorders;

		public:
			ParsingMultiplePrintNodeRecorder();
			~ParsingMultiplePrintNodeRecorder();

			void								AddRecorder(Ptr<IParsingPrintNodeRecorder> recorder);
			void								Record(ParsingAstBase* node, const ParsingTextRange& range)override;
		};

		class ParsingOriginalLocationRecorder : public Object, public virtual IParsingPrintNodeRecorder
		{
		protected:
			Ptr<IParsingPrintNodeRecorder>		recorder;

		public:
			ParsingOriginalLocationRecorder(Ptr<IParsingPrintNodeRecorder> _recorder);
			~ParsingOriginalLocationRecorder();

			void								Record(ParsingAstBase* node, const ParsingTextRange& range)override;
		};

		class ParsingGeneratedLocationRecorder : public Object, public virtual IParsingPrintNodeRecorder
		{
			typedef collections::Dictionary<ParsingAstBase*, ParsingTextRange>		RangeMap;
		protected:
			RangeMap&							rangeMap;

		public:
			ParsingGeneratedLocationRecorder(RangeMap& _rangeMap);
			~ParsingGeneratedLocationRecorder();

			void								Record(ParsingAstBase* node, const ParsingTextRange& range)override;
		};

		class ParsingUpdateLocationRecorder : public Object, public virtual IParsingPrintNodeRecorder
		{
		public:
			ParsingUpdateLocationRecorder();
			~ParsingUpdateLocationRecorder();

			void								Record(ParsingAstBase* node, const ParsingTextRange& range)override;
		};

/***********************************************************************
ParsingWriter
***********************************************************************/

		class ParsingWriter : public stream::TextWriter
		{
			typedef collections::Pair<ParsingAstBase*, ParsingTextPos>				NodePosPair;
			typedef collections::List<NodePosPair>											NodePosList;
		protected:
			stream::TextWriter&					writer;
			Ptr<IParsingPrintNodeRecorder>		recorder;
			vint								codeIndex;
			ParsingTextPos						lastPos;
			ParsingTextPos						currentPos;
			NodePosList							nodePositions;

			void								HandleChar(wchar_t c);
		public:
			ParsingWriter(stream::TextWriter& _writer, Ptr<IParsingPrintNodeRecorder> _recorder = nullptr, vint _codeIndex = -1);
			~ParsingWriter();

			using stream::TextWriter::WriteString;
			void								WriteChar(wchar_t c)override;
			void								WriteString(const wchar_t* string, vint charCount)override;
			void								BeforePrint(ParsingAstBase* node);
			void								AfterPrint(ParsingAstBase* node);
		};
	}
}

#endif