CC        := gcc
LD        := gcc

SRC_DIR   := .
BUILD_DIR := build

SRC       := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ       := $(patsubst src/%.c,build/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(SRC_DIR))

vpath %.c $(SRC_DIR)

define make-goal
$1/%.o: %.c
	$(CC) $(INCLUDES) -Wall -g -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs SimpleProjectTODO.exe

SimpleProjectTODO.exe: $(OBJ)
	$(LD) $^ -o $@ -lwsock32


checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir $@

clean:
	@erase /F /Q $(BUILD_DIR)

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))

