CC := gcc
CFLAGS ?= -std=c23 -Wall -Wextra -O2

TARGET := ray_caster
SRCDIR := src
HDRDIR := headers
OBJDIR := obj

ALL_BACKENDS := $(notdir $(wildcard $(SRCDIR)/platform/*))

PROBE_sdl3 := pkg-config --exists sdl3 || command -v sdl3-config

BACKEND_PREFERENCE := sdl3
ORDERED_BACKENDS := $(filter $(ALL_BACKENDS),$(BACKEND_PREFERENCE)) \
                    $(filter-out $(BACKEND_PREFERENCE),$(ALL_BACKENDS))

probe_backend = $(if $(PROBE_$(1)),$(shell { $(PROBE_$(1)); } >/dev/null 2>&1 && echo $(1)))
DETECTED_BACKENDS := $(strip $(foreach b,$(ORDERED_BACKENDS),$(call probe_backend,$(b))))

BACKEND ?= $(firstword $(DETECTED_BACKENDS))

BACKENDLESS_GOALS := backends clean

ifeq ($(filter $(BACKENDLESS_GOALS),$(MAKECMDGOALS)),)
    ifeq ($(BACKEND),)
        $(error No usable backend detected. Implementations present: $(ALL_BACKENDS). \
For sdl3, install the SDL3 development files so that `pkg-config --exists sdl3` \
or `sdl3-config` succeeds. Run `make backends` to see what was probed, or pass \
BACKEND=<name> to force one)
    endif
endif

PLATFORM_DIR := $(SRCDIR)/platform/$(BACKEND)

ifeq ($(wildcard $(PLATFORM_DIR)),)
    $(error Unknown BACKEND '$(BACKEND)'; available: $(ALL_BACKENDS))
endif

ifeq ($(BACKEND),sdl3)
    ifeq ($(shell pkg-config --exists sdl3 >/dev/null 2>&1 && echo yes),yes)
        BACKEND_CFLAGS := $(shell pkg-config --cflags sdl3)
        BACKEND_LIBS := -ldl $(shell pkg-config --libs sdl3)
    else ifneq ($(shell command -v sdl3-config 2>/dev/null),)
        BACKEND_CFLAGS := $(shell sdl3-config --cflags)
        BACKEND_LIBS := -ldl $(shell sdl3-config --libs)
    else
        $(error BACKEND=sdl3 was requested, but SDL3 was not found; tried \
`pkg-config --exists sdl3` and `sdl3-config`)
    endif
endif

CFLAGS += $(BACKEND_CFLAGS)
LDFLAGS := $(BACKEND_LIBS) -lm

CORE_SRCS := $(shell find $(SRCDIR) -name '*.c' -not -path '$(SRCDIR)/platform/*')
PLATFORM_SRCS := $(wildcard $(PLATFORM_DIR)/*.c)
SRCS := $(CORE_SRCS) $(PLATFORM_SRCS)

OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

INCLUDES := $(shell find $(HDRDIR) -type d)
INC_FLAGS := $(addprefix -I, $(INCLUDES))

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_FLAGS) -MMD -MP -c -o $@ $<

clean:
	rm -rf $(OBJDIR) $(TARGET)

backends:
	@echo "selected:  $(if $(BACKEND),$(BACKEND),<none>)"
	@echo "detected:  $(if $(DETECTED_BACKENDS),$(DETECTED_BACKENDS),<none>)"
	@echo "available: $(ALL_BACKENDS)"

-include $(DEPS)

.PHONY: clean backends
