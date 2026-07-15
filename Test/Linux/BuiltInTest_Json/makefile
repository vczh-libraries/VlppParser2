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

./Bin/UnitTest:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppReflection.o ./Obj/VlppRegex.o ./Obj/AstBase.o ./Obj/AstPrint.o ./Obj/Executable.o ./Obj/SyntaxBase.o ./Obj/TmPtr_AllocateExecutionData.o ./Obj/TmPtr_BuildAmbiguityStructures.o ./Obj/TmPtr_PartialExecuteTraces.o ./Obj/TmPtr_PartialExecuteTraces_SummarizeInstructionRange.o ./Obj/TmPtr_PartialExecuteTraces_EnsureInsExecContextCompatible.o ./Obj/TmPtr_PartialExecuteTraces_MergeInsExecContext.o ./Obj/TmPtr_PartialExecuteTraces_PartialExecuteOrdinaryTrace.o ./Obj/TmRa.o ./Obj/TmRa_BuildExecutionOrder.o ./Obj/TmRa_CheckMergeTraces.o ./Obj/TraceManager.o ./Obj/TraceManager_Common.o ./Obj/TraceManager_ExecuteTrace.o ./Obj/TmInput.o ./Obj/TmInput_Ambiguity.o ./Obj/TmInput_Competition.o ./Obj/TmInput_ReturnStack.o ./Obj/TmInput_Walk.o ./Obj/TmPtr.o ./Obj/JsonAst.o ./Obj/JsonAst_Builder.o ./Obj/JsonAst_Copy.o ./Obj/JsonAst_Empty.o ./Obj/JsonAst_Json.o ./Obj/JsonAst_Traverse.o ./Obj/JsonParser.o ./Obj/Json_Assembler.o ./Obj/Json_Lexer.o ./Obj/GlrJson.o ./Obj/LogAutomaton.o ./Obj/LogTrace.o ./Obj/CommonMain.o ./Obj/TestAPI.o ./Obj/TestParser.o
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

./Obj/LogAutomaton.o: ../../Source/LogAutomaton.cpp
	$(CPP_COMPILE)

./Obj/LogTrace.o: ../../Source/LogTrace.cpp
	$(CPP_COMPILE)

./Obj/CommonMain.o: ../../UnitTest/CommonMain.cpp
	$(CPP_COMPILE)

./Obj/TestAPI.o: ../../UnitTest/BuiltInTest_Json/TestAPI.cpp
	$(CPP_COMPILE)

./Obj/TestParser.o: ../../UnitTest/BuiltInTest_Json/TestParser.cpp
	$(CPP_COMPILE)
