#Repertoires du projet

OBJ_DIR = bin
NEW_OBJ_DIR = obj
SRC_DIR = src
INC_DIR = include

# Options de compilation

CC = gcc
LD = gcc
INC = -I$(INC_DIR)
CFLAGS = $(INC) -Wall -std=c99 -O3 #-Wextra -g #-O3
LDFLAGS = -lm

# Liste des objets encadrants

OBJ_FILES = $(OBJ_DIR)/main.o
#OBJ_FILES += $(OBJ_DIR)/iqzz.o
OBJ_FILES += $(OBJ_DIR)/idct.o
OBJ_FILES += $(OBJ_DIR)/conv.o
OBJ_FILES += $(OBJ_DIR)/upsampler.o
OBJ_FILES += $(OBJ_DIR)/huffman.o
OBJ_FILES += $(OBJ_DIR)/unpack.o
OBJ_FILES += $(OBJ_DIR)/bitstream.o
OBJ_FILES += $(OBJ_DIR)/tiff.o

# Liste des objets realises
# NEW_OBJ_FILES = $(NEW_NEW_OBJ_DIR)/conv.o
# NEW_OBJ_FILES = $(NEW_OBJ_DIR)/main.o
 NEW_OBJ_FILES += $(NEW_OBJ_DIR)/iqzz.o
# NEW_OBJ_FILES += $(NEW_OBJ_DIR)/idct.o
# NEW_OBJ_FILES += $(NEW_OBJ_DIR)/upsampler.o
# NEW_OBJ_FILES += $(NEW_OBJ_DIR)/huffman.o
# NEW_OBJ_FILES += $(NEW_OBJ_DIR)/unpack.o
# NEW_OBJ_FILES += $(NEW_OBJ_DIR)/bitstream.o
# NEW_OBJ_FILES += $(NEW_OBJ_DIR)/tiff.o

all : jpeg2tiff

# Edition de lien des executables
jpeg2tiff : $(OBJ_FILES) $(NEW_OBJ_FILES)
	$(LD) -o jpeg2tiff $(OBJ_FILES) $(NEW_OBJ_FILES) $(LDFLAGS)

# Compilation des sources
$(NEW_OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f jpeg2tiff $(NEW_OBJ_FILES) # tests/*.tiff
