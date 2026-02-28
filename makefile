# Компилятор
CXX = g++
CXXFLAGS = -Wall -fPIC -std=c++11

# Файлы
TARGET = crypto
FILE_LIB = libfile_ops.so
CIPHER_LIB = libciphers.so

all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): main.o $(FILE_LIB) $(CIPHER_LIB)
	$(CXX) main.o -o $(TARGET) -L. -lfile_ops -lciphers
	@echo "Сборка завершена: $(TARGET)"

# Библиотека файловых операций
$(FILE_LIB): file_ops.o
	$(CXX) -shared -o $(FILE_LIB) file_ops.o

# Библиотека шифров
$(CIPHER_LIB): ciphers.o
	$(CXX) -shared -o $(CIPHER_LIB) ciphers.o

# Компиляция объектов
main.o: main.cpp file_ops.h ciphers.h
	$(CXX) $(CXXFLAGS) -c main.cpp

file_ops.o: file_ops.cpp file_ops.h
	$(CXX) $(CXXFLAGS) -c file_ops.cpp

ciphers.o: ciphers.cpp ciphers.h
	$(CXX) $(CXXFLAGS) -c ciphers.cpp

clean:
	rm -f *.o *.so $(TARGET)
	@echo "Очистка выполнена."

run: all
	LD_LIBRARY_PATH=. ./$(TARGET)