# #OBJS specifies which files to compile as part of the project
# OBJS = test.c 
# #CC specifies which compiler we're using
# CC = gcc
# #INCLUDE_PATHS specifies the additional include paths we'll need
# INCLUDE_PATHS = -I src\include
# #LIBRARY_PATHS specifies the additional library paths we'll need
# LIBRARY_PATHS = -L src\lib
# #COMPILER_FLAGS specifies the additional compilation options we're using
# # -w suppresses all warnings
# # -Wl,-subsystem,windows gets rid of the console window
# # -Wall get all warnings (use when dev game)
# COMPILER_FLAGS = -Wall #-w -Wl,-subsystem,windows
# #LINKER_FLAGS specifies the libraries we're linking against
# LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lws2_32 -lSDL2_image
# #OBJ_NAME specifies the name of our exectuable
# OBJ_NAME = test
# #This is the target that compiles our executable
# all : $(OBJS) 
# 	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
# clean:
# 	del test.exe

all :  server test
server2: server2.c
		gcc server2.c -Wall -pthread -lws2_32  -o server2
server: server.c
		gcc server.c -Wall -pthread -lws2_32  -o server 	
test2:  test2.c
	gcc test2.c -I src\include -L src\lib -Wall -pthread  -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_mixer -lws2_32 -lSDL2_image -o test2
test:  test.c
	gcc test.c -I src\include -L src\lib -Wall -pthread  -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_mixer -lws2_32 -lSDL2_image -o test
clean:
	del server.exe client.exe linklist.o packet.o player.o packet.h.gch linklist.h.gch player.h.gch client.o client.h.gch  main.exe
