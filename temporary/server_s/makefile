SUB_DIRS = display

OBJ_DIR = $(CURDIR)/../output
unexport OBJ_DIR


all: BUILD_DISPLAY
	gcc -o $(OBJ_DIR)/server_interface.o -c server_interface.c \
		$(CFLAGS) $(INCLUDE_FALGS)
	@echo 'server_interface finished'

BUILD_DISPLAY:
	@for displaydir in $(SUB_DIRS); do \
		$(MAKE) -C $$displaydir; \
		done

