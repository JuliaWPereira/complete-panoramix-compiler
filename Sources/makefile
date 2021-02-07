PROJ_NAME = panoramix

all: $(PROJ_NAME)

$(PROJ_NAME): *.o
	gcc -g -o $@ *.o -ll -lfl
	mv $(PROJ_NAME) ../$(PROJ_NAME)

*.o: *.c $(PROJ_NAME).l $(PROJ_NAME).y
	flex $(PROJ_NAME).l
	bison -d -v $(PROJ_NAME).y
	gcc -g -c *.c

clean:
	rm  *.o $(PROJ_NAME).output $(PROJ_NAME).tab.c $(PROJ_NAME).tab.h lex.yy.c
