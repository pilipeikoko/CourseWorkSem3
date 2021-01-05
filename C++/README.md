# Радиус взвешенного ориентированного графа.

## Как поставить пример

Для начала, необходимо поставить [остис ветки 0.5.0](https://github.com/ostis-dev/ostis-web-platform.git) и поставить [пример](https://github.com/ostis-apps/wave_find_path_sc_memory.git)

Далее, в папку wave_find_path_sc_memory, скачать содержимое папки C++. в файле radius.cpp в функции main изменить пути к библиотеке. Тесты поместить в папку graph, предварительно удалив существующие там тесты. 

Из сред разработки рекомендуется использовать CLion  или QT Creator.

## Про алгоритм

Используется обход в глубину. При обходе формируются все возможные дуги графа c минимально возможным весом .Обход в глубину проводится с каждой вершины графа.
Далее находим эксцентриситет каждой вершины, из которых выбираем минимальный, это и будет радиусом графа.



