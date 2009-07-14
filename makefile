CC = gcc

CFLAGS = -Wall -DWIN32
LDFLAGS = -lws2_32

TARGET = skypeeee.exe
OBJS = skypeeee.o strque.o httpd.o

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS) $(CFLAGS)
	strip $(TARGET)

clean :
	rm -f $(TARGET) *o *~

test : $(TARGET)
	.\\$(TARGET)

skypeeee.o : skypeeee.c

strque.o : strque.c strque.h

httpd.o : httpd.c httpd.h
