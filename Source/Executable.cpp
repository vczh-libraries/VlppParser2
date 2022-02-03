#include "Executable.h"

namespace vl
{
	namespace stream
	{
		namespace internal
		{
			using namespace glr;
			using namespace glr::automaton;

			SERIALIZE_ENUM(AstInsType)
			SERIALIZE_ENUM(SwitchInsType)
			SERIALIZE_ENUM(EdgePriority)
			SERIALIZE_ENUM(ReturnRuleType)

			BEGIN_SERIALIZATION(AstIns)
				SERIALIZE(type)
				SERIALIZE(param)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(SwitchIns)
				SERIALIZE(type)
				SERIALIZE(param)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(InstructionArray)
				SERIALIZE(start)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(StringLiteral)
				SERIALIZE(start)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(ReturnIndexArray)
				SERIALIZE(start)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(EdgeArray)
				SERIALIZE(start)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(ReturnDesc)
				SERIALIZE(consumedRule)
				SERIALIZE(returnState)
				SERIALIZE(priority)
				SERIALIZE(ruleType)
				SERIALIZE(insAfterInput)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(EdgeDesc)
				SERIALIZE(fromState)
				SERIALIZE(toState)
				SERIALIZE(condition)
				SERIALIZE(priority)
				SERIALIZE(insSwitch)
				SERIALIZE(insBeforeInput)
				SERIALIZE(insAfterInput)
				SERIALIZE(returnIndices)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(StateDesc)
				SERIALIZE(rule)
				SERIALIZE(clause)
				SERIALIZE(endingState)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(Executable)
				SERIALIZE(tokenCount)
				SERIALIZE(ruleCount)
				SERIALIZE(ruleStartStates)
				SERIALIZE(transitions)
				SERIALIZE(astInstructions)
				SERIALIZE(switchInstructions)
				SERIALIZE(returnIndices)
				SERIALIZE(returns)
				SERIALIZE(edges)
				SERIALIZE(states)
				SERIALIZE(stringLiteralBuffer)
				SERIALIZE(switchDefaultValues)
			END_SERIALIZATION
		}
	}

	namespace glr
	{
		namespace automaton
		{
			using namespace stream;

/***********************************************************************
Executable
***********************************************************************/

			Executable::Executable(stream::IStream& inputStream)
			{
				internal::ContextFreeReader reader(inputStream);
				reader << *this;
			}

			void Executable::Serialize(stream::IStream& outputStream)
			{
				internal::ContextFreeWriter writer(outputStream);
				writer << *this;
			}
		}
	}
}