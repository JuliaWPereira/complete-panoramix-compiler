INPUT_NAME = InterCodeTest

all:
	(cd Sources;make all)
	./panoramix < Inputs/$(INPUT_NAME).txt
	mkdir -p Outputs/$(INPUT_NAME)
	mv analysis.txt ./Outputs/$(INPUT_NAME)/analysis.txt
	mv symtable.txt ./Outputs/$(INPUT_NAME)/symtable.txt
	mv ast.txt ./Outputs/$(INPUT_NAME)/ast.txt
	mv interCode.txt ./Outputs/$(INPUT_NAME)/interCode.txt
	(cd Sources;make clean)
	

