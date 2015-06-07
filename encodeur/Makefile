#Repertoires du projet

OBJ_DIR = obj
SRC_DIR = src
INC_DIR = include

# Options de compilation

CC = gcc
LD = gcc
INC = -I$(INC_DIR)
CFLAGS = $(INC) -Wall -std=c99 -O0 -Wextra -g #-O3
LDFLAGS = -lm

# Liste des objets
OBJ_FILES = $(OBJ_DIR)/main.o
OBJ_FILES += $(OBJ_DIR)/qzz.o
OBJ_FILES += $(OBJ_DIR)/dct.o
OBJ_FILES += $(OBJ_DIR)/conv.o
OBJ_FILES += $(OBJ_DIR)/downsampler.o
OBJ_FILES += $(OBJ_DIR)/huffman.o
OBJ_FILES += $(OBJ_DIR)/pack.o
OBJ_FILES += $(OBJ_DIR)/bitstream.o
OBJ_FILES += $(OBJ_DIR)/jpeg.o
OBJ_FILES += $(OBJ_DIR)/utils.o

all : tiff2jpeg

# Edition de lien des executables
tiff2jpeg : $(OBJ_FILES)
	$(LD) -o tiff2jpeg $(OBJ_FILES) $(LDFLAGS)

# Compilation des sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f tiff2jpeg $(OBJ_FILES) # tests/*.tiff
