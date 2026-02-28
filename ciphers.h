#pragma once
#include <cstdint> 

#ifdef _MSC_VER
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 1. Шифр Атбаш (зеркальная замена)
EXPORT void atbash(char* data, long size);

// 2. Двойная перестановка (возвращает новый буфер)
EXPORT char* doubleTransposition(const char* data, long size, const char* colKey, const char* rowKey, bool encrypt, long& newSize);

// Вспомогательная функция проверки ключа
bool isValidKey(const char* key, int length);

// 3. TEA (Tiny Encryption Algorithm)
EXPORT char* teaProcess(const char* data, long size, const char* password, bool encrypt, long& newSize);

#ifdef __cplusplus
}
#endif