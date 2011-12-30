.PHONY:all clean

EXEC:=game

all:$(EXEC)

clean:
	rm $(EXEC)

$(EXEC):tools.c timer.c menu.c menu_functions.c list.c game.c sigmsg.c msgs.c main.c
	gcc -g -lrt -pthread -o $@ $^ -Wall

tools.c:tools.h
timer.c:timer.h
menu.c:menu.h
menu_functions.c:menu_functions.h
list.c:list.h
game.c:game.h
sigmsg.c:sigmsg.h
msgs.c:msgs.h

