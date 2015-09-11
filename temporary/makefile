CLIENT_DIRS = client_s
SERVER_DIRS = server_s

INCLUDE_DIRS = \
			   $(CURDIR) \
			   $(CURDIR)/client_s \
			   $(CURDIR)/server_s \
			   $(CURDIR)/client_s/cam_pack \
			   $(CURDIR)/server_s/display
INCLUDE_FALGS =  $(foreach incdir,$(INCLUDE_DIRS),-I$(incdir))
export INCLUDE_FALGS

OBJ_DIR =			$(CURDIR)/output
unexport OBJ_DIR

GFLAGS =			-m32
CLIENT_CFLAGS = 	-lpthread -m32
SERVER_CFLAGS = 	`pkg-config --cflags --libs gtk+-2.0`

CLIENT_OBJS = $(OBJ_DIR)/client.o $(OBJ_DIR)/client_interface.o $(OBJ_DIR)/camera.o $(OBJ_DIR)/BSD.o
SERVER_OBJS = $(OBJ_DIR)/server.o $(OBJ_DIR)/server_interface.o $(OBJ_DIR)/display.o

all:get_ready client server
	@echo 'Finish'

get_ready:
	-mkdir output


client: BUILD_CLIENT
	gcc -c client.c -o $(OBJ_DIR)/client.o $(INCLUDE_FALGS) $(CLIENT_CFLAGS)
	gcc -o client $(CLIENT_OBJS) $(CLIENT_CFLAGS)

server: BUILD_SERVER
	gcc -c server.c -o $(OBJ_DIR)/server.o $(INCLUDE_FALGS) $(SERVER_CFLAGS)
	gcc -o server $(SERVER_OBJS) $(SERVER_CFLAGS)

BUILD_CLIENT: 
	@for clidir in $(CLIENT_DIRS); do \
		$(MAKE) -C $$clidir; \
		done

BUILD_SERVER:
	@for serdir in $(SERVER_DIRS); do \
		$(MAKE) -C $$serdir; \
		done

test:
	@echo '$(CURDIR)'
	@echo '$(OBJ_DIR)'
	@echo '$(INCLUDE_FALGS)'


clean:
	-rm $(OBJ_DIR)/*.o


