# Variable definitions
MKDIR   = mkdir -p
RMDIR	= rm -rf
COPY	= cp
TAR		= tar -cf

PROG	= findpart
MAN		= manual.txt

CC      = mpic++

BIN     = ./bin
OBJ     = ./obj
INCLUDE = ./include
SRC     = ./src

SRCS    = $(wildcard $(SRC)/*.cpp)
OBJS    = $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SRCS))
EXE     = $(BIN)/$(PROG)

CFLAGS  = -I$(INCLUDE) -O3 -std=c++11
LDFLAGS =
LDLIBS  = -lm


# Phony targets
.PHONY: build rebuid clean tar


# Build target
build: $(EXE)
	$(info Building a program is complete. Executable file is located \
		in "$(BIN)" directory.)

# Rebuild target
rebuild: clean build

# Clean target
clean:
	$(info Removing a directories "$(OBJ)" and "$(BIN)"...)
	$(RMDIR) $(OBJ) $(BIN)

# Create "tar" target
tar:
	$(info Archiving the project...)
	$(TAR) $(PROG).tar $(INCLUDE) $(SRC) Makefile $(MAN)

# Creating directories target
$(BIN) $(OBJ):
	$(info Creating a directories "$(OBJ)" and "$(BIN)"...)
	$(MKDIR) $@

# Compilation target
$(OBJ)/%.o: $(SRC)/%.cpp | $(OBJ)
	$(info Compiling a "$<" file...)
	$(CC) $(CFLAGS) -c $< -o $@ $(LDLIBS)

# Linkage and copying manual target
$(EXE): $(OBJS) | $(BIN)
	for item in $^ ; do \
		echo "Linking a $$item file..." ; \
	done
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	$(info Copy the manual file "$(MAN)" to folder "$(BIN)"...)
	$(COPY) $(MAN) $(BIN)/$(MAN)
