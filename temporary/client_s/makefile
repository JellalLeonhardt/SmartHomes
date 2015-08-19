CFLAGS = -m32

CAMERA_DIR = cam_pack

OBJ_DIR = $(CURDIR)/../output
unexport OBJ_DIR


all: BUILD_CAMERA
	gcc -o $(OBJ_DIR)/client_interface.o -c client_interface.c $(CFLAGS) $(INCLUDE_FALGS)
	gcc -o $(OBJ_DIR)/BSD.o -c BSD.c $(CFLAGS) $(INCLUDE_FALGS)
	@echo 'client finished.'

BUILD_CAMERA:
	@for camdir in $(CAMERA_DIR); do \
		$(MAKE) -C $$camdir; \
		done

test:
	@echo '$(OBJ_DIR)'


