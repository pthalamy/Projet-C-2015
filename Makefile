#Repertoires du projet

OBJ_DIR = bin
NEW_OBJ_DIR = obj
SRC_DIR = src
INC_DIR = include
TEST_DIR = tests

# Options de compilation

CC = gcc
LD = gcc
INC = -I$(INC_DIR)
CFLAGS = $(INC) -Wall -std=c99 -O0 -Wextra -g -DLOEFFLER # -O3
LDFLAGS = -lm

# Liste des objets encadrants

# OBJ_FILES = $(OBJ_DIR)/main.o
# OBJ_FILES += $(OBJ_DIR)/iqzz.o
# OBJ_FILES += $(OBJ_DIR)/idct.o
# OBJ_FILES += $(OBJ_DIR)/conv.o
# OBJ_FILES += $(OBJ_DIR)/upsampler.o
# OBJ_FILES += $(OBJ_DIR)/huffman.o
# OBJ_FILES += $(OBJ_DIR)/unpack.o
#OBJ_FILES += $(OBJ_DIR)/bitstream.o
# OBJ_FILES += $(OBJ_DIR)/tiff.o

# Liste des objets realises
NEW_OBJ_FILES = $(NEW_OBJ_DIR)/main.o
NEW_OBJ_FILES += $(NEW_OBJ_DIR)/iqzz.o
NEW_OBJ_FILES += $(NEW_OBJ_DIR)/conv.o
NEW_OBJ_FILES += $(NEW_OBJ_DIR)/idct.o
NEW_OBJ_FILES += $(NEW_OBJ_DIR)/upsampler.o
NEW_OBJ_FILES += $(NEW_OBJ_DIR)/huffman.o
NEW_OBJ_FILES += $(NEW_OBJ_DIR)/unpack.o
NEW_OBJ_FILES += $(NEW_OBJ_DIR)/bitstream.o
NEW_OBJ_FILES += $(NEW_OBJ_DIR)/tiff.o

# Liste des images de test
TEST_FILES = $(wildcard $(TEST_DIR)/*.jp*)

TEST_OUTPUTS1 = $(TEST_FILES:.jpeg=.tiff)
TEST_OUTPUTS += $(TEST_OUTPUTS1:.jpg=.tiff)

all: jpeg2tiff

# Edition de lien des executables
jpeg2tiff: $(OBJ_FILES) $(NEW_OBJ_FILES)
	$(LD) -o jpeg2tiff $(OBJ_FILES) $(NEW_OBJ_FILES) $(LDFLAGS)

# Compilation des sources
$(NEW_OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h
	$(CC) -c $(CFLAGS) $< -o $@

$(NEW_OBJ_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) -c $(CFLAGS) $< -o $@

# Décodage des images de test
$(TEST_DIR)/%.tiff: $(TEST_DIR)/%.jpeg jpeg2tiff
	./jpeg2tiff $<

$(TEST_DIR)/%.tiff: $(TEST_DIR)/%.jpg jpeg2tiff
	./jpeg2tiff $<

.PHONY: clean realclean tests
tests: $(TEST_OUTPUTS)

clean:
	rm -f jpeg2tiff $(NEW_OBJ_FILES)

realclean: clean
	rm -f tests/*.tiff
