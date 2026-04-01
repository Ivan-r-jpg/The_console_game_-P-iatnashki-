#include <iostream> // Підключення стандартної бібліотеки, необхідної для вводу/виводу 
#include <limits> // Підключення бібліотеки, необхідної для очищення потоку вводу
#include <string> // Підключення бібліотеки, необхідної для роботи з рядками
#include "GameController.h" // Підлючення заголовочного файлу, що містить оголошену структуру GameController


void GameController::StartGame() // Визначення функції, необхідної для початку гри
{
	std::string input; // Оголошення змінної input типу string для контролю введення користувача
	
	while (true) // Оператор циклу while, нескінченний цикл
	{
		std::cout << "Введіть розмір ігрового поля: ";
		std::getline(std::cin, input); // Введення користувачем текстового рядка
		try // Брок обробки виключення
		{
			field.d = InputControling(input); // Виклик функції для обробки введення користувача

			if (field.d < MIN_BOARD_SIZE || field.d > MAX_BOARD_SIZE) // Умовний оператор if, якщо введене користувачем число не задовольняє допустимий розмір заданих меж
			{
				std::cerr << "\n[УВАГА] - Розмір поля повинен бути в межах від " << MIN_BOARD_SIZE << " до " << MAX_BOARD_SIZE << '!' << std::endl;
				continue; // Виклик нової ітерації циклу
			}
			std::cout << std::endl;
			break; // Вихід з циклу, якщо все чудово
		}
		catch (const std::invalid_argument& e) // Перехоплення помилки оператором catch
		{
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
	bool isSpace = false; // Оголошення змінної булевого типу, для перевірки чи поставлено гру на паузу
	bool gameOver = false;
	std::string input; // Оголошення змінної input типу string для контролю введення користувача
	int step; // Оголошення змінної типу int для зберігання номеру ходової фішки
	char c;

	while (field.status == GameField::Active) // Оператор циклу while, цикл повторюється доти, поки гра перебуває у активному стані
	{
		std::cout << "Поточний стан ігрового поля:" << std::endl; 
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
						
						field.status = GameField::Active; // Відновлення статусу гри як активної
						break; // Вихід з поточного циклу
					}
					else if (c == 32) // Умовний оператор if, якщо натиснуто Space
					{
						gameOver = true; // Встановлення флага
						break; // Вихід з поточного циклу
					}
					else // Інакше, якщо введено щось зовсім інше
					{
						std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n'); // Очищення потока вводу від залишкових символів, щоб не заважали наступному введенню
						std::cout << std::endl;
						std::cout << "[УВАГА] - Введіть ще раз: ";
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
				
				moveCount++; // Підрахунок кількості ходів, зроблених користувачем

				break; // Вихід з внутрішнього циклу
			}
			catch (const std::invalid_argument& e) // Перехоплення помилки оператором catch
			{
				std::cerr << "\n" << e.what() << "\nВведіть ще раз коректне ціле число!\n";
				continue;
			}
			catch (const std::logic_error& e)
			{
				std::cerr << "\n" << e.what() << "\nХід пропущено!\n";
				break;
			}
		}
		if (field.IsGameOver()) // Умовний оператор if, якщо гру завершено
		{
			std::cout << field; // Друк фінального ігрового поля на екран

			field.status = GameField::Win; // Встановлення статусус гри як завершеної
			std::cout << std::endl;
			std::cout << "[УВАГА] - Гра завершена! Вітаємо з перемогою!\n" << std::endl;;
			std::cout << "Кількість зроблених ходів: " << moveCount << std::endl;
		}
		else if (gameOver) // Умовний оператор else if, якщо гру завершили при паузі
		{
			std::cout << std::endl;
			std::cout << "Завершення гри..." << std::endl;
			field.status = GameField::UserInterrupted; // Встановлення статусу гри як перехопленої
		}
		else // Інакше
		{
			std::cout << std::endl;
			std::cout << "[УВАГА] - Гра продовжується. Зробіть наступний хід!\n" << std::endl;
		}
	}
}