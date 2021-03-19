# fichier Makefile 

CPP=g++#compiler
LDFLAGS=-lmingw32 -lSDLmain -lSDL -lSDL_image -lSDL_mixer  -liconv -lavfilter -lavformat -lavcodec -lswscale -lavutil -lswresample -luser32 -lgdi32 -lwinmm -ldxguid 
LDLIBS =-IC:/MinGW/include -IC:/MinGW/include/ffmpeg -LC:/MinGW/lib/ffmpeg -LC:/MinGW/lib/SDL -LC:/MinGW/lib/SDL_image -LC:/MinGW/lib/SDL_mixer
EXEC=ffmpeg#executable name
SOURCES=main.cpp VideoPlayer.cpp
.DEFAULT_GOAL =help  

.PHONY: clean mrproper 

help: script.sh
	./script.sh

compile: $(SOURCES) 		#compiles the program
	${CPP} ${LDLIBS} ${SOURCES} -o ${EXEC} ${LDFLAGS} -w -g
	
run: ${EXEC}			#starts the program
	./${EXEC}

clean:	
	rm -fr *.o

mrproper: clean			#delete the executable
	rm -fr $(EXEC)


