# Учебный проект: Электронная таблица

# Описание проекта:	
Консольная таблица представляет собой упрощенный аналог листа Microsoft Excel, состоящий из ячеек.
В ячейках таблицы могут храниться текст, формулы и индексы других ячеек.
	
# Требования:
- Ubuntu 22.04.4 LTS
- GCC 11.4.0 
- ANTLR
- JDK
# ANTLR:
Инструкцию по установке ```ANTLR``` можно найти на сайте ```antlr.org```

Для компиляции кода понадобится библиотека ```ANTLR4 C++ Runtime```. Скачайте архив ```antlr4-cpp-runtime*.zip``` из раздела ```Download``` на сайте ```antlr.org```. Извлеките содержимое архива к остальным файлам.

Загрузите JAR-файл antlr*.jar к остальным файлам:
```
curl -O https://www.antlr.org/download/antlr-4.13.1-complete.jar
```
# Установка:
Создание папки ```build``` и переход в нее:
```
mkdir build && cd build
```
Сборка проекта:
```
cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build .
```
Запуск исполняемого файла: 
```
./spreadsheet
```
# Использование:
В файле ```main.cpp``` примеры использования.
# Технологии:
- ООП
- C++17 STL
	
# Планы по доработке:
Привести код в нормальное состояние

Расширение функционала
