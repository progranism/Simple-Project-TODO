CC        := gcc
LD        := gcc

SRC_DIR   := .
BUILD_DIR := build

DATAFILES := $(foreach sdir,data,$(wildcard $(sdir)/*.png)) $(wildcard data/*.html) $(wildcard data/*.js)
DATAFILES := $(patsubst data/%,%,$(DATAFILES))
SRC       := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ       := $(patsubst src/%.c,build/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(SRC_DIR))

vpath %.c $(SRC_DIR)

define make-goal
$1/%.o: %.c datapack.h
	$(CC) $(INCLUDES) -Wall -g -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs SimpleProjectTODO.exe FileToDatapack.exe

datapack.h: FileToDatapack.exe
	cd data && ..\FileToDatapack.exe $(DATAFILES) > ..\datapack.h

SimpleProjectTODO.exe: build/main.o
	$(LD) $^ -o $@ -lwsock32

FileToDatapack.exe: build/file_to_datapack.o
	$(LD) $^ -o $@


checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir $@

clean:
	@erase /F /Q $(BUILD_DIR)

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))

