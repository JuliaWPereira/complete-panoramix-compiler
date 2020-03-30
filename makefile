all:
	./panoramix < Inputs/test.txt
	mv symtab.txt ./Outputs/symtab.txt