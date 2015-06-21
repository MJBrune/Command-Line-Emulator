#include <iostream>
#include <windows.h>
#include <string>
#include <Lmcons.h>

void PrintHeader(); //Prints the header of the command line
std::string GetInput();  //Gathers the input from user and puts into a string for command parsing.

int main()
{
	bool bIsInConsole = true;
	while (bIsInConsole)
	{
		PrintHeader();
		

		//todo: rewrite for multiple commands
		if (GetInput() == "exit")
		{
			bIsInConsole = false;
		}

	}
}

void PrintHeader()
{
	TCHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 2];
	DWORD ComputerNameSize;
	ComputerNameSize = sizeof ComputerName - 1;
	GetComputerName(ComputerName, &ComputerNameSize);
	TCHAR UserName[UNLEN + 1];
	DWORD UserNameSize = UNLEN + 1;
	GetUserName(UserName, &UserNameSize);
	std::wcout << "[" << UserName << "@" << ComputerName << "]# ";
}

std::string GetInput()
{
	std::string InputString;
	std::cin >> InputString;
	return InputString;
}