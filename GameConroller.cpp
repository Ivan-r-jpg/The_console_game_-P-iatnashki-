#define _CRT_SECURE_NO_WARNINGS // Визначення макроса для відключення деяких безпечних функцій, які можуть викликати попередження компілятора
#include <iostream> // Підключення стандартної бібліотеки, необхідної для вводу/виводу 
#include <limits> // Підключення бібліотеки, необхідної для очищення потоку вводу
#include <string> // Підключення бібліотеки, необхідної для роботи з рядками
#include "GameController.h" // Підлючення заголовочного файлу, що містить оголошену структуру GameController
#include <chrono> // Підключення бібліотеки, необхідної для роботи з часом
#include <fstream> // Підключення бібліотеки, необхідної для роботи з файлами
#include <iomanip> // Підключення бібліотеки, необхідної для форматування виводу часу

void GameController::StartGame() // Визначення функції, необхідної для початку гри
{
	std::string input; // Оголошення змінної input типу string для контролю введення користувача
	
	std::ofstream logFile("game_log.txt", std::ios::app); // Відкриття файлу для логування гри в режимі додавання
	if (logFile.fail()) // Умовний оператор if, якщо файл не вдалося відкрити
	{
		std::cerr << "[ПОМИЛКА] - Не вдалося відкрити файл для логування гри!" << std::endl;
		return; // Вихід з функції
	}

	while (true) // Оператор циклу while, нескінченний цикл
	{
		std::cout << "Введіть розмір ігрового поля: ";
		logFile << "Запит розміру поля...\n" << std::endl; 
		std::getline(std::cin, input); // Введення користувачем текстового рядка
		try // Брок обробки виключення
		{
			field.d = InputControling(input); // Виклик функції для обробки введення користувача

			if (field.d < MIN_BOARD_SIZE || field.d > MAX_BOARD_SIZE) // Умовний оператор if, якщо введене користувачем число не задовольняє допустимий розмір заданих меж
			{
				logFile << "[УВАГА] - Введено некоректний розмір поля: " << field.d << "\nРозмір поля повинен бути в межах від " << MIN_BOARD_SIZE << " до " << MAX_BOARD_SIZE << '!\n' << std::endl;
				std::cerr << "\n[УВАГА] - Розмір поля повинен бути в межах від " << MIN_BOARD_SIZE << " до " << MAX_BOARD_SIZE;
				std::cout << "!\n\n";
				continue; // Виклик нової ітерації циклу
			}
			std::cout << std::endl;
			break; // Вихід з циклу, якщо все чудово
		}
		catch (const std::invalid_argument& e) // Перехоплення помилки оператором catch
		{
			logFile << "\n" << e.what() << "\nВведення: " << input << std::endl;
			std::cerr << "\n" << e.what() << "\nВведіть ще раз коректне ціле число!\n\n";
			continue;
		}
	}
	
	field.InitialField(field.d); // Ініціалізація ігрового поля
	
	field.status = GameField::Active; // Встановлення статусу гри як активної

	moveCount = 0; // Ініціалізація лічильника ходів
	
	GameLoop(); // Запуск основного ігрового циклу

	// Очищення динамічно виділеної пам'яті під двовимірний масив
	for (int i = 0; i < field.d; i++) // Оператор циклу for, звільнення пам'яті, виділеної для кожного рядка ігрового поля
	{
		delete[] field.field[i];
	}

	delete[] field.field; // Звільнення пам'яті, виділеної для масиву рядків ігрового поля

	logFile.close(); // Закриття файлу логування
}

int GameController::InputControling(std::string input) // Визначення функції, необхідної для обробки введення користувача
{
	int variable;
	size_t pos; // Оголошення змінної для визначення позиції останнього числа
	try
	{
		variable = stoi(input, &pos); // Перетворення рядка в числовий тип
	}
	catch (const std::invalid_argument&)
	{
		throw std::invalid_argument("[ПОМИЛКА] - Введено не число!");
	}
	catch (const std::out_of_range&)
	{
		throw std::invalid_argument("[ПОМИЛКА] - Занадто велике число!!");
	}
	if (pos < input.length()) // Умовний оператор if, якщо в записі наявні зайві символи
	{
		throw std::invalid_argument("\nУ записі числа наявні зайві символи!"); // Оператор генерування винятку throw
	}
	return variable;
}

void GameController::GameLoop() // Визначення функції, необхідного для основного циклу гри, де відбувається логіка гри
{

	std::ofstream logFile("game_log.txt", std::ios::app); // Відкриття файлу для логування гри в режимі додавання
	if (logFile.fail()) // Умовний оператор if, якщо файл не вдалося відкрити
	{
		std::cerr << "[ПОМИЛКА] - Не вдалося відкрити файл для логування гри!" << std::endl;
		exit(1); // Вихід з програми
	}

	using clock = std::chrono::system_clock; // Спрощення запису типу для роботи з часом
	auto startTime = clock::now(); // Запис часу початку гри
	std::chrono::milliseconds pause(0); // Ініціалізація змінної для підрахунку часу паузи
	std::time_t startTime_t = std::chrono::system_clock::to_time_t(startTime); // Запис часу початку гри у форматі time_t для логування
	std::tm* startTime_tm = std::localtime(&startTime_t); // Конвертація часу початку гри у структуру tm для зручного форматування

	bool isSpace = false; // Оголошення змінної булевого типу, для перевірки чи поставлено гру на паузу
	bool gameOver = false; // Оголошення змінної булевого типу, для перевірки чи гру було завчасно завершено користувачем
	std::string input; // Оголошення змінної input типу string для контролю введення користувача
	int step; // Оголошення змінної типу int для зберігання номеру ходової фішки
	char c;

	logFile << "\n[УВАГА] - Гра розпочата: " << std::put_time(startTime_tm, "%Y-%m-%d %H:%M:%S") << std::endl; // Логування часу початку гри

	while (field.status == GameField::Active) // Оператор циклу while, цикл повторюється доти, поки гра перебуває у активному стані
	{
		std::cout << "Поточний стан ігрового поля:" << std::endl; 

		logFile << "\nПоточний стан ігрового поля:\n\n";
		logFile << field; // Логування поточного стану ігрового поля

		std::cout << std::endl;
		std::cout << field; // Друк ігрового поля на екран
		
		while (true) // Оператор циклу while, нескінченний цикл
		{
			isSpace = false; // Оновлення змінної isSpace
			std::cout << std::endl;
			std::cout << "Введіть номер фішки, яку бажаєте пересунути: ";
			std::getline(std::cin, input); // Введення користувачем текстового рядка
			for (int i = 0; i < input.length(); i++) // Оператор циклу for, прохід по кожному символу введеного рядка
			{
				if (input[i] == 32) // Умовний оператор if, якщо натиснуто пробіл
				{
					isSpace = true; // Встановлення флагу
					break; // Вихід з внутрішнього циклу
					
				}
			}
			if (isSpace) // Умовний оператор if, якщо було натиснуто пробіл
			{
				auto pauseStart = clock::now(); // Запис часу початку паузи
				std::time_t pauseStart_t = std::chrono::system_clock::to_time_t(pauseStart); // Запис часу початку паузи у форматі time_t для логування
				std::tm* pauseStart_tm = std::localtime(&pauseStart_t); // Конвертація часу початку паузи у структуру tm для зручного форматування
				logFile << "\n[УВАГА] - Гра поставлена на паузу користувачем: " << std::put_time(pauseStart_tm, "%Y-%m-%d %H:%M:%S") << "\n";
				field.status = GameField::UserInterrupted; // Встановлення статусу гри як в режимі паузи
				std::cout << std::endl;
				std::cout << "[УВАГА] - Увімкнено режим паузи...\nНатисніть:\n\nSpace - Щоб завершити гру;\nEnter - Щоб відновити гру.\n\nВведіть ваш вибір: ";
				
				while (true)
				{
					isSpace = false; // Оновлення змінної isSpace

					std::cin.get(c); // Зчитування одного символу

					if (c == 10) // Умовний оператор if, якщо натиснуто Enter
					{
						
						isSpace = true; // Встановлення флагу
						auto pauseEnd = clock::now(); // Запис часу закінчення паузи
						field.status = GameField::Active; // Відновлення статусу гри як активної
						pause += std::chrono::duration_cast<std::chrono::milliseconds>(pauseEnd - pauseStart); // Додавання часу паузи до загального часу пауз
						logFile << "\n[УВАГА] - Гра відновлена користувачем!\nЧас очікування в режимі паузи: " << pause.count() / 60000 << "хв. " << (pause.count() % 60000) / 1000 << "c.\n";
						break; // Вихід з поточного циклу
					}
					else if (c == 32) // Умовний оператор if, якщо натиснуто Space
					{
						gameOver = true; // Встановлення флага
						auto pauseEnd = clock::now(); // Запис часу закінчення паузи
						pause += std::chrono::duration_cast<std::chrono::milliseconds>(pauseEnd - pauseStart); // Додавання часу паузи до загального часу пауз
						logFile << "\n[УВАГА] - Гра перервана користувачем під час паузи!\nЧас очікування в режимі паузи: " << pause.count() / 60000 << "хв. " << (pause.count() % 60000) / 1000 << "c.\n";
						break; // Вихід з поточного циклу
					}
					else // Інакше, якщо введено щось зовсім інше
					{
						std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n'); // Очищення потока вводу від залишкових символів, щоб не заважали наступному введенню
						std::cout << std::endl;
						std::cout << "[УВАГА] - Введіть ще раз: ";
						logFile << "\n[УВАГА] - Користувач ввів некоректний вибір в режимі паузи!\n";
						continue; // Виклик нової ітерації циклу
					}
				}
				if (gameOver)
				{
					break; // Вихід з поточного циклу
				}
				else if (isSpace)
				{
					continue; // Виклик нової ітерації циклу
				}
			}
			if (gameOver)
			{
				break; // Вихід з внутрішнього циклу
			}
			try // Брок обробки виключення
			{
				step = InputControling(input); // Виклик функції для обробки введення користувача
				
				field += step; // Виконання ходу
				
				std::cout << std::endl;
				std::cout << "[УВАГА] - При обраній фішці для руху: №" << step << " - хід можливий!\nХід виконано!" << std::endl;
				logFile << "\n[УВАГА] - Користувач вибрав фішку №" << step << " для руху!\nХід виконано!\n";
				moveCount++; // Підрахунок кількості ходів, зроблених користувачем

				break; // Вихід з внутрішнього циклу
			}
			catch (const std::invalid_argument& e) // Перехоплення помилки оператором catch
			{
				logFile << "\n" << e.what() << "\nВведення: " << input << std::endl;
				std::cerr << "\n" << e.what() << "\nВведіть ще раз коректне ціле число!\n";
				continue;
			}
			catch (const std::logic_error& e)
			{
				logFile << "\n" << e.what() << "\nВведення: " << input << std::endl;
				std::cerr << "\n" << e.what() << "\nХід пропущено!\n";
				break;
			}
		}
		if (field.IsGameOver()) // Умовний оператор if, якщо гру завершено
		{
			logFile << field; // Логування фінального стану ігрового поля
			std::cout << "\n" << field; // Друк фінального ігрового поля на екран

			field.status = GameField::Win; // Встановлення статусус гри як завершеної
			auto endTime = clock::now(); // Запис часу завершення гри
			std::chrono::seconds totalTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime - pause); // Підрахунок загального часу гри, виключаючи час пауз
			std::time_t endTime_t = std::chrono::system_clock::to_time_t(endTime); // Запис часу початку гри у форматі time_t для логування
			std::tm* endTime_tm = std::localtime(&endTime_t); // Конвертація часу початку гри у структуру tm для зручного форматування
			logFile << "\n[УВАГА] - Гра завершена! Вітаємо з перемогою!\nЗагальний час гри (без урахування пауз): " << totalTime.count() / 60 << "хв. " << totalTime.count() % 60 << "c."
				"\nКількість зроблених ходів: " << moveCount << "\n\n[УВАГА] - Час завершення гри: " << std::put_time(endTime_tm, "%Y-%m-%d %H:%M:%S") << 
				"\nСтатус завершення гри: " << field.status << std::endl;
			std::cout << std::endl;
			std::cout << "[УВАГА] - Гра завершена! Вітаємо з перемогою!\nЗагальний час гри (без урахування пауз): " << totalTime.count() / 60 << "хв. " << totalTime.count() % 60 << "c.\n";
			std::cout << "Кількість зроблених ходів: " << moveCount << std::endl;
		}
		else if (gameOver) // Умовний оператор else if, якщо гру завершили при паузі
		{
			field.status = GameField::UserInterrupted; // Встановлення статусу гри як перехопленої
			auto endTime = clock::now(); // Запис часу завершення гри
			std::cout << std::endl;
			std::cout << "Завершення гри..." << std::endl;
			std::chrono::seconds totalTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime - pause); // Підрахунок загального часу гри, виключаючи час пауз
			std::time_t endTime_t = std::chrono::system_clock::to_time_t(endTime); // Запис часу початку гри у форматі time_t для логування
			std::tm* endTime_tm = std::localtime(&endTime_t); // Конвертація часу початку гри у структуру tm для зручного форматування
			logFile << "\n[УВАГА] - Гра завчасно завершена користувачем!\nЗагальний час гри (без урахування пауз): " << totalTime.count() / 60 << "хв. " << totalTime.count() % 60 << "c."
				"\nКількість зроблених ходів: " << moveCount << "\n\n[УВАГА] - Час завершення гри: " << std::put_time(endTime_tm, "%Y-%m-%d %H:%M:%S") <<
				"\nСтатус завершення гри: " << field.status << std::endl;
		}
		else // Інакше
		{
			std::cout << std::endl;
			std::cout << "[УВАГА] - Гра продовжується. Зробіть наступний хід!\n" << std::endl;
			logFile << "\n[УВАГА] - Гра продовжується\nХід №" << moveCount << ":\n";
		}
	}
	logFile.close(); // Закриття файлу логування
}