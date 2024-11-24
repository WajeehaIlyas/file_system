# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -Wall -Wextra -g -Iheaders  # Add the -I flag for header directory

# Source and Object Paths
SRCDIR = src
OBJDIR = build
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Output Binary
TARGET = file_system

# Default Rule
all: $(TARGET)

# Rule to Build the Target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Rule to Build Object Files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create Build Directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Clean Rule
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Phony Targets
.PHONY: all clean
