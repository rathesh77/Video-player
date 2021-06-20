# fichier Makefile 

CPP=C:/MinGW/bin/g++#compiler
LDFLAGS=-lmingw32 -lSDLmain -lSDL -lSDL_image  -liconv -lavfilter -lavformat -lavcodec -lswscale -lavutil -lswresample -luser32 -lgdi32 -lwinmm -ldxguid 
LDLIBS =-IC:/SDL/include -IC:/Ffmpeg/include -LC:/Ffmpeg/lib -LC:/SDL/lib -LC:/SDL_image/lib
EXEC=ffmpeg#executable name
SOURCES=main.cpp VideoPlayer.cpp
.DEFAULT_GOAL =help  

.PHONY: clean mrproper 

help: script.sh
	./script.sh

compile: $(SOURCES) 		#compiles the program
	${CPP} ${LDLIBS} ${SOURCES} -o ${EXEC} ${LDFLAGS} -w -g
	
run: ${EXEC}.exe			#starts the program
	./${EXEC}

clean:	
	rm -fr *.o

mrproper: clean			#delete the executable
	rm -fr $(EXEC)


