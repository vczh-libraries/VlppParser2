.PHONY: all clean pre-build
.DEFAULT_GOAL := all

CPP_COMPILE_OPTIONS=-I ../../Import -I ../../Release
include $(VCPROOT)/vl/makefile-cpp

pre-build:
	if ! [ -d ./Bin ]; then mkdir ./Bin; fi
	if ! [ -d ./Obj ]; then mkdir ./Obj; fi
	if ! [ -d ./Coverage ]; then mkdir ./Coverage; fi

clean:
	if [ -d ./Bin ]; then rm -r ./Bin; fi
	if [ -d ./Obj ]; then rm -r ./Obj; fi
	if [ -d ./Coverage ]; then rm -r ./Coverage; fi

all:pre-build ./Bin/CodePack

./Bin/CodePack:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppReflection.o ./Obj/VlppRegex.o ./Obj/VlppParser.o ./Obj/Codepack_CategorizeCodeFiles.o ./Obj/Codepack_Combine.o ./Obj/Codepack_GetFiles.o ./Obj/Codepack_GetIncludeFiles.o ./Obj/Main.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../Import/Vlpp.cpp ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/Vlpp.Linux.o: ../../Import/Vlpp.Linux.cpp ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../Import/VlppOS.cpp ../../Import/VlppOS.h ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppOS.Linux.o: ../../Import/VlppOS.Linux.cpp ../../Import/VlppOS.h ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppReflection.o: ../../Import/VlppReflection.cpp ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/VlppRegex.o: ../../Import/VlppRegex.cpp ../../Import/VlppRegex.h ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppParser.o: ../../Release/VlppParser.cpp ../../Release/VlppParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/Codepack_CategorizeCodeFiles.o: CodePack/Codepack_CategorizeCodeFiles.cpp CodePack/Codepack.h ../../Release/VlppParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/Codepack_Combine.o: CodePack/Codepack_Combine.cpp CodePack/Codepack.h ../../Release/VlppParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/Codepack_GetFiles.o: CodePack/Codepack_GetFiles.cpp CodePack/Codepack.h ../../Release/VlppParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/Codepack_GetIncludeFiles.o: CodePack/Codepack_GetIncludeFiles.cpp CodePack/Codepack.h ../../Release/VlppParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/Main.o: CodePack/Main.cpp CodePack/Codepack.h ../../Release/VlppParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)
