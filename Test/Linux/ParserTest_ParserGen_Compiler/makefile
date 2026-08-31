.PHONY: all clean pre-build
.DEFAULT_GOAL := all

CPP_COMPILE_OPTIONS=-I ../../../Import
include $(VCPROOT)/vl/makefile-cpp

pre-build:
	if ! [ -d ./Bin ]; then mkdir ./Bin; fi
	if ! [ -d ./Obj ]; then mkdir ./Obj; fi
	if ! [ -d ./Coverage ]; then mkdir ./Coverage; fi
	if ! [ -d ../../Output ]; then mkdir ../../Output; fi

clean:
	if [ -d ./Bin ]; then rm -r ./Bin; fi
	if [ -d ./Obj ]; then rm -r ./Obj; fi
	if [ -d ./Coverage ]; then rm -r ./Coverage; fi
	if [ -d ./../../Output ]; then rm -r ../../Output; fi

all:pre-build ./Bin/UnitTest

./Bin/UnitTest:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppReflection.o ./Obj/VlppRegex.o ./Obj/AstBase.o ./Obj/AstPrint.o ./Obj/AstCppGen.o ./Obj/AstCppGen_Assembler.o ./Obj/AstCppGen_Builder.o ./Obj/AstCppGen_Classes.o ./Obj/AstCppGen_CopyVisitor.o ./Obj/AstCppGen_EmptyVisitor.o ./Obj/AstCppGen_JsonVisitor.o ./Obj/AstCppGen_TraverseVisitor.o ./Obj/AstSymbol.o ./Obj/AstSymbol_CreateParserGenRuleAst.o ./Obj/AstSymbol_CreateParserGenTypeAst.o ./Obj/LexerCppGen.o ./Obj/LexerSymbol.o ./Obj/LexerSymbol_CreateParserGenLexer.o ./Obj/Executable.o ./Obj/SyntaxBase.o ./Obj/TmPtr_AllocateExecutionData.o ./Obj/TmPtr_BuildAmbiguityStructures.o ./Obj/TmPtr_PartialExecuteTraces.o ./Obj/TmPtr_PartialExecuteTraces_SummarizeInstructionRange.o ./Obj/TmPtr_PartialExecuteTraces_EnsureInsExecContextCompatible.o ./Obj/TmPtr_PartialExecuteTraces_MergeInsExecContext.o ./Obj/TmPtr_PartialExecuteTraces_PartialExecuteOrdinaryTrace.o ./Obj/TmRa.o ./Obj/TmRa_BuildExecutionOrder.o ./Obj/TmRa_CheckMergeTraces.o ./Obj/TraceManager.o ./Obj/TraceManager_Common.o ./Obj/TraceManager_ExecuteTrace.o ./Obj/TmInput.o ./Obj/TmInput_Ambiguity.o ./Obj/TmInput_Competition.o ./Obj/TmInput_ReturnStack.o ./Obj/TmInput_Walk.o ./Obj/TmPtr.o ./Obj/SyntaxCppGen.o ./Obj/SyntaxSymbol.o ./Obj/SyntaxSymbolWriter.o ./Obj/SyntaxSymbol_Automaton.o ./Obj/SyntaxSymbol_CreateParserGenRuleSyntax.o ./Obj/SyntaxSymbol_CreateParserGenTypeSyntax.o ./Obj/SyntaxSymbol_CreateParserGenUtility.o ./Obj/SyntaxSymbol_NFACompact.o ./Obj/SyntaxSymbol_NFACompact_EliminateEpsilonEdges.o ./Obj/SyntaxSymbol_NFACompact_EliminateLeftRecursion.o ./Obj/SyntaxSymbol_NFACompact_MergeEdgesWithSameInput.o ./Obj/SyntaxSymbol_NFACompact_PrefixMergeCrossReference.o ./Obj/SyntaxSymbol_NFACompact_MergeEdgesWithSameRuleUsingLeftrec.o ./Obj/SyntaxSymbol_NFACrossReferenced.o ./Obj/ParserCppGen.o ./Obj/ParserSymbol.o ./Obj/ParserGenRuleAst.o ./Obj/ParserGenRuleAst_Builder.o ./Obj/ParserGenRuleAst_Copy.o ./Obj/ParserGenRuleAst_Empty.o ./Obj/ParserGenRuleAst_Json.o ./Obj/ParserGenRuleAst_Traverse.o ./Obj/ParserGenRuleParser.o ./Obj/ParserGenTypeAst.o ./Obj/ParserGenTypeAst_Builder.o ./Obj/ParserGenTypeAst_Copy.o ./Obj/ParserGenTypeAst_Empty.o ./Obj/ParserGenTypeAst_Json.o ./Obj/ParserGenTypeAst_Traverse.o ./Obj/ParserGenTypeParser.o ./Obj/ParserGen_Assembler.o ./Obj/ParserGen_Lexer.o ./Obj/CompileAst.o ./Obj/CompileLexer.o ./Obj/CompileSyntax.o ./Obj/CompileSyntax_CalculateFirstSet.o ./Obj/CompileSyntax_CalculateTypes.o ./Obj/CompileSyntax_CompileSyntax.o ./Obj/CompileSyntax_ResolveName.o ./Obj/CompileSyntax_RewriteSyntax_Switch.o ./Obj/CompileSyntax_ValidateStructure.o ./Obj/CompileSyntax_ValidateSwitchesAndConditions.o ./Obj/CompileSyntax_ValidateTypes.o ./Obj/SyntaxAstToCode.o ./Obj/TypeAstToCode.o ./Obj/TypeSymbolToAst.o ./Obj/JsonAst.o ./Obj/JsonAst_Builder.o ./Obj/JsonAst_Copy.o ./Obj/JsonAst_Empty.o ./Obj/JsonAst_Json.o ./Obj/JsonAst_Traverse.o ./Obj/JsonParser.o ./Obj/Json_Assembler.o ./Obj/Json_Lexer.o ./Obj/GlrJson.o ./Obj/CalculatorExprAst.o ./Obj/CalculatorExprAst_Builder.o ./Obj/CalculatorExprAst_Copy.o ./Obj/CalculatorExprAst_Empty.o ./Obj/CalculatorExprAst_Json.o ./Obj/CalculatorExprAst_Traverse.o ./Obj/Calculator_Assembler.o ./Obj/Calculator_Lexer.o ./Obj/CalculatorModuleParser.o ./Obj/LogAutomaton.o ./Obj/LogParser.o ./Obj/LogTrace.o ./Obj/CommonMain.o ./Obj/TestParserGen.o ./Obj/TestSyntax.o ./Obj/TestError.o ./Obj/TestError_Ast.o ./Obj/TestError_CalculatorAstAndLexer.o ./Obj/TestError_Condition_ResolveName.o ./Obj/TestError_Condition_RewriteSyntax.o ./Obj/TestError_Condition_ValidateSwitches.o ./Obj/TestError_Lexer.o ./Obj/TestError_Syntax_Automaton.o ./Obj/TestError_Syntax_CalculateTypes.o ./Obj/TestError_Syntax_ResolveName.o ./Obj/TestError_Syntax_ValidateStructure.o ./Obj/TestError_Syntax_ValidateTypes.o ./Obj/TestRewrite_Condition.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../../Import/Vlpp.cpp
	$(CPP_COMPILE)

./Obj/Vlpp.Linux.o: ../../../Import/Vlpp.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../../Import/VlppOS.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.Linux.o: ../../../Import/VlppOS.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppReflection.o: ../../../Import/VlppReflection.cpp
	$(CPP_COMPILE)

./Obj/VlppRegex.o: ../../../Import/VlppRegex.cpp
	$(CPP_COMPILE)

./Obj/AstBase.o: ../../../Source/AstBase.cpp
	$(CPP_COMPILE)

./Obj/AstPrint.o: ../../../Source/AstPrint.cpp
	$(CPP_COMPILE)

./Obj/AstCppGen.o: ../../../Source/Ast/AstCppGen.cpp
	$(CPP_COMPILE)

./Obj/AstCppGen_Assembler.o: ../../../Source/Ast/AstCppGen_Assembler.cpp
	$(CPP_COMPILE)

./Obj/AstCppGen_Builder.o: ../../../Source/Ast/AstCppGen_Builder.cpp
	$(CPP_COMPILE)

./Obj/AstCppGen_Classes.o: ../../../Source/Ast/AstCppGen_Classes.cpp
	$(CPP_COMPILE)

./Obj/AstCppGen_CopyVisitor.o: ../../../Source/Ast/AstCppGen_CopyVisitor.cpp
	$(CPP_COMPILE)

./Obj/AstCppGen_EmptyVisitor.o: ../../../Source/Ast/AstCppGen_EmptyVisitor.cpp
	$(CPP_COMPILE)

./Obj/AstCppGen_JsonVisitor.o: ../../../Source/Ast/AstCppGen_JsonVisitor.cpp
	$(CPP_COMPILE)

./Obj/AstCppGen_TraverseVisitor.o: ../../../Source/Ast/AstCppGen_TraverseVisitor.cpp
	$(CPP_COMPILE)

./Obj/AstSymbol.o: ../../../Source/Ast/AstSymbol.cpp
	$(CPP_COMPILE)

./Obj/AstSymbol_CreateParserGenRuleAst.o: ../../../Source/Ast/AstSymbol_CreateParserGenRuleAst.cpp
	$(CPP_COMPILE)

./Obj/AstSymbol_CreateParserGenTypeAst.o: ../../../Source/Ast/AstSymbol_CreateParserGenTypeAst.cpp
	$(CPP_COMPILE)

./Obj/LexerCppGen.o: ../../../Source/Lexer/LexerCppGen.cpp
	$(CPP_COMPILE)

./Obj/LexerSymbol.o: ../../../Source/Lexer/LexerSymbol.cpp
	$(CPP_COMPILE)

./Obj/LexerSymbol_CreateParserGenLexer.o: ../../../Source/Lexer/LexerSymbol_CreateParserGenLexer.cpp
	$(CPP_COMPILE)

./Obj/Executable.o: ../../../Source/Executable.cpp
	$(CPP_COMPILE)

./Obj/SyntaxBase.o: ../../../Source/SyntaxBase.cpp
	$(CPP_COMPILE)

./Obj/TmPtr_AllocateExecutionData.o: ../../../Source/TraceManager/TmPtr_AllocateExecutionData.cpp
	$(CPP_COMPILE)

./Obj/TmPtr_BuildAmbiguityStructures.o: ../../../Source/TraceManager/TmPtr_BuildAmbiguityStructures.cpp
	$(CPP_COMPILE)

./Obj/TmPtr_PartialExecuteTraces.o: ../../../Source/TraceManager/TmPtr_PartialExecuteTraces.cpp
	$(CPP_COMPILE)

./Obj/TmPtr_PartialExecuteTraces_SummarizeInstructionRange.o: ../../../Source/TraceManager/TmPtr_PartialExecuteTraces_SummarizeInstructionRange.cpp
	$(CPP_COMPILE)

./Obj/TmPtr_PartialExecuteTraces_EnsureInsExecContextCompatible.o: ../../../Source/TraceManager/TmPtr_PartialExecuteTraces_EnsureInsExecContextCompatible.cpp
	$(CPP_COMPILE)

./Obj/TmPtr_PartialExecuteTraces_MergeInsExecContext.o: ../../../Source/TraceManager/TmPtr_PartialExecuteTraces_MergeInsExecContext.cpp
	$(CPP_COMPILE)

./Obj/TmPtr_PartialExecuteTraces_PartialExecuteOrdinaryTrace.o: ../../../Source/TraceManager/TmPtr_PartialExecuteTraces_PartialExecuteOrdinaryTrace.cpp
	$(CPP_COMPILE)

./Obj/TmRa.o: ../../../Source/TraceManager/TmRa.cpp
	$(CPP_COMPILE)

./Obj/TmRa_BuildExecutionOrder.o: ../../../Source/TraceManager/TmRa_BuildExecutionOrder.cpp
	$(CPP_COMPILE)

./Obj/TmRa_CheckMergeTraces.o: ../../../Source/TraceManager/TmRa_CheckMergeTraces.cpp
	$(CPP_COMPILE)

./Obj/TraceManager.o: ../../../Source/TraceManager/TraceManager.cpp
	$(CPP_COMPILE)

./Obj/TraceManager_Common.o: ../../../Source/TraceManager/TraceManager_Common.cpp
	$(CPP_COMPILE)

./Obj/TraceManager_ExecuteTrace.o: ../../../Source/TraceManager/TraceManager_ExecuteTrace.cpp
	$(CPP_COMPILE)

./Obj/TmInput.o: ../../../Source/TraceManager/TmInput.cpp
	$(CPP_COMPILE)

./Obj/TmInput_Ambiguity.o: ../../../Source/TraceManager/TmInput_Ambiguity.cpp
	$(CPP_COMPILE)

./Obj/TmInput_Competition.o: ../../../Source/TraceManager/TmInput_Competition.cpp
	$(CPP_COMPILE)

./Obj/TmInput_ReturnStack.o: ../../../Source/TraceManager/TmInput_ReturnStack.cpp
	$(CPP_COMPILE)

./Obj/TmInput_Walk.o: ../../../Source/TraceManager/TmInput_Walk.cpp
	$(CPP_COMPILE)

./Obj/TmPtr.o: ../../../Source/TraceManager/TmPtr.cpp
	$(CPP_COMPILE)

./Obj/SyntaxCppGen.o: ../../../Source/Syntax/SyntaxCppGen.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol.o: ../../../Source/Syntax/SyntaxSymbol.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbolWriter.o: ../../../Source/Syntax/SyntaxSymbolWriter.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol_Automaton.o: ../../../Source/Syntax/SyntaxSymbol_Automaton.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol_CreateParserGenRuleSyntax.o: ../../../Source/Syntax/SyntaxSymbol_CreateParserGenRuleSyntax.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol_CreateParserGenTypeSyntax.o: ../../../Source/Syntax/SyntaxSymbol_CreateParserGenTypeSyntax.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol_CreateParserGenUtility.o: ../../../Source/Syntax/SyntaxSymbol_CreateParserGenUtility.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol_NFACompact.o: ../../../Source/Syntax/SyntaxSymbol_NFACompact.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol_NFACompact_EliminateEpsilonEdges.o: ../../../Source/Syntax/SyntaxSymbol_NFACompact_EliminateEpsilonEdges.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol_NFACompact_EliminateLeftRecursion.o: ../../../Source/Syntax/SyntaxSymbol_NFACompact_EliminateLeftRecursion.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol_NFACompact_MergeEdgesWithSameInput.o: ../../../Source/Syntax/SyntaxSymbol_NFACompact_MergeEdgesWithSameInput.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol_NFACompact_PrefixMergeCrossReference.o: ../../../Source/Syntax/SyntaxSymbol_NFACompact_PrefixMergeCrossReference.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol_NFACompact_MergeEdgesWithSameRuleUsingLeftrec.o: ../../../Source/Syntax/SyntaxSymbol_NFACompact_MergeEdgesWithSameRuleUsingLeftrec.cpp
	$(CPP_COMPILE)

./Obj/SyntaxSymbol_NFACrossReferenced.o: ../../../Source/Syntax/SyntaxSymbol_NFACrossReferenced.cpp
	$(CPP_COMPILE)

./Obj/ParserCppGen.o: ../../../Source/ParserGen_Global/ParserCppGen.cpp
	$(CPP_COMPILE)

./Obj/ParserSymbol.o: ../../../Source/ParserGen_Global/ParserSymbol.cpp
	$(CPP_COMPILE)

./Obj/ParserGenRuleAst.o: ../../../Source/ParserGen_Generated/ParserGenRuleAst.cpp
	$(CPP_COMPILE)

./Obj/ParserGenRuleAst_Builder.o: ../../../Source/ParserGen_Generated/ParserGenRuleAst_Builder.cpp
	$(CPP_COMPILE)

./Obj/ParserGenRuleAst_Copy.o: ../../../Source/ParserGen_Generated/ParserGenRuleAst_Copy.cpp
	$(CPP_COMPILE)

./Obj/ParserGenRuleAst_Empty.o: ../../../Source/ParserGen_Generated/ParserGenRuleAst_Empty.cpp
	$(CPP_COMPILE)

./Obj/ParserGenRuleAst_Json.o: ../../../Source/ParserGen_Generated/ParserGenRuleAst_Json.cpp
	$(CPP_COMPILE)

./Obj/ParserGenRuleAst_Traverse.o: ../../../Source/ParserGen_Generated/ParserGenRuleAst_Traverse.cpp
	$(CPP_COMPILE)

./Obj/ParserGenRuleParser.o: ../../../Source/ParserGen_Generated/ParserGenRuleParser.cpp
	$(CPP_COMPILE)

./Obj/ParserGenTypeAst.o: ../../../Source/ParserGen_Generated/ParserGenTypeAst.cpp
	$(CPP_COMPILE)

./Obj/ParserGenTypeAst_Builder.o: ../../../Source/ParserGen_Generated/ParserGenTypeAst_Builder.cpp
	$(CPP_COMPILE)

./Obj/ParserGenTypeAst_Copy.o: ../../../Source/ParserGen_Generated/ParserGenTypeAst_Copy.cpp
	$(CPP_COMPILE)

./Obj/ParserGenTypeAst_Empty.o: ../../../Source/ParserGen_Generated/ParserGenTypeAst_Empty.cpp
	$(CPP_COMPILE)

./Obj/ParserGenTypeAst_Json.o: ../../../Source/ParserGen_Generated/ParserGenTypeAst_Json.cpp
	$(CPP_COMPILE)

./Obj/ParserGenTypeAst_Traverse.o: ../../../Source/ParserGen_Generated/ParserGenTypeAst_Traverse.cpp
	$(CPP_COMPILE)

./Obj/ParserGenTypeParser.o: ../../../Source/ParserGen_Generated/ParserGenTypeParser.cpp
	$(CPP_COMPILE)

./Obj/ParserGen_Assembler.o: ../../../Source/ParserGen_Generated/ParserGen_Assembler.cpp
	$(CPP_COMPILE)

./Obj/ParserGen_Lexer.o: ../../../Source/ParserGen_Generated/ParserGen_Lexer.cpp
	$(CPP_COMPILE)

./Obj/CompileAst.o: ../../../Source/ParserGen/CompileAst.cpp
	$(CPP_COMPILE)

./Obj/CompileLexer.o: ../../../Source/ParserGen/CompileLexer.cpp
	$(CPP_COMPILE)

./Obj/CompileSyntax.o: ../../../Source/ParserGen/CompileSyntax.cpp
	$(CPP_COMPILE)

./Obj/CompileSyntax_CalculateFirstSet.o: ../../../Source/ParserGen/CompileSyntax_CalculateFirstSet.cpp
	$(CPP_COMPILE)

./Obj/CompileSyntax_CalculateTypes.o: ../../../Source/ParserGen/CompileSyntax_CalculateTypes.cpp
	$(CPP_COMPILE)

./Obj/CompileSyntax_CompileSyntax.o: ../../../Source/ParserGen/CompileSyntax_CompileSyntax.cpp
	$(CPP_COMPILE)

./Obj/CompileSyntax_ResolveName.o: ../../../Source/ParserGen/CompileSyntax_ResolveName.cpp
	$(CPP_COMPILE)

./Obj/CompileSyntax_RewriteSyntax_Switch.o: ../../../Source/ParserGen/CompileSyntax_RewriteSyntax_Switch.cpp
	$(CPP_COMPILE)

./Obj/CompileSyntax_ValidateStructure.o: ../../../Source/ParserGen/CompileSyntax_ValidateStructure.cpp
	$(CPP_COMPILE)

./Obj/CompileSyntax_ValidateSwitchesAndConditions.o: ../../../Source/ParserGen/CompileSyntax_ValidateSwitchesAndConditions.cpp
	$(CPP_COMPILE)

./Obj/CompileSyntax_ValidateTypes.o: ../../../Source/ParserGen/CompileSyntax_ValidateTypes.cpp
	$(CPP_COMPILE)

./Obj/SyntaxAstToCode.o: ../../../Source/ParserGen_Printer/SyntaxAstToCode.cpp
	$(CPP_COMPILE)

./Obj/TypeAstToCode.o: ../../../Source/ParserGen_Printer/TypeAstToCode.cpp
	$(CPP_COMPILE)

./Obj/TypeSymbolToAst.o: ../../../Source/ParserGen_Printer/TypeSymbolToAst.cpp
	$(CPP_COMPILE)

./Obj/JsonAst.o: ../../../Source/Json/Generated/JsonAst.cpp
	$(CPP_COMPILE)

./Obj/JsonAst_Builder.o: ../../../Source/Json/Generated/JsonAst_Builder.cpp
	$(CPP_COMPILE)

./Obj/JsonAst_Copy.o: ../../../Source/Json/Generated/JsonAst_Copy.cpp
	$(CPP_COMPILE)

./Obj/JsonAst_Empty.o: ../../../Source/Json/Generated/JsonAst_Empty.cpp
	$(CPP_COMPILE)

./Obj/JsonAst_Json.o: ../../../Source/Json/Generated/JsonAst_Json.cpp
	$(CPP_COMPILE)

./Obj/JsonAst_Traverse.o: ../../../Source/Json/Generated/JsonAst_Traverse.cpp
	$(CPP_COMPILE)

./Obj/JsonParser.o: ../../../Source/Json/Generated/JsonParser.cpp
	$(CPP_COMPILE)

./Obj/Json_Assembler.o: ../../../Source/Json/Generated/Json_Assembler.cpp
	$(CPP_COMPILE)

./Obj/Json_Lexer.o: ../../../Source/Json/Generated/Json_Lexer.cpp
	$(CPP_COMPILE)

./Obj/GlrJson.o: ../../../Source/Json/GlrJson.cpp
	$(CPP_COMPILE)

./Obj/CalculatorExprAst.o: ../../Source/Calculator/Parser/CalculatorExprAst.cpp
	$(CPP_COMPILE)

./Obj/CalculatorExprAst_Builder.o: ../../Source/Calculator/Parser/CalculatorExprAst_Builder.cpp
	$(CPP_COMPILE)

./Obj/CalculatorExprAst_Copy.o: ../../Source/Calculator/Parser/CalculatorExprAst_Copy.cpp
	$(CPP_COMPILE)

./Obj/CalculatorExprAst_Empty.o: ../../Source/Calculator/Parser/CalculatorExprAst_Empty.cpp
	$(CPP_COMPILE)

./Obj/CalculatorExprAst_Json.o: ../../Source/Calculator/Parser/CalculatorExprAst_Json.cpp
	$(CPP_COMPILE)

./Obj/CalculatorExprAst_Traverse.o: ../../Source/Calculator/Parser/CalculatorExprAst_Traverse.cpp
	$(CPP_COMPILE)

./Obj/Calculator_Assembler.o: ../../Source/Calculator/Parser/Calculator_Assembler.cpp
	$(CPP_COMPILE)

./Obj/Calculator_Lexer.o: ../../Source/Calculator/Parser/Calculator_Lexer.cpp
	$(CPP_COMPILE)

./Obj/CalculatorModuleParser.o: ../../Source/Calculator/Parser/CalculatorModuleParser.cpp
	$(CPP_COMPILE)

./Obj/LogAutomaton.o: ../../Source/LogAutomaton.cpp
	$(CPP_COMPILE)

./Obj/LogParser.o: ../../Source/LogParser.cpp
	$(CPP_COMPILE)

./Obj/LogTrace.o: ../../Source/LogTrace.cpp
	$(CPP_COMPILE)

./Obj/CommonMain.o: ../../UnitTest/CommonMain.cpp
	$(CPP_COMPILE)

./Obj/TestParserGen.o: ../../UnitTest/ParserTest_ParserGen_Compiler/TestParserGen.cpp
	$(CPP_COMPILE)

./Obj/TestSyntax.o: ../../UnitTest/ParserTest_ParserGen_Compiler/TestSyntax.cpp
	$(CPP_COMPILE)

./Obj/TestError.o: ../../UnitTest/ParserTest_ParserGen/TestError.cpp
	$(CPP_COMPILE)

./Obj/TestError_Ast.o: ../../UnitTest/ParserTest_ParserGen/TestError_Ast.cpp
	$(CPP_COMPILE)

./Obj/TestError_CalculatorAstAndLexer.o: ../../UnitTest/ParserTest_ParserGen/TestError_CalculatorAstAndLexer.cpp
	$(CPP_COMPILE)

./Obj/TestError_Condition_ResolveName.o: ../../UnitTest/ParserTest_ParserGen/TestError_Condition_ResolveName.cpp
	$(CPP_COMPILE)

./Obj/TestError_Condition_RewriteSyntax.o: ../../UnitTest/ParserTest_ParserGen/TestError_Condition_RewriteSyntax.cpp
	$(CPP_COMPILE)

./Obj/TestError_Condition_ValidateSwitches.o: ../../UnitTest/ParserTest_ParserGen/TestError_Condition_ValidateSwitches.cpp
	$(CPP_COMPILE)

./Obj/TestError_Lexer.o: ../../UnitTest/ParserTest_ParserGen/TestError_Lexer.cpp
	$(CPP_COMPILE)

./Obj/TestError_Syntax_Automaton.o: ../../UnitTest/ParserTest_ParserGen/TestError_Syntax_Automaton.cpp
	$(CPP_COMPILE)

./Obj/TestError_Syntax_CalculateTypes.o: ../../UnitTest/ParserTest_ParserGen/TestError_Syntax_CalculateTypes.cpp
	$(CPP_COMPILE)

./Obj/TestError_Syntax_ResolveName.o: ../../UnitTest/ParserTest_ParserGen/TestError_Syntax_ResolveName.cpp
	$(CPP_COMPILE)

./Obj/TestError_Syntax_ValidateStructure.o: ../../UnitTest/ParserTest_ParserGen/TestError_Syntax_ValidateStructure.cpp
	$(CPP_COMPILE)

./Obj/TestError_Syntax_ValidateTypes.o: ../../UnitTest/ParserTest_ParserGen/TestError_Syntax_ValidateTypes.cpp
	$(CPP_COMPILE)

./Obj/TestRewrite_Condition.o: ../../UnitTest/ParserTest_ParserGen/TestRewrite_Condition.cpp
	$(CPP_COMPILE)
