CC := gcc
CFLAGS := -Wall -std=gnu11

DEPS := petri.h action.h utils.h cJSON.h
OBJS := main.o petri.o action.o utils.o cJSON.o

PROJ_NAME := petrinet

all: build

.PHONY: build clean

build: $(PROJ_NAME)

$(PROJ_NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f $(OBJS)
	rm -f $(PROJ_NAME)

