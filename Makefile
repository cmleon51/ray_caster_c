CC := gcc
CFLAGS ?= -std=c23 -Wall -Wextra -O2 -I${HDRDIR}

TARGET := ray_caster
SRCDIR := src
HDRDIR := headers
OBJDIR := obj

PKG_CONFIG := $(shell command -v pkg-config 2>/dev/null)

ifeq ($(PKG_CONFIG),)
    SDL_CONFIG := $(shell command -v sdl3-config 2>/dev/null)
else
    SDL_CONFIG :=
endif

ifeq ($(SDL_CONFIG),)
    CFLAGS += $(shell pkg-config --cflags sdl3)
    LIBS := -ldl $(shell pkg-config --libs sdl3)
else
    CFLAGS += $(shell sdl3-config --cflags)
    LIBS := -ldl $(shell sdl3-config --libs)
endif

LDFLAGS := $(LIBS) -lm
LDLIBS :=

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
