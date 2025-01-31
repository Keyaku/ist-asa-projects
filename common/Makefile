### INSERT THIS MAKE FILE AT THE SAME LEVEL OF src/ AND bld/

# Directories
SRCDIR = src
OBJDIR = bld

# Compilers
CC  ?= gcc
CXX ?= g++

# Shared compilation flags
CFLAGS = -Wall -Wextra -Wunreachable-code
CFLAGS+= -Wshadow -Wpointer-arith -Wcast-qual -Wno-missing-braces -Winline
CFLAGS+= -Wno-missing-field-initializers

LDFLAGS = -lm

BUILD ?= release
ifeq ($(BUILD), debug)
	CFLAGS += -g -O0 -DDEBUG
else ifeq ($(BUILD), time)
	CFLAGS += -std=c99 -DASA_BENCHMARK
else
	CFLAGS += -O3
endif

# Specific flags
CCFLAGS = $(CFLAGS) -ansi -pedantic
CXFLAGS = $(CFLAGS) -std=c++11

# Executables
EXEC_PROJ = $(OBJDIR)/proj
EXEC_PROJPP = $(OBJDIR)/projpp
EXECS = $(EXEC_PROJ) $(EXEC_PROJPP)
TARGET ?= $(EXEC_PROJ)


# General rules (point to main focus files)
all: $(TARGET)

clean:
	rm -rf $(EXECS) $(OBJDIR)/*.o $(OBJDIR)/*.dSYM

# Specific rules
proj: $(EXEC_PROJ)
projpp: $(EXEC_PROJPP)

# Compilation rules
$(EXEC_PROJ): $(SRCDIR)/proj.c
	@$(shell mkdir -p $(OBJDIR))
	$(CC) $(CCFLAGS) $^ -o $@ $(LDFLAGS)

$(EXEC_PROJPP): $(SRCDIR)/proj.cpp
	@$(shell mkdir -p $(OBJDIR))
	$(CXX) $(CXFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: all clean debug test valgrind
