CC = gcc
CPPFLAGS = -Iinclude
CFLAGS = -Wall -g -std=c11 -fPIC
LDFLAGS = -L./bin/ -lllist -lcparse

BIN = ./bin/
INC = ./include/
SRC = ./src/

all: parser

# targets for parser
parser: ../libcparse.so

../libcparse.so: $(BIN)VCardParser.o $(BIN)LinkedListAPI.o $(BIN)ParserFunctions.o
	gcc -shared -o ../libcparse.so $(BIN)VCardParser.o $(BIN)LinkedListAPI.o $(BIN)ParserFunctions.o

# targets for list library
#list: libllist.so

#libllist.so: $(BIN)LinkedListAPI.o
#	gcc -shared -o libllist.so $(BIN)LinkedListAPI.o

# object files

$(BIN)VCardParser.o: $(SRC)VCardParser.c $(INC)VCardParser.h $(INC)LinkedListAPI.h $(INC)ParserFunctions.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(SRC)VCardParser.c -o $(BIN)VCardParser.o

$(BIN)LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(SRC)LinkedListAPI.c -o $(BIN)LinkedListAPI.o
	
$(BIN)ParserFunctions.o: $(SRC)ParserFunctions.c $(INC)VCardParser.h $(INC)ParserFunctions.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(SRC)ParserFunctions.c -o $(BIN)ParserFunctions.o
	
# clean files
clean:
	rm -f $(BIN)*.o ../*.so
