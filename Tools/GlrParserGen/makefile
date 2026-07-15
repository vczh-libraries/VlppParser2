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

all:pre-build ./Bin/GlrParserGen

./Bin/GlrParserGen:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppReflection.o ./Obj/VlppRegex.o ./Obj/VlppGlrParser.o ./Obj/VlppGlrParserCompiler.o ./Obj/Main.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../Import/Vlpp.cpp
	$(CPP_COMPILE)

./Obj/Vlpp.Linux.o: ../../Import/Vlpp.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../Import/VlppOS.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.Linux.o: ../../Import/VlppOS.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppReflection.o: ../../Import/VlppReflection.cpp
	$(CPP_COMPILE)

./Obj/VlppRegex.o: ../../Import/VlppRegex.cpp
	$(CPP_COMPILE)

./Obj/VlppGlrParser.o: ../../Release/VlppGlrParser.cpp
	$(CPP_COMPILE)

./Obj/VlppGlrParserCompiler.o: ../../Release/VlppGlrParserCompiler.cpp
	$(CPP_COMPILE)

./Obj/Main.o: GlrParserGen/Main.cpp
	$(CPP_COMPILE)
