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

./Bin/UnitTest:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppReflection.o ./Obj/VlppRegex.o ./Obj/AstBase.o ./Obj/AstPrint.o ./Obj/Executable.o ./Obj/SyntaxBase.o ./Obj/TmPtr_AllocateExecutionData.o ./Obj/TmPtr_BuildAmbiguityStructures.o ./Obj/TmPtr_PartialExecuteTraces.o ./Obj/TmPtr_PartialExecuteTraces_CalculateObjectFirstInstruction.o ./Obj/TmPtr_PartialExecuteTraces_EnsureInsExecContextCompatible.o ./Obj/TmPtr_PartialExecuteTraces_MergeInsExecContext.o ./Obj/TmPtr_PartialExecuteTraces_PartialExecuteOrdinaryTrace.o ./Obj/TmRa.o ./Obj/TmRa_BuildExecutionOrder.o ./Obj/TmRa_CheckMergeTraces.o ./Obj/TraceManager.o ./Obj/TraceManager_Common.o ./Obj/TraceManager_ExecuteTrace.o ./Obj/TmInput.o ./Obj/TmInput_Ambiguity.o ./Obj/TmInput_Competition.o ./Obj/TmInput_ReturnStack.o ./Obj/TmInput_Walk.o ./Obj/TmPtr.o ./Obj/WorkflowAst.o ./Obj/WorkflowAst_Builder.o ./Obj/WorkflowAst_Copy.o ./Obj/WorkflowAst_Empty.o ./Obj/WorkflowAst_Json.o ./Obj/WorkflowAst_Traverse.o ./Obj/WorkflowParser.o ./Obj/Workflow_Assembler.o ./Obj/Workflow_Lexer.o ./Obj/LogTrace.o ./Obj/CommonMain.o ./Obj/TestParser.o
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

./Obj/TmPtr_PartialExecuteTraces_CalculateObjectFirstInstruction.o: ../../../Source/TraceManager/TmPtr_PartialExecuteTraces_CalculateObjectFirstInstruction.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
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

./Obj/WorkflowAst.o: ../../Source/BuiltIn-Workflow/Generated/WorkflowAst.cpp ../../Source/BuiltIn-Workflow/Generated/WorkflowAst.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/SyntaxBase.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/Executable.h
	$(CPP_COMPILE)

./Obj/WorkflowAst_Builder.o: ../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Builder.cpp ../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Builder.h ../../Source/BuiltIn-Workflow/Generated/WorkflowAst.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/SyntaxBase.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/Executable.h
	$(CPP_COMPILE)

./Obj/WorkflowAst_Copy.o: ../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Copy.cpp ../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Copy.h ../../Source/BuiltIn-Workflow/Generated/WorkflowAst.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/SyntaxBase.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/Executable.h
	$(CPP_COMPILE)

./Obj/WorkflowAst_Empty.o: ../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Empty.cpp ../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Empty.h ../../Source/BuiltIn-Workflow/Generated/WorkflowAst.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/SyntaxBase.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/Executable.h
	$(CPP_COMPILE)

./Obj/WorkflowAst_Json.o: ../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Json.cpp ../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Json.h ../../Source/BuiltIn-Workflow/Generated/WorkflowAst.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/SyntaxBase.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/Executable.h
	$(CPP_COMPILE)

./Obj/WorkflowAst_Traverse.o: ../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Traverse.cpp ../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Traverse.h ../../Source/BuiltIn-Workflow/Generated/WorkflowAst.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/SyntaxBase.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/Executable.h
	$(CPP_COMPILE)

./Obj/WorkflowParser.o: ../../Source/BuiltIn-Workflow/Generated/WorkflowParser.cpp ../../Source/BuiltIn-Workflow/Generated/WorkflowParser.h ../../Source/BuiltIn-Workflow/Generated/Workflow_Assembler.h ../../Source/BuiltIn-Workflow/Generated/WorkflowAst.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/SyntaxBase.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/Executable.h ../../Source/BuiltIn-Workflow/Generated/Workflow_Lexer.h
	$(CPP_COMPILE)

./Obj/Workflow_Assembler.o: ../../Source/BuiltIn-Workflow/Generated/Workflow_Assembler.cpp ../../Source/BuiltIn-Workflow/Generated/Workflow_Assembler.h ../../Source/BuiltIn-Workflow/Generated/WorkflowAst.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/SyntaxBase.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/Executable.h
	$(CPP_COMPILE)

./Obj/Workflow_Lexer.o: ../../Source/BuiltIn-Workflow/Generated/Workflow_Lexer.cpp ../../Source/BuiltIn-Workflow/Generated/Workflow_Lexer.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/SyntaxBase.h ../../Source/BuiltIn-Workflow/Generated/../../../../Source/Executable.h
	$(CPP_COMPILE)

./Obj/LogTrace.o: ../../Source/LogTrace.cpp ../../Source/LogTrace.h ../../Source/../../Source/SyntaxBase.h ../../Source/../../Source/Executable.h ../../Source/../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/../../Source/TraceManager/TraceManager.h ../../Source/../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/CommonMain.o: ../../UnitTest/CommonMain.cpp ../../../Import/VlppOS.h ../../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/TestParser.o: ../../UnitTest/BuiltInTest_Workflow/TestParser.cpp ../../UnitTest/BuiltInTest_Workflow/../../Source/BuiltIn-Workflow/Generated/WorkflowParser.h ../../UnitTest/BuiltInTest_Workflow/../../Source/BuiltIn-Workflow/Generated/Workflow_Assembler.h ../../UnitTest/BuiltInTest_Workflow/../../Source/BuiltIn-Workflow/Generated/WorkflowAst.h ../../UnitTest/BuiltInTest_Workflow/../../Source/BuiltIn-Workflow/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../UnitTest/BuiltInTest_Workflow/../../Source/BuiltIn-Workflow/Generated/../../../../Source/SyntaxBase.h ../../UnitTest/BuiltInTest_Workflow/../../Source/BuiltIn-Workflow/Generated/../../../../Source/Executable.h ../../UnitTest/BuiltInTest_Workflow/../../Source/BuiltIn-Workflow/Generated/Workflow_Lexer.h ../../UnitTest/BuiltInTest_Workflow/../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Json.h ../../UnitTest/BuiltInTest_Workflow/../../Source/LogTrace.h ../../UnitTest/BuiltInTest_Workflow/../../Source/../../Source/SyntaxBase.h ../../UnitTest/BuiltInTest_Workflow/../../Source/../../Source/TraceManager/TraceManager.h ../../UnitTest/BuiltInTest_Workflow/../../Source/../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)
