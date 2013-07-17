CC 	= gcc
OBJ 	= datagen.c
# Target object
TARGET	= datagen

# All Phony Targets
.PHONY : everything clean

# Default starting position
everything : $(TARGET)

# Clean OBJECTS
clean : 
	rm -f $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) -o $@ $^ -lm -lgmp

