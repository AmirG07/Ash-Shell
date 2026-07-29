# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Iinclude

# Directories
SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib
BIN_DIR = bin

# Target names
TARGET = $(BIN_DIR)/ash
LIBRARY = $(LIB_DIR)/libash.a

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Everything except main.o goes into the library
LIB_OBJS = $(filter-out $(OBJ_DIR)/main.o,$(OBJS))

# Default target
all: $(TARGET)

# Build executable
$(TARGET): $(OBJ_DIR)/main.o $(LIBRARY)
	$(CC) $(OBJ_DIR)/main.o -L$(LIB_DIR) -lash -o $(TARGET)

# Build static library
$(LIBRARY): $(LIB_OBJS)
	ar rcs $(LIBRARY) $(LIB_OBJS)

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object files
clean:
	rm -f $(OBJ_DIR)/*.o

# Remove everything built
fclean: clean
	rm -f $(LIBRARY)
	rm -f $(TARGET)

# Rebuild
re: fclean all

.PHONY: all clean fclean re
