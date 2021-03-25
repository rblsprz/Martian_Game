INCLUDE_DIR = ./include


ALLEGRO_FLAGS := $(shell pkg-config allegro-5 allegro_font-5 allegro_image-5 allegro_dialog-5 --libs --cflags)
P_THREAD = -pthread
CC = gcc
APP_NAME = ALIENGAME

all: buildApp

buildApp: $(APP_NAME).c
		gcc -I $(INCLUDE_DIR) $(APP_NAME).c -o PROYECTO $(APP_NAME) $(P_THREAD) $(ALLEGRO_FLAGS)


