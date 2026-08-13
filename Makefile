CC := gcc
CFLAGS ?= -Wall -Wextra -I${HDRDIR}
LDFLAGS :=

TARGET := ray_caster
SRCDIR := src
HDRDIR := headers
OBJDIR := obj

SRCS := $(shell find $(SRCDIR) -name "*.c")

OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

INCLUDES := $(shell find $(HDRDIR) -type d)
INC_FLAGS := $(addprefix -I, $(INCLUDES))

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(shell find $(HDRDIR) -name "*.h")
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_FLAGS) -MMD -MP -c -o $@ $<

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: clean
