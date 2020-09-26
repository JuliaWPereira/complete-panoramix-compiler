INPUT_NAME = sort_old

all:
	(cd Sources;make all)
	./panoramix < Inputs/$(INPUT_NAME).txt
	mkdir -p Outputs/$(INPUT_NAME)
	mv analysis.txt ./Outputs/$(INPUT_NAME)/analysis.txt
	mv symtable.txt ./Outputs/$(INPUT_NAME)/symtable.txt
	mv ast.txt ./Outputs/$(INPUT_NAME)/ast.txt
	mv interCode.txt ./Outputs/$(INPUT_NAME)/interCode.txt
	(cd Sources;make clean)
	(cd SyntesisSources;make all)
	./assembler Outputs/$(INPUT_NAME)/interCode.txt
	mv assemblyCode.txt ./Outputs/$(INPUT_NAME)/assemblyCode.txt
	mv memorytab.txt ./Outputs/$(INPUT_NAME)/memorytab.txt

