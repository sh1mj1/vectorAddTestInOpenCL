CC = arm-linux-androideabi-gcc
ADB = adb

OPENCL_PATH = /code/w10/OpenCL_lib_and_include		#########################
CFLAG = -I$(OPENCL_PATH)/include -g					#########################
LDFLAGS = -l$(OPENCL_PATH)/lib/libGLES_mali.so -lm	#########################

TARGET = Vadd

TARGET_SRC = $(TARGET).c
TARGET_OBJ = $(TARGET).o

all: $(TARGET)

$(TARGET): $(TARGET_OBJ)
	$(CC) -static $(TARGET_OBJ) $(LDFLAGS) -o $(TARGET)	#########################
	echo
	echo "**** Install:" /data/local/tmp/$(TARGET) "****"
	$(ADB) push $(TARGET) /data/local/tmp
	$(ADB) push $(TARGET).cl /data/local/tmp
	$(ADB) shell chmod 755 /data/local/tmp/$(TARGET)

$(TARGET_OBJ): $(TARGET_SRC)
	$(CC) $(CFLAG) -c -o $(TARGET_OBJ) $(TARGET_SRC)
clean:
	rm -f *.o
	rm -f $(TARGET)
