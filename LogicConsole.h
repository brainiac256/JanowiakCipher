/*
	Author: Drunken.Canadian
	Release Date: 1/10/2011
	Website: infinitylogic.net

	I hope you enjoy using this class in your console projects,
	and will recommend it to your friends :P

	Current Features
	clear        - Clears the console screen; clear lats n charaters; clear last line;
	pause        - Pauses the application and waits for input form the user
	flashWindow  - Flashes the taskbar icon at the user
	setTitle     - Sets the console title
	randNumber   - Generates a random number
	randString   - Generates a random String; user can set length and charaters used. char chars[] = {'a', 'b'};
	stdioColor   - Sets the output & input color of the console
	setCursorPos - Sets the console cursor position (x, y)
	getCursorPos - Get the console cursor current position
	login        - allows your applications to be password protected
	parse        - parses input by the user; seperated into an array by the specified charater
	cinHidden    - Gets input from the user; no chataters are displayed or they are masked
	hideConsole  - Hides the console from the user >:)
*/

/*
	I added in the function resizeWindowV.  
	Posted my code to the forum where I found this class here: infinitylogic.net/forum/index.php?topic=503.0
	My Screenname: Shattered.likeness
*/

#ifndef LOGICCONSOLE_H
#define LOGICCONSOLE_H
#define _WIN32_WINNT 0x0500

#include <iostream>
#include <windows.h>
#include <string>
#include <conio.h>

/*Define Colors*/
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define LIGHTGREY 7
#define DARKGREY 8
#define LIGHTBLUE 9
#define LIGHTGREEN 10
#define LIGHTCYAN 11
#define LIGHTRED 12
#define LIGHTMAGENTA 13
#define YELLOW 14
#define WHITE 15
#define BLINK 128
/*End Define Colors*/

class LogicConsole
{
private:
	HANDLE getConsoleHandel(void);
	HWND getConsoleHwnd(void);
public:
	void clear(void);
	void clear(int);
	void clearLastLine(void);
	void pause(void);
	void flashWindow(void);
	int randNumber(void);
	int randNumber(int, int);
	std::string randString(void);
	std::string randString(int);
	std::string randString(int, char*);
	void setTitle(std::string);
	void setCursorPos(int, int);
	COORD getCursorPos(void);
	int getCursorPosX(void);
	int getCursorPosY(void);
	void stdioColorFg(int);
	bool login(std::string, std::string, bool, int);
	bool login(std::string, std::string, bool, int, std::string, std::string);
	std::string cinHidden(void);
	std::string cinHidden(char);
	void hideConsole(void);
	void resizeWindowV (int y);
};

HANDLE LogicConsole::getConsoleHandel(void)
{
	return GetStdHandle((DWORD)-11);
}

HWND LogicConsole::getConsoleHwnd(void)
{
	return GetConsoleWindow();
}

void LogicConsole::clear(void)
{
	COORD topLeft = {0, 0};
	DWORD cCharsWritten, dwConSize;
	CONSOLE_SCREEN_BUFFER_INFO cInfo;
	GetConsoleScreenBufferInfo(LogicConsole::getConsoleHandel(), &cInfo);
	dwConSize = cInfo.dwSize.X * cInfo.dwSize.Y;
	FillConsoleOutputCharacter(LogicConsole::getConsoleHandel(), (TCHAR)' ', dwConSize, topLeft, &cCharsWritten);
	GetConsoleScreenBufferInfo(LogicConsole::getConsoleHandel(), &cInfo);
	FillConsoleOutputAttribute(LogicConsole::getConsoleHandel(), cInfo.wAttributes, dwConSize, topLeft, &cCharsWritten);
	SetConsoleCursorPosition(LogicConsole::getConsoleHandel(), topLeft);
}

void LogicConsole::clear(int len)
{
	for (int i = 0; i < len; i++)
	{
		std::cout << "\b \b";
	}
}

void LogicConsole::clearLastLine(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(LogicConsole::getConsoleHandel(), &csbi);
	int y = csbi.dwCursorPosition.Y;
	LogicConsole::setCursorPos(0, y);

	for (int i = 0; i < csbi.dwSize.X; i++)
	{
		std::cout << " ";
	}
	LogicConsole::setCursorPos(0, y);
}

void LogicConsole::pause(void)
{
	char buffer;

	std::cout << "Press Enter To Continue...";

	while (true)
	{
		buffer = _getch();

		if (buffer == 13)
			break;
	}
}

void LogicConsole::stdioColorFg(int foreground)
{
	SetConsoleTextAttribute(LogicConsole::getConsoleHandel(), foreground);
}

void LogicConsole::setCursorPos(int X, int Y)
{
	COORD pos = {X, Y};
	SetConsoleCursorPosition(LogicConsole::getConsoleHandel(), pos);
}

COORD LogicConsole::getCursorPos(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(LogicConsole::getConsoleHandel(), &csbi);

	COORD pos = {csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y};
	return pos;
}

int LogicConsole::getCursorPosX(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(LogicConsole::getConsoleHandel(), &csbi);

	return csbi.dwCursorPosition.X;
}

int LogicConsole::getCursorPosY(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(LogicConsole::getConsoleHandel(), &csbi);

	return csbi.dwCursorPosition.Y;
}

std::string LogicConsole::cinHidden(void)
{
	char buffer;
	std::string output;

	while (buffer = _getch())
	{
		if (buffer == 13) // Enter key pressed
			break;

		else if (buffer == 8) // Backspace key pressed
		{
			if (output.length() >= 1)
				output.erase(output.length() -1, output.length());
		}

		else
			output += buffer;
	}
	return output;
}

std::string LogicConsole::cinHidden(char mask)
{
	char buffer;
	std::string output;

	while (buffer = _getch())
	{
		if (buffer == 13) // Enter key pressed
			break;
		else if (buffer == 8) // Backspace key pressed
		{
			if (output.length() == 1)
				std::cout << "\b \b";

			else if (output.length() > 1)
			{
				output.erase(output.length() -1, output.length());
				std::cout << "\b \b";
			}
		}
		else
		{
			std::cout << mask;
			output += buffer;
		}
	}
	return output;
}

bool LogicConsole::login(std::string username, std::string password, bool masked, int maxAttempts)
{
	std::string bufUsr, bufPas;
	int i = 0;

	while (i < maxAttempts)
	{
		COORD pos = {10,0};

		LogicConsole::clear();
		std::cout << "Username:\nPassword:";
		SetConsoleCursorPosition(LogicConsole::getConsoleHandel(), pos);
		getline(std::cin, bufUsr);
		pos.Y += 1;
		SetConsoleCursorPosition(LogicConsole::getConsoleHandel(), pos);

		if (masked == true)
			bufPas = LogicConsole::cinHidden('*');
		else
			getline(std::cin, bufPas);

		if (bufUsr.compare(username) == 0 && bufPas.compare(password) == 0)
		{
			std::cout << "\nLoggin Successful...\n";
			Sleep(2000);
			LogicConsole::clear();
			return true;
			break;
		}
		else
		{
			if (i == maxAttempts - 1)
			{
				std::cout << "\nLogin Failed...\n";
				Sleep(2000);
				LogicConsole::clear();
				return false;
				break;
			}
		}
		i++;
	}
	return false;
}

bool LogicConsole::login(std::string username, std::string password, bool masked, int maxAttempts, std::string msgSuccess, std::string msgFailed)
{
	std::string bufUsr, bufPas;
	int i = 0;

	while (i < maxAttempts)
	{
		COORD pos = {10,0};

		LogicConsole::clear();
		std::cout << "Username:\nPassword:";
		SetConsoleCursorPosition(LogicConsole::getConsoleHandel(), pos);
		getline(std::cin, bufUsr);
		pos.Y += 1;
		SetConsoleCursorPosition(LogicConsole::getConsoleHandel(), pos);

		if (masked == true)
			bufPas = LogicConsole::cinHidden('*');
		else
			getline(std::cin, bufPas);

		if (bufUsr.compare(username) == 0 && bufPas.compare(password) == 0)
		{
			std::cout << "\n" << msgSuccess;
			Sleep(2000);
			LogicConsole::clear();
			return true;
			break;
		}
		else
		{
			if (i == maxAttempts - 1)
			{
				std::cout << "\n" << msgFailed;
				Sleep(2000);
				LogicConsole::clear();
				return false;
				break;
			}
		}
		i++;
	}
	return false;
}

void LogicConsole::setTitle(std::string title)
{
	LPCTSTR text = _strdup(title.c_str());
	SetConsoleTitle(text);
}

void LogicConsole::hideConsole(void)
{
	ShowWindow(LogicConsole::getConsoleHwnd(), SW_HIDE);
}

void LogicConsole::flashWindow(void)
{
	FlashWindow(LogicConsole::getConsoleHwnd(), TRUE);
}

int LogicConsole::randNumber(void)
{
	srand(GetTickCount() + GetTickCount() % 3);
	int ranNum = rand();
	return ranNum;
}

int LogicConsole::randNumber(int min, int max)
{
	srand(GetTickCount() + GetTickCount() % 7);
	int ranNum = rand() % max + min;
	return ranNum;
}

std::string LogicConsole::randString(void)
{
	int len = LogicConsole::randNumber();
	char buffer;
	std::string output;

	srand(LogicConsole::randNumber());

	for (int i = 0; i < len; i++)
	{
		buffer = rand();
		output += buffer;
	}
	return output;
}

std::string LogicConsole::randString(int len)
{
	char buffer;
	std::string output;

	srand(LogicConsole::randNumber());

	for (int i = 0; i < len; i++)
	{
		buffer = rand();
		output += buffer;
	}
	return output;
}

std::string LogicConsole::randString(int len, char* charArr)
{
	char buffer;
	std::string output;

	srand(LogicConsole::randNumber());

	for (int i = 0; i < len; i++)
	{
		buffer = charArr[rand() % sizeof(charArr) + 0];
		output += buffer;
	}
	return output;
}

void LogicConsole::resizeWindowV (int y)
{
  HWND console = GetConsoleWindow();
  RECT r;
  GetWindowRect(console, &r);
  MoveWindow(console, r.left, r.top, 800, y, TRUE);
}


#endif