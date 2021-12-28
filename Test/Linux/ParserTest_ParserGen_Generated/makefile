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

./Bin/UnitTest:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppReflection.o ./Obj/VlppRegex.o ./Obj/AstBase.o ./Obj/AstPrint.o ./Obj/Executable.o ./Obj/SyntaxBase.o ./Obj/TraceManager.o ./Obj/TraceManager_ExecuteTrace.o ./Obj/TraceManager_Input.o ./Obj/TraceManager_Input_Ambiguity.o ./Obj/TraceManager_Input_Competition.o ./Obj/TraceManager_Input_Walk.o ./Obj/TraceManager_PrepareTraceRoute.o ./Obj/BinaryOpExprAst.o ./Obj/BinaryOpExprAst_Builder.o ./Obj/BinaryOpExprAst_Copy.o ./Obj/BinaryOpExprAst_Empty.o ./Obj/BinaryOpExprAst_Json.o ./Obj/BinaryOpExprAst_Traverse.o ./Obj/BinaryOpModuleParser.o ./Obj/BinaryOp_Assembler.o ./Obj/BinaryOp_Lexer.o ./Obj/CalculatorExprAst.o ./Obj/CalculatorExprAst_Builder.o ./Obj/CalculatorExprAst_Copy.o ./Obj/CalculatorExprAst_Empty.o ./Obj/CalculatorExprAst_Json.o ./Obj/CalculatorExprAst_Traverse.o ./Obj/CalculatorModuleParser.o ./Obj/Calculator_Assembler.o ./Obj/Calculator_Lexer.o ./Obj/FeatureTestFeatureAst.o ./Obj/FeatureTestFeatureAst_Builder.o ./Obj/FeatureTestFeatureAst_Copy.o ./Obj/FeatureTestFeatureAst_Empty.o ./Obj/FeatureTestFeatureAst_Json.o ./Obj/FeatureTestFeatureAst_Traverse.o ./Obj/FeatureTestModuleParser.o ./Obj/FeatureTest_Assembler.o ./Obj/FeatureTest_Lexer.o ./Obj/GenericAmbiguityExprAst.o ./Obj/GenericAmbiguityExprAst_Builder.o ./Obj/GenericAmbiguityExprAst_Copy.o ./Obj/GenericAmbiguityExprAst_Empty.o ./Obj/GenericAmbiguityExprAst_Json.o ./Obj/GenericAmbiguityExprAst_Traverse.o ./Obj/GenericAmbiguityModuleParser.o ./Obj/GenericAmbiguity_Assembler.o ./Obj/GenericAmbiguity_Lexer.o ./Obj/IfElseAmbiguityModuleParser.o ./Obj/IfElseAmbiguityStatAst.o ./Obj/IfElseAmbiguityStatAst_Builder.o ./Obj/IfElseAmbiguityStatAst_Copy.o ./Obj/IfElseAmbiguityStatAst_Empty.o ./Obj/IfElseAmbiguityStatAst_Json.o ./Obj/IfElseAmbiguityStatAst_Traverse.o ./Obj/IfElseAmbiguity_Assembler.o ./Obj/IfElseAmbiguity_Lexer.o ./Obj/IfElsePriorityModuleParser.o ./Obj/IfElsePriorityStatAst.o ./Obj/IfElsePriorityStatAst_Builder.o ./Obj/IfElsePriorityStatAst_Copy.o ./Obj/IfElsePriorityStatAst_Empty.o ./Obj/IfElsePriorityStatAst_Json.o ./Obj/IfElsePriorityStatAst_Traverse.o ./Obj/IfElsePriority_Assembler.o ./Obj/IfElsePriority_Lexer.o ./Obj/LogTrace.o ./Obj/CommonMain.o ./Obj/TestParser_Generated.o
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

./Obj/SyntaxBase.o: ../../../Source/SyntaxBase.cpp ../../../Source/SyntaxBase.h ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TraceManager.o: ../../../Source/TraceManager/TraceManager.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TraceManager_ExecuteTrace.o: ../../../Source/TraceManager/TraceManager_ExecuteTrace.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TraceManager_Input.o: ../../../Source/TraceManager/TraceManager_Input.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TraceManager_Input_Ambiguity.o: ../../../Source/TraceManager/TraceManager_Input_Ambiguity.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TraceManager_Input_Competition.o: ../../../Source/TraceManager/TraceManager_Input_Competition.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TraceManager_Input_Walk.o: ../../../Source/TraceManager/TraceManager_Input_Walk.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/TraceManager_PrepareTraceRoute.o: ../../../Source/TraceManager/TraceManager_PrepareTraceRoute.cpp ../../../Source/TraceManager/TraceManager.h ../../../Source/TraceManager/../Executable.h ../../../Source/TraceManager/../AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/BinaryOpExprAst.o: ../../Source/BinaryOp/Generated/BinaryOpExprAst.cpp ../../Source/BinaryOp/Generated/BinaryOpExprAst.h ../../Source/BinaryOp/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BinaryOp/Generated/../../../../Source/SyntaxBase.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/BinaryOpExprAst_Builder.o: ../../Source/BinaryOp/Generated/BinaryOpExprAst_Builder.cpp ../../Source/BinaryOp/Generated/BinaryOpExprAst_Builder.h ../../Source/BinaryOp/Generated/BinaryOpExprAst.h ../../Source/BinaryOp/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BinaryOp/Generated/../../../../Source/SyntaxBase.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/BinaryOpExprAst_Copy.o: ../../Source/BinaryOp/Generated/BinaryOpExprAst_Copy.cpp ../../Source/BinaryOp/Generated/BinaryOpExprAst_Copy.h ../../Source/BinaryOp/Generated/BinaryOpExprAst.h ../../Source/BinaryOp/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BinaryOp/Generated/../../../../Source/SyntaxBase.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/BinaryOpExprAst_Empty.o: ../../Source/BinaryOp/Generated/BinaryOpExprAst_Empty.cpp ../../Source/BinaryOp/Generated/BinaryOpExprAst_Empty.h ../../Source/BinaryOp/Generated/BinaryOpExprAst.h ../../Source/BinaryOp/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BinaryOp/Generated/../../../../Source/SyntaxBase.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/BinaryOpExprAst_Json.o: ../../Source/BinaryOp/Generated/BinaryOpExprAst_Json.cpp ../../Source/BinaryOp/Generated/BinaryOpExprAst_Json.h ../../Source/BinaryOp/Generated/BinaryOpExprAst.h ../../Source/BinaryOp/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BinaryOp/Generated/../../../../Source/SyntaxBase.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/BinaryOpExprAst_Traverse.o: ../../Source/BinaryOp/Generated/BinaryOpExprAst_Traverse.cpp ../../Source/BinaryOp/Generated/BinaryOpExprAst_Traverse.h ../../Source/BinaryOp/Generated/BinaryOpExprAst.h ../../Source/BinaryOp/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BinaryOp/Generated/../../../../Source/SyntaxBase.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/BinaryOpModuleParser.o: ../../Source/BinaryOp/Generated/BinaryOpModuleParser.cpp ../../Source/BinaryOp/Generated/BinaryOpModuleParser.h ../../Source/BinaryOp/Generated/BinaryOp_Assembler.h ../../Source/BinaryOp/Generated/BinaryOpExprAst.h ../../Source/BinaryOp/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BinaryOp/Generated/../../../../Source/SyntaxBase.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/../Executable.h ../../Source/BinaryOp/Generated/BinaryOp_Lexer.h
	$(CPP_COMPILE)

./Obj/BinaryOp_Assembler.o: ../../Source/BinaryOp/Generated/BinaryOp_Assembler.cpp ../../Source/BinaryOp/Generated/BinaryOp_Assembler.h ../../Source/BinaryOp/Generated/BinaryOpExprAst.h ../../Source/BinaryOp/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BinaryOp/Generated/../../../../Source/SyntaxBase.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/BinaryOp_Lexer.o: ../../Source/BinaryOp/Generated/BinaryOp_Lexer.cpp ../../Source/BinaryOp/Generated/BinaryOp_Lexer.h ../../Source/BinaryOp/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/BinaryOp/Generated/../../../../Source/SyntaxBase.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/BinaryOp/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/CalculatorExprAst.o: ../../Source/Calculator/Generated/CalculatorExprAst.cpp ../../Source/Calculator/Generated/CalculatorExprAst.h ../../Source/Calculator/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/Calculator/Generated/../../../../Source/SyntaxBase.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/CalculatorExprAst_Builder.o: ../../Source/Calculator/Generated/CalculatorExprAst_Builder.cpp ../../Source/Calculator/Generated/CalculatorExprAst_Builder.h ../../Source/Calculator/Generated/CalculatorExprAst.h ../../Source/Calculator/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/Calculator/Generated/../../../../Source/SyntaxBase.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/CalculatorExprAst_Copy.o: ../../Source/Calculator/Generated/CalculatorExprAst_Copy.cpp ../../Source/Calculator/Generated/CalculatorExprAst_Copy.h ../../Source/Calculator/Generated/CalculatorExprAst.h ../../Source/Calculator/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/Calculator/Generated/../../../../Source/SyntaxBase.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/CalculatorExprAst_Empty.o: ../../Source/Calculator/Generated/CalculatorExprAst_Empty.cpp ../../Source/Calculator/Generated/CalculatorExprAst_Empty.h ../../Source/Calculator/Generated/CalculatorExprAst.h ../../Source/Calculator/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/Calculator/Generated/../../../../Source/SyntaxBase.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/CalculatorExprAst_Json.o: ../../Source/Calculator/Generated/CalculatorExprAst_Json.cpp ../../Source/Calculator/Generated/CalculatorExprAst_Json.h ../../Source/Calculator/Generated/CalculatorExprAst.h ../../Source/Calculator/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/Calculator/Generated/../../../../Source/SyntaxBase.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/CalculatorExprAst_Traverse.o: ../../Source/Calculator/Generated/CalculatorExprAst_Traverse.cpp ../../Source/Calculator/Generated/CalculatorExprAst_Traverse.h ../../Source/Calculator/Generated/CalculatorExprAst.h ../../Source/Calculator/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/Calculator/Generated/../../../../Source/SyntaxBase.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/CalculatorModuleParser.o: ../../Source/Calculator/Generated/CalculatorModuleParser.cpp ../../Source/Calculator/Generated/CalculatorModuleParser.h ../../Source/Calculator/Generated/Calculator_Assembler.h ../../Source/Calculator/Generated/CalculatorExprAst.h ../../Source/Calculator/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/Calculator/Generated/../../../../Source/SyntaxBase.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/../Executable.h ../../Source/Calculator/Generated/Calculator_Lexer.h
	$(CPP_COMPILE)

./Obj/Calculator_Assembler.o: ../../Source/Calculator/Generated/Calculator_Assembler.cpp ../../Source/Calculator/Generated/Calculator_Assembler.h ../../Source/Calculator/Generated/CalculatorExprAst.h ../../Source/Calculator/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/Calculator/Generated/../../../../Source/SyntaxBase.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/Calculator_Lexer.o: ../../Source/Calculator/Generated/Calculator_Lexer.cpp ../../Source/Calculator/Generated/Calculator_Lexer.h ../../Source/Calculator/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/Calculator/Generated/../../../../Source/SyntaxBase.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/Calculator/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/FeatureTestFeatureAst.o: ../../Source/FeatureTest/Generated/FeatureTestFeatureAst.cpp ../../Source/FeatureTest/Generated/FeatureTestFeatureAst.h ../../Source/FeatureTest/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/FeatureTest/Generated/../../../../Source/SyntaxBase.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/FeatureTestFeatureAst_Builder.o: ../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Builder.cpp ../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Builder.h ../../Source/FeatureTest/Generated/FeatureTestFeatureAst.h ../../Source/FeatureTest/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/FeatureTest/Generated/../../../../Source/SyntaxBase.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/FeatureTestFeatureAst_Copy.o: ../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Copy.cpp ../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Copy.h ../../Source/FeatureTest/Generated/FeatureTestFeatureAst.h ../../Source/FeatureTest/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/FeatureTest/Generated/../../../../Source/SyntaxBase.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/FeatureTestFeatureAst_Empty.o: ../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Empty.cpp ../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Empty.h ../../Source/FeatureTest/Generated/FeatureTestFeatureAst.h ../../Source/FeatureTest/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/FeatureTest/Generated/../../../../Source/SyntaxBase.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/FeatureTestFeatureAst_Json.o: ../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Json.cpp ../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Json.h ../../Source/FeatureTest/Generated/FeatureTestFeatureAst.h ../../Source/FeatureTest/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/FeatureTest/Generated/../../../../Source/SyntaxBase.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/FeatureTestFeatureAst_Traverse.o: ../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Traverse.cpp ../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Traverse.h ../../Source/FeatureTest/Generated/FeatureTestFeatureAst.h ../../Source/FeatureTest/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/FeatureTest/Generated/../../../../Source/SyntaxBase.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/FeatureTestModuleParser.o: ../../Source/FeatureTest/Generated/FeatureTestModuleParser.cpp ../../Source/FeatureTest/Generated/FeatureTestModuleParser.h ../../Source/FeatureTest/Generated/FeatureTest_Assembler.h ../../Source/FeatureTest/Generated/FeatureTestFeatureAst.h ../../Source/FeatureTest/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/FeatureTest/Generated/../../../../Source/SyntaxBase.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/../Executable.h ../../Source/FeatureTest/Generated/FeatureTest_Lexer.h
	$(CPP_COMPILE)

./Obj/FeatureTest_Assembler.o: ../../Source/FeatureTest/Generated/FeatureTest_Assembler.cpp ../../Source/FeatureTest/Generated/FeatureTest_Assembler.h ../../Source/FeatureTest/Generated/FeatureTestFeatureAst.h ../../Source/FeatureTest/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/FeatureTest/Generated/../../../../Source/SyntaxBase.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/FeatureTest_Lexer.o: ../../Source/FeatureTest/Generated/FeatureTest_Lexer.cpp ../../Source/FeatureTest/Generated/FeatureTest_Lexer.h ../../Source/FeatureTest/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/FeatureTest/Generated/../../../../Source/SyntaxBase.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/FeatureTest/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/GenericAmbiguityExprAst.o: ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst.cpp ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst.h ../../Source/GenericAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/GenericAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/GenericAmbiguityExprAst_Builder.o: ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Builder.cpp ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Builder.h ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst.h ../../Source/GenericAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/GenericAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/GenericAmbiguityExprAst_Copy.o: ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Copy.cpp ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Copy.h ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst.h ../../Source/GenericAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/GenericAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/GenericAmbiguityExprAst_Empty.o: ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Empty.cpp ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Empty.h ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst.h ../../Source/GenericAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/GenericAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/GenericAmbiguityExprAst_Json.o: ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Json.cpp ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Json.h ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst.h ../../Source/GenericAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/GenericAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/GenericAmbiguityExprAst_Traverse.o: ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Traverse.cpp ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Traverse.h ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst.h ../../Source/GenericAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/GenericAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/GenericAmbiguityModuleParser.o: ../../Source/GenericAmbiguity/Generated/GenericAmbiguityModuleParser.cpp ../../Source/GenericAmbiguity/Generated/GenericAmbiguityModuleParser.h ../../Source/GenericAmbiguity/Generated/GenericAmbiguity_Assembler.h ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst.h ../../Source/GenericAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/GenericAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h ../../Source/GenericAmbiguity/Generated/GenericAmbiguity_Lexer.h
	$(CPP_COMPILE)

./Obj/GenericAmbiguity_Assembler.o: ../../Source/GenericAmbiguity/Generated/GenericAmbiguity_Assembler.cpp ../../Source/GenericAmbiguity/Generated/GenericAmbiguity_Assembler.h ../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst.h ../../Source/GenericAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/GenericAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/GenericAmbiguity_Lexer.o: ../../Source/GenericAmbiguity/Generated/GenericAmbiguity_Lexer.cpp ../../Source/GenericAmbiguity/Generated/GenericAmbiguity_Lexer.h ../../Source/GenericAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/GenericAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/GenericAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElseAmbiguityModuleParser.o: ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityModuleParser.cpp ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityModuleParser.h ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguity_Assembler.h ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguity_Lexer.h
	$(CPP_COMPILE)

./Obj/IfElseAmbiguityStatAst.o: ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst.cpp ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElseAmbiguityStatAst_Builder.o: ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Builder.cpp ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Builder.h ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElseAmbiguityStatAst_Copy.o: ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Copy.cpp ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Copy.h ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElseAmbiguityStatAst_Empty.o: ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Empty.cpp ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Empty.h ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElseAmbiguityStatAst_Json.o: ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Json.cpp ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Json.h ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElseAmbiguityStatAst_Traverse.o: ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Traverse.cpp ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Traverse.h ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElseAmbiguity_Assembler.o: ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguity_Assembler.cpp ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguity_Assembler.h ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElseAmbiguity_Lexer.o: ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguity_Lexer.cpp ../../Source/IfElseAmbiguity/Generated/IfElseAmbiguity_Lexer.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElseAmbiguity/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElsePriorityModuleParser.o: ../../Source/IfElsePriority/Generated/IfElsePriorityModuleParser.cpp ../../Source/IfElsePriority/Generated/IfElsePriorityModuleParser.h ../../Source/IfElsePriority/Generated/IfElsePriority_Assembler.h ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst.h ../../Source/IfElsePriority/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElsePriority/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/../Executable.h ../../Source/IfElsePriority/Generated/IfElsePriority_Lexer.h
	$(CPP_COMPILE)

./Obj/IfElsePriorityStatAst.o: ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst.cpp ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst.h ../../Source/IfElsePriority/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElsePriority/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElsePriorityStatAst_Builder.o: ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Builder.cpp ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Builder.h ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst.h ../../Source/IfElsePriority/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElsePriority/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElsePriorityStatAst_Copy.o: ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Copy.cpp ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Copy.h ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst.h ../../Source/IfElsePriority/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElsePriority/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElsePriorityStatAst_Empty.o: ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Empty.cpp ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Empty.h ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst.h ../../Source/IfElsePriority/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElsePriority/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElsePriorityStatAst_Json.o: ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Json.cpp ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Json.h ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst.h ../../Source/IfElsePriority/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElsePriority/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElsePriorityStatAst_Traverse.o: ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Traverse.cpp ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Traverse.h ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst.h ../../Source/IfElsePriority/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElsePriority/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElsePriority_Assembler.o: ../../Source/IfElsePriority/Generated/IfElsePriority_Assembler.cpp ../../Source/IfElsePriority/Generated/IfElsePriority_Assembler.h ../../Source/IfElsePriority/Generated/IfElsePriorityStatAst.h ../../Source/IfElsePriority/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElsePriority/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/IfElsePriority_Lexer.o: ../../Source/IfElsePriority/Generated/IfElsePriority_Lexer.cpp ../../Source/IfElsePriority/Generated/IfElsePriority_Lexer.h ../../Source/IfElsePriority/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../Source/IfElsePriority/Generated/../../../../Source/SyntaxBase.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/TraceManager.h ../../Source/IfElsePriority/Generated/../../../../Source/TraceManager/../Executable.h
	$(CPP_COMPILE)

./Obj/LogTrace.o: ../../Source/LogTrace.cpp ../../Source/LogTrace.h ../../Source/../../Source/SyntaxBase.h ../../Source/../../Source/TraceManager/TraceManager.h ../../Source/../../Source/TraceManager/../Executable.h ../../Source/../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/CommonMain.o: ../../UnitTest/CommonMain.cpp ../../../Import/VlppOS.h ../../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/TestParser_Generated.o: ../../UnitTest/ParserTest_ParserGen_Generated/TestParser_Generated.cpp ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/Calculator/Generated/CalculatorExprAst_Json.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/Calculator/Generated/CalculatorExprAst.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/Calculator/Generated/../../../../Source/AstBase.h ../../../Import/VlppReflection.h ../../../Import/VlppOS.h ../../../Import/Vlpp.h ../../../Import/VlppRegex.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/Calculator/Generated/../../../../Source/SyntaxBase.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/Calculator/Generated/../../../../Source/TraceManager/TraceManager.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/Calculator/Generated/../../../../Source/TraceManager/../Executable.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/Calculator/Generated/CalculatorModuleParser.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/Calculator/Generated/Calculator_Assembler.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/Calculator/Generated/Calculator_Lexer.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst_Json.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityStatAst.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElseAmbiguity/Generated/../../../../Source/AstBase.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElseAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElseAmbiguity/Generated/IfElseAmbiguityModuleParser.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElseAmbiguity/Generated/IfElseAmbiguity_Assembler.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElseAmbiguity/Generated/IfElseAmbiguity_Lexer.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElsePriority/Generated/IfElsePriorityStatAst_Json.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElsePriority/Generated/IfElsePriorityStatAst.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElsePriority/Generated/../../../../Source/AstBase.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElsePriority/Generated/../../../../Source/SyntaxBase.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElsePriority/Generated/IfElsePriorityModuleParser.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElsePriority/Generated/IfElsePriority_Assembler.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/IfElsePriority/Generated/IfElsePriority_Lexer.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst_Json.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/GenericAmbiguity/Generated/GenericAmbiguityExprAst.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/GenericAmbiguity/Generated/../../../../Source/AstBase.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/GenericAmbiguity/Generated/../../../../Source/SyntaxBase.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/GenericAmbiguity/Generated/GenericAmbiguityModuleParser.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/GenericAmbiguity/Generated/GenericAmbiguity_Assembler.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/GenericAmbiguity/Generated/GenericAmbiguity_Lexer.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/FeatureTest/Generated/FeatureTestFeatureAst_Json.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/FeatureTest/Generated/FeatureTestFeatureAst.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/FeatureTest/Generated/../../../../Source/AstBase.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/FeatureTest/Generated/../../../../Source/SyntaxBase.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/FeatureTest/Generated/FeatureTestModuleParser.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/FeatureTest/Generated/FeatureTest_Assembler.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/FeatureTest/Generated/FeatureTest_Lexer.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/BinaryOp/Generated/BinaryOpExprAst_Json.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/BinaryOp/Generated/BinaryOpExprAst.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/BinaryOp/Generated/../../../../Source/AstBase.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/BinaryOp/Generated/../../../../Source/SyntaxBase.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/BinaryOp/Generated/BinaryOpModuleParser.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/BinaryOp/Generated/BinaryOp_Assembler.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/BinaryOp/Generated/BinaryOp_Lexer.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/LogTrace.h ../../UnitTest/ParserTest_ParserGen_Generated/../../Source/../../Source/SyntaxBase.h
	$(CPP_COMPILE)
