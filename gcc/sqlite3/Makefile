
LIB_SRC=../../src/sqlite-3.19.3/sqlite3.c

LIB_TARGET = sqlite3.a

all: $(LIB_TARGET)

$(LIB_TARGET): $(LIB_SRC)
	$(CC) -c -g3 $(LIB_SRC)
	rm -f $(LIB_TARGET)
	ar vq $(LIB_TARGET) *.o

lib : $(LIB_TARGET)
	rm -f *.o

clean:
	rm -f $(LIB_TARGET) *.o 