# fichier Makefile 

CPP=g++#compilateur
LDFLAGS=-lmingw32 -lSDLmain -lSDL -lSDL_image -lSDL_mixer  -liconv -lavfilter -lavformat -lavcodec -lswscale -lavutil -lswresample -luser32 -lgdi32 -lwinmm -ldxguid 
LDLIBS =-IC:/MinGW/include -IC:/MinGW/include/ffmpeg -LC:/MinGW/lib/ffmpeg -LC:/MinGW/lib/SDL -LC:/MinGW/lib/SDL_image -LC:/MinGW/lib/SDL_mixer
EXEC=ffmpeg#Nom du programme 
SOURCES= main.cpp
FOLDER_ABSOLUTE_PATH= # !IMPORTANT specify a path here
.DEFAULT_GOAL = help  

.PHONY: clean mrproper 

help: script.sh
	./script.sh

compile: $(SOURCES) 		#Compiler le programme
	${CPP} ${LDLIBS} ${SOURCES} -o ${EXEC} ${LDFLAGS} -w -g
	
run: ${EXEC}			#executer le programme 
	./${EXEC} "${FOLDER_ABSOLUTE_PATH}"

clean:	
	rm -fr *.o

mrproper: clean			#supprimer l'executable
	rm -fr $(EXEC)


