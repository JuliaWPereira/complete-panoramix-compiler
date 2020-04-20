all:
	./panoramix < Inputs/sort.txt
	mv analysis.txt ./Outputs/analysis.txt
	mv symtable.txt ./Outputs/symtable.txt
	mv ast.txt ./Outputs/ast.txt
	