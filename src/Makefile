
# C compiler
CC = gcc

# C flags
CFLAGS = -g -Wall -Werror

# required libraries
LIBS =

SRCFILES = helpers.c main.c

# required object files
OBJFILES = $(SRCFILES:.c=.o)

# target executable
TARGET = sysprak-client

# default target
all: $(TARGET)

# linking
$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LIBS)

# compile (but don't link) source files
%.o: %.c
	$(CC) -c $(SRCFILES)

# clean-up
clean:
	rm -f $(OBJFILES) $(TARGET)



