CC := gcc
CFLAGS := -g -Wall
LDFLAGS := -ljson-c -lcurl
LDFLAGS_DEBUG := -lcurl


OBJDIR := obj

SRCS := $(wildcard *.c)

OBJECTS := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

TARGET_MAIN := main
TARGET_DEBUG := debug


MAIN_OBJECTS := $(filter-out $(OBJDIR)/debug.o, $(OBJECTS))
DEBUG_OBJECTS := $(OBJDIR)/debug.o $(OBJDIR)/fetch_data.o


all: $(TARGET_MAIN) $(TARGET_DEBUG)
.PHONY: all

$(TARGET_MAIN): $(MAIN_OBJECTS)
	$(CC) $(CFLAGS) $(MAIN_OBJECTS) -o $(TARGET_MAIN) $(LDFLAGS)


$(TARGET_DEBUG): $(DEBUG_OBJECTS)
	$(CC) $(CFLAGS) $(DEBUG_OBJECTS) -o $(TARGET_DEBUG) $(LDFLAGS_DEBUG)

$(OBJDIR)/%.o: %.c crawler.h Makefile 
	@mkdir -p $(OBJDIR) 
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(TARGET_MAIN) $(TARGET_DEBUG)
	@rm -rf $(OBJDIR) 
.PHONY: clean
