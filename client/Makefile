HEADERS = ppnfs_client.h

CC     = gcc

CFLAGS += -g -O0
# CFLAGS += -O3
CFLAGS += -pg
CFLAGS += -DHAVE_CONFIG_H -D_FILE_OFFSET_BITS=64

LCFLAGS += -lpthread -lrt -lm -ldl -lfuse -lgmp

OBJ 	= ppnfs_init.o ppnfs_metadata.o ppnfs_util.o ppnfs_client.o \
          ppnfs_mutex.o ppnfs_grpir.o Pohlig_hellman.o ppnfs_fuse.o ppnfs_net.o

TESTFILE 	= ppnfs_testsuit.c
TESTTARGET 	= ppnfs_testsuit
# Target object
TARGET	= ppnfs_client

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
	rm -f $(TARGET) $(OBJ) PPNFS_METAFILE

$(TARGET) : $(OBJ)
	$(CC) -o $@ $^ $(LCFLAGS)

# .cc.o :
# 	$(CC) $(CFLAGS) $<

.c.o: $(HEADERS) $<
	$(CC) -c $(CFLAGS) $(MODULECOMPILEFLAGS) -o $@ $<

# .c.o :
# 	$(CC) $(CFLAGS) $<

