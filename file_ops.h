#pragma once

// Макрос для экспорта функций
#ifdef _MSC_VER
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

// Читает файл в бинарном режиме. Возвращает буфер (требует free).
EXPORT char* readFile(const char* filename, long& size);

// Записывает бинарный буфер в файл.
EXPORT bool writeFile(const char* filename, const char* buffer, long size);

// Печатает содержимое файла на экран.
EXPORT void printFile(const char* filename);