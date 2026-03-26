// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h> // Підключення бібліотеки для роботи з консоллю (для встановлення кодування)
#include "GameController.h" // Підключення заголовочного файлу для керування ігровим процесом

int main() // Головна функція, точка входу в програму
{
	SetConsoleOutputCP(65001); // Встановлення кодування UTF-8 для виводу в консоль
	SetConsoleCP(65001); // Встановлення кодування UTF-8 для вводу з консолі

	GameController Game; // Створення змінної типу GameController

	Game.StartGame(); // Запуск основного ігрового процесу

	return 0; // Завершення програми з кодом успішного виконання
}