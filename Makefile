TARGET = pictoro
SRC_DIR = src
CC = clang
CFLAGS = -Wall -Wextra -std=c11
LIBS = -lSDL2 -lm

.PHONY: default all clean

default: $(TARGET)
all: default

debug: CFLAGS += -DDEBUG_MODE -g
debug: default


OBJECTS = $(patsubst %.c, %.o, $(wildcard $(SRC_DIR)/*.c))
HEADERS = $(wildcard $(SRC_DIR)/*.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f $(SRC_DIR)/*.o
	-rm -f $(TARGET)
	-rm -f *.ppm

run: $(TARGET)
	./$(TARGET)

install: all
	cp $(TARGET) /usr/local/bin/$(TARGET)

uninstall:
	rm -f /usr/local/bin/$(TARGET)
