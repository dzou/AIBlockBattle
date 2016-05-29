# Specify all the cpp files you need to compile the binary
# Binary will be named a.out by the all: target.
SOURCES = botParser.cpp botState.cpp shape.cpp botStarter.cpp bitField.cpp heuristic.cpp

OBJ_DIR = obj
OBJECTS = $(patsubst %, $(OBJ_DIR)/%.o, $(basename $(SOURCES)))

CC=g++-4.9
BASEFLAGS= -std=c++1y -Wall -Wno-sign-compare -O2
CPPFLAGS= -std=c++1y -Wall -Wno-sign-compare -MMD -MP -O2

all: $(OBJECTS)
	$(CC) $(CPPFLAGS) -o a.out $^

bitf:
	$(CC) $(BASEFLAGS) -g -o a.out bitField.cpp shape.cpp

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CPPFLAGS) -o $@ -c $<

# Compiles the binary fresh
simple:
	$(CC) $(BASEFLAGS) -o a.out $(SOURCES)

clean:
	rm a.out $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d

-include $(OBJECTS:%.o=%.d)
