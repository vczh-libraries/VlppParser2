#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
SyntaxSymbolManager::BuildAutomaton
***********************************************************************/

			void SyntaxSymbolManager::BuildAutomaton(automaton::Executable& executable, automaton::Metadata& metadata)
			{
				CHECK_ERROR(global.Errors().Count() == 0, L"vl::gre::parsergen::SyntaxSymbolManager::BuildAutomaton(Executable&, Metadata&)#BuildAutomaton() cannot be called before errors are resolved.");
				CHECK_ERROR(phase == SyntaxPhase::CrossReferencedNFA, L"vl::gre::parsergen::SyntaxSymbolManager::BuildAutomaton(Executable&, Metadata&)#BuildAutomaton() can only be called on cross referenced NFA.");
			}
		}
	}
}