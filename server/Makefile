HEADERS = ppnfs_server.h
CC 		= g++
OBJ 	= ppnfs_init.o ppnfs_metadata.o ppnfs_util.o ppnfs_net.o \
	      ppnfs_server.o ppnfs_mutex.o ppnfs_grpir.o Pohlig_hellman.o

CFLAGS += -g -O0
# CFLAGS += -O3
CFLAGS += -pg
CFLAGS += -DHAVE_CONFIG_H -W -D_FILE_OFFSET_BITS=64
LCFLAGS += -lpthread -lrt -ldl -lfuse -lgmp -lm

TESTFILE 	= ppnfs_testsuit.c
TESTTARGET 	= ppnfs_testsuit
# Target object
TARGET	= ppnfs_server

# All Phony Targets
.PHONY : everything clean

# Default starting position
everything : $(TARGET)

test: 
	$(CC) -o $(TESTTARGET) $(TESTFILE)

run:
	./$(TESTTARGET)
# Clean OBJECTS
clean :
	rm -rf $(TARGET) $(OBJ) err lerr gmon.out PPNFS_METAFILE

$(TARGET) : $(OBJ) $(HEADERS)
	$(CC) -o $@ $^ $(LCFLAGS)

.cc.o : $(HEADERS) $<
	$(CC) -c $(CFLAGS) $(LCFLAGS) $(MODULECOMPILEFLAGS) -o $@ $<

