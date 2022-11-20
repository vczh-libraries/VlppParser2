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

./Bin/UnitTest:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppReflection.o ./Obj/VlppRegex.o ./Obj/AstBase.o ./Obj/AstPrint.o ./Obj/Executable.o ./Obj/SyntaxBase.o ./Obj/TmPtr_AllocateExecutionData.o ./Obj/TmPtr_BuildAmbiguityStructures.o ./Obj/TmPtr_PartialExecuteTraces.o ./Obj/TmPtr_PartialExecuteTraces_EnsureInsExecContextCompatible.o ./Obj/TmPtr_PartialExecuteTraces_MergeInsExecContext.o ./Obj/TmPtr_PartialExecuteTraces_PartialExecuteOrdinaryTrace.o ./Obj/TmRa.o ./Obj/TmRa_BuildExecutionOrder.o ./Obj/TmRa_CheckMergeTraces.o ./Obj/TraceManager.o ./Obj/TraceManager_Common.o ./Obj/TraceManager_ExecuteTrace.o ./Obj/TmInput.o ./Obj/TmInput_Ambiguity.o ./Obj/TmInput_Competition.o ./Obj/TmInput_ReturnStack.o ./Obj/TmInput_Walk.o ./Obj/TmPtr.o ./Obj/JsonAst.o ./Obj/JsonAst_Builder.o ./Obj/JsonAst_Copy.o ./Obj/JsonAst_Empty.o ./Obj/JsonAst_Json.o ./Obj/JsonAst_Traverse.o ./Obj/JsonParser.o ./Obj/Json_Assembler.o ./Obj/Json_Lexer.o ./Obj/GlrJson.o ./Obj/LogTrace.o ./Obj/CommonMain.o ./Obj/TestAPI.o ./Obj/TestParser.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../../Import/Vlpp.cpp ../../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/Vlpp.Linux.o: ../../../Import/Vlpp.Linux.cpp ../../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../../Import/VlppOS.cpp ../../../Import/VlppOS.h ../../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppOS.Linux.o: ../../../Import/VlppOS.Linux.cpp ../../../Import/VlppOS.h ../../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppReflection.o: ../../../Import/VlppReflection.cpp ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/VlppRegex.o: ../../../Import/VlppRegex.cpp ../../../Import/VlppRegex.h ../../../Import/Vlpp.h ../../../Import/VlppOS.h
	$(CPP_COMPILE)

./Obj/AstBase.o: ../../../Source/AstBase.cpp ../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/AstPrint.o: ../../../Source/AstPrint.cpp ../../../Source/AstPrint.h ../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/Executable.o: ../../../Source/Executable.cpp ../../../Source/Executable.h ../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/SyntaxBase.o: ../../../Source/SyntaxBase.cpp ../../../Source/SyntaxBase.h ../../../Source/Executable.h ../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmPtr_AllocateExecutionData.o: ../../../Source/TraceManager/TmPtr_AllocateExecutionData.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/TraceManager/TraceManager_Common.h
	$(CPP_COMPILE)

./Obj/TmPtr_BuildAmbiguityStructures.o: ../../../Source/TraceManager/TmPtr_BuildAmbiguityStructures.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/TraceManager/TraceManager_Common.h
	$(CPP_COMPILE)

./Obj/TmPtr_PartialExecuteTraces.o: ../../../Source/TraceManager/TmPtr_PartialExecuteTraces.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/TraceManager/TraceManager_Common.h
	$(CPP_COMPILE)

./Obj/TmPtr_PartialExecuteTraces_EnsureInsExecContextCompatible.o: ../../../Source/TraceManager/TmPtr_PartialExecuteTraces_EnsureInsExecContextCompatible.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmPtr_PartialExecuteTraces_MergeInsExecContext.o: ../../../Source/TraceManager/TmPtr_PartialExecuteTraces_MergeInsExecContext.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmPtr_PartialExecuteTraces_PartialExecuteOrdinaryTrace.o: ../../../Source/TraceManager/TmPtr_PartialExecuteTraces_PartialExecuteOrdinaryTrace.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmRa.o: ../../../Source/TraceManager/TmRa.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmRa_BuildExecutionOrder.o: ../../../Source/TraceManager/TmRa_BuildExecutionOrder.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmRa_CheckMergeTraces.o: ../../../Source/TraceManager/TmRa_CheckMergeTraces.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TraceManager.o: ../../../Source/TraceManager/TraceManager.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TraceManager_Common.o: ../../../Source/TraceManager/TraceManager_Common.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TraceManager_ExecuteTrace.o: ../../../Source/TraceManager/TraceManager_ExecuteTrace.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmInput.o: ../../../Source/TraceManager/TmInput.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmInput_Ambiguity.o: ../../../Source/TraceManager/TmInput_Ambiguity.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmInput_Competition.o: ../../../Source/TraceManager/TmInput_Competition.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmInput_ReturnStack.o: ../../../Source/TraceManager/TmInput_ReturnStack.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmInput_Walk.o: ../../../Source/TraceManager/TmInput_Walk.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TmPtr.o: ../../../Source/TraceManager/TmPtr.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/TraceManager/TraceManager_Common.h
	$(CPP_COMPILE)

./Obj/JsonAst.o: ../../../Source/Json/Generated/JsonAst.cpp ../../../Source/Json/Generated/JsonAst.h ../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/Json/Generated/../../SyntaxBase.h ../../../Source/Json/Generated/../../Executable.h
	$(CPP_COMPILE)

./Obj/JsonAst_Builder.o: ../../../Source/Json/Generated/JsonAst_Builder.cpp ../../../Source/Json/Generated/JsonAst_Builder.h ../../../Source/Json/Generated/JsonAst.h ../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/Json/Generated/../../SyntaxBase.h ../../../Source/Json/Generated/../../Executable.h
	$(CPP_COMPILE)

./Obj/JsonAst_Copy.o: ../../../Source/Json/Generated/JsonAst_Copy.cpp ../../../Source/Json/Generated/JsonAst_Copy.h ../../../Source/Json/Generated/JsonAst.h ../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/Json/Generated/../../SyntaxBase.h ../../../Source/Json/Generated/../../Executable.h
	$(CPP_COMPILE)

./Obj/JsonAst_Empty.o: ../../../Source/Json/Generated/JsonAst_Empty.cpp ../../../Source/Json/Generated/JsonAst_Empty.h ../../../Source/Json/Generated/JsonAst.h ../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/Json/Generated/../../SyntaxBase.h ../../../Source/Json/Generated/../../Executable.h
	$(CPP_COMPILE)

./Obj/JsonAst_Json.o: ../../../Source/Json/Generated/JsonAst_Json.cpp ../../../Source/Json/Generated/JsonAst_Json.h ../../../Source/Json/Generated/JsonAst.h ../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/Json/Generated/../../SyntaxBase.h ../../../Source/Json/Generated/../../Executable.h
	$(CPP_COMPILE)

./Obj/JsonAst_Traverse.o: ../../../Source/Json/Generated/JsonAst_Traverse.cpp ../../../Source/Json/Generated/JsonAst_Traverse.h ../../../Source/Json/Generated/JsonAst.h ../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/Json/Generated/../../SyntaxBase.h ../../../Source/Json/Generated/../../Executable.h
	$(CPP_COMPILE)

./Obj/JsonParser.o: ../../../Source/Json/Generated/JsonParser.cpp ../../../Source/Json/Generated/JsonParser.h ../../../Source/Json/Generated/Json_Assembler.h ../../../Source/Json/Generated/JsonAst.h ../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/Json/Generated/../../SyntaxBase.h ../../../Source/Json/Generated/../../Executable.h ../../../Source/Json/Generated/Json_Lexer.h
	$(CPP_COMPILE)

./Obj/Json_Assembler.o: ../../../Source/Json/Generated/Json_Assembler.cpp ../../../Source/Json/Generated/Json_Assembler.h ../../../Source/Json/Generated/JsonAst.h ../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/Json/Generated/../../SyntaxBase.h ../../../Source/Json/Generated/../../Executable.h
	$(CPP_COMPILE)

./Obj/Json_Lexer.o: ../../../Source/Json/Generated/Json_Lexer.cpp ../../../Source/Json/Generated/Json_Lexer.h ../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/Json/Generated/../../SyntaxBase.h ../../../Source/Json/Generated/../../Executable.h
	$(CPP_COMPILE)

./Obj/GlrJson.o: ../../../Source/Json/GlrJson.cpp ../../../Source/Json/GlrJson.h ../../../Source/Json/Generated/JsonParser.h ../../../Source/Json/Generated/Json_Assembler.h ../../../Source/Json/Generated/JsonAst.h ../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../../Source/Json/Generated/../../SyntaxBase.h ../../../Source/Json/Generated/../../Executable.h ../../../Source/Json/Generated/Json_Lexer.h ../../../Source/Json/Generated/JsonAst_Traverse.h
	$(CPP_COMPILE)

./Obj/LogTrace.o: ../../Source/LogTrace.cpp ../../Source/LogTrace.h ../../Source/../../Source/SyntaxBase.h ../../Source/../../Source/Executable.h ../../Source/../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/../../Source/TraceManager/TraceManager.h ../../Source/../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/CommonMain.o: ../../UnitTest/CommonMain.cpp ../../../Import/VlppOS.h ../../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/TestAPI.o: ../../UnitTest/BuiltInTest_Json/TestAPI.cpp ../../UnitTest/BuiltInTest_Json/../../../Source/Json/GlrJson.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/JsonParser.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/Json_Assembler.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/JsonAst.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/../../SyntaxBase.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/../../Executable.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/Json_Lexer.h
	$(CPP_COMPILE)

./Obj/TestParser.o: ../../UnitTest/BuiltInTest_Json/TestParser.cpp ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/JsonParser.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/Json_Assembler.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/JsonAst.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/../../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/../../SyntaxBase.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/../../Executable.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/Json_Lexer.h ../../UnitTest/BuiltInTest_Json/../../../Source/Json/Generated/JsonAst_Json.h ../../UnitTest/BuiltInTest_Json/../../Source/LogTrace.h ../../UnitTest/BuiltInTest_Json/../../Source/../../Source/SyntaxBase.h ../../UnitTest/BuiltInTest_Json/../../Source/../../Source/TraceManager/TraceManager.h ../../UnitTest/BuiltInTest_Json/../../Source/../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)
