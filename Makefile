# ProjectNX - Nintendo Switch homebrew

ifeq ($(strip $(DEVKITPRO)),)
DEVKITPRO_MISSING := 1
else
include $(DEVKITPRO)/libnx/switch_rules
endif

PROJECT       := ProjectNX
TARGET        := ProjectNX
BUILD         := build
SOURCES       := source
INCLUDES      := include
APP_TITLE     := ProjectNX
APP_AUTHOR    := ProjectNX Contributors
APP_VERSION   := 0.3.1

ARCH          := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
CFLAGS        := -g -Wall -Wextra -Werror -O2 -ffunction-sections $(ARCH) \
                 $(shell curl-config --cflags 2>/dev/null)
CFLAGS        += $(INCLUDE) -D__SWITCH__ -DPROJECTNX_VERSION=\"$(APP_VERSION)\" -std=gnu11
ASFLAGS       := -g $(ARCH)
LDFLAGS       := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) \
                 -Wl,-Map,$(notdir $*.map)
LIBS          := $(shell curl-config --libs 2>/dev/null) -lnx
LIBDIRS       := $(PORTLIBS) $(LIBNX)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT  := $(CURDIR)/$(TARGET)
export TOPDIR  := $(CURDIR)
export VPATH   := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES         := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES         := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
OFILES_SRC     := $(CFILES:.c=.o) $(SFILES:.s=.o)

export LD       := $(CC)
export OFILES_SRC
export OFILES   := $(OFILES_SRC)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  -I$(CURDIR)/$(BUILD) \
                  -I$(LIBNX)/include \
                  -I$(PORTLIBS)/include
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export NROFLAGS += --icon=$(APP_ICON)
export NROFLAGS += --nacp=$(CURDIR)/$(TARGET).nacp

.PHONY: all clean package test validate toolchain-check $(BUILD)

all: toolchain-check $(BUILD)

toolchain-check:
ifdef DEVKITPRO_MISSING
	$(error DEVKITPRO não configurado. Instale devkitPro/devkitA64 e exporte DEVKITPRO)
endif

$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).nacp $(TARGET).nro $(TARGET).map dist

package: all
	@mkdir -p dist/projectnx/switch/projectnx
	@cp $(TARGET).nro dist/projectnx/switch/projectnx/projectnx.nro
	@cp README.md VERSION config.example.ini dist/projectnx/switch/projectnx/
	@cp config.example.ini dist/projectnx/switch/projectnx/config.ini
	@echo "Pacote criado em dist/projectnx"

test:
	@mkdir -p build/host
	@cc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
		source/app.c source/config.c source/json.c tests/test_app.c \
		-o build/host/test_app
	@cc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude -Itests/stubs \
		-DPROJECTNX_VERSION=\"$(APP_VERSION)\" -c source/main.c \
		-o build/host/main_switch_syntax.o
	@cc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude -Itests/stubs \
		$(shell curl-config --cflags 2>/dev/null) -c source/network.c \
		-o build/host/network_switch_syntax.o
	@cc -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
		$(shell curl-config --cflags 2>/dev/null) \
		-DPROJECTNX_VERSION=\"$(APP_VERSION)\" -c source/auth.c \
		-o build/host/auth_syntax.o
	@build/host/test_app

validate:
	@sh scripts/validate.sh

else

DEPENDS := $(OFILES:.o=.d)

.PHONY: all

all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp

$(OUTPUT).elf: $(OFILES)

$(OFILES_SRC):

-include $(DEPENDS)

endif
