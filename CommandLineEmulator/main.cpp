#include <iostream>
#include <windows.h>
#include <string>
#include <Lmcons.h>
#include <vector>
void PrintHeader(); //Prints the header of the command line.
void GetInput();  //Gathers the input from user and puts into a string for command parsing.
void KeyEventProc(KEY_EVENT_RECORD ker); //Processes the key events from the user to the console.
void EnterCommand(std::vector<WCHAR> Command); //Process Command from input.
void AttemptAutoComplete(); //Attempts to auto complete the line buffer.

bool bIsInConsole;
bool bIsInInputMode;
std::vector<WCHAR> LineBuffer;
std::vector<std::vector<WCHAR>> CommandBuffer;
std::vector<std::string> Commands;

int main()
{
	bIsInConsole = true;
	bIsInInputMode = false;
	LineBuffer.clear();
	CommandBuffer.clear();
	Commands = { "exit" };
	while (bIsInConsole)
	{
		PrintHeader();
		GetInput();
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

void GetInput()
{
	bIsInInputMode = true;
	HANDLE hStdin;
	DWORD fdwSaveOldMode;

	DWORD cNumRead, fdwMode, i;
	INPUT_RECORD irInBuf[128];
	int counter = 0;

	// Get the standard input handle. 
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hStdin == INVALID_HANDLE_VALUE)
		return;

	// Save the current input mode, to be restored on exit. 
	if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
		return;

	// Enable the window and mouse input events. 
	fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
	if (!SetConsoleMode(hStdin, fdwMode))
		return;

	while (bIsInInputMode)
	{
		// Wait for the events. 

		if (!ReadConsoleInput(
			hStdin,      // input buffer handle 
			irInBuf,     // buffer to read into 
			128,         // size of read buffer 
			&cNumRead)) // number of records read 
			return;

		// Dispatch the events to the appropriate handler. 

		for (i = 0; i < cNumRead; i++)
		{
			switch (irInBuf[i].EventType)
			{
			case KEY_EVENT: // keyboard input 
				KeyEventProc(irInBuf[i].Event.KeyEvent);
				break;
			}
		}
	}

	// Restore input mode on exit.

	SetConsoleMode(hStdin, fdwSaveOldMode);

}

void KeyEventProc(KEY_EVENT_RECORD ker)
{
	if (ker.bKeyDown)
	{
		if (!iswspace(ker.uChar.UnicodeChar))
		{
			std::wcout << ker.uChar.UnicodeChar /*<< ker.uChar.AsciiChar << ker.wVirtualKeyCode*/;
			LineBuffer.push_back(ker.uChar.UnicodeChar);
		}
		if (ker.wVirtualKeyCode == 13) //Return
		{
			std::wcout << std::endl;
			EnterCommand(LineBuffer);
			CommandBuffer.push_back(LineBuffer);
			LineBuffer.clear();
			bIsInInputMode = false;
		}
		if (ker.wVirtualKeyCode == 9) //Tab
		{
			AttemptAutoComplete();
		}
	}
}

void EnterCommand(std::vector<WCHAR> Command)
{
	std::string commandString = "";
	for (WCHAR ComChar : Command)
	{
		commandString += ComChar;
	}

	if (commandString == "exit")
	{
		bIsInConsole = false;
	}
}

void AttemptAutoComplete()
{
	std::wstring CurLine;
	CurLine.clear();
	for (WCHAR LineChar : LineBuffer)
	{
		CurLine += LineChar;
	}

	std::vector<std::string> CommandsFound;
	for (std::string Command : Commands)
	{
		if (Command.length() >= CurLine.length())
		{
			for (unsigned int i = 0; i < CurLine.length(); i++)
			{
				if (CurLine.at(i) != Command.at(i))
				{
					i = CurLine.length();
				}

				if (CurLine.length() == i+1)
				{
					CommandsFound.push_back(Command);
				}
			}
		}
	}

	if (CommandsFound.size() == 1)
	{
		int PrintedSoFar = LineBuffer.size() - 1;
		LineBuffer.clear();
		for (int i = 0; i < CommandsFound.at(0).length(); i++)
		{
			LineBuffer.push_back(CommandsFound.at(0).at(i));
			if (PrintedSoFar < i)
			{
				std::cout << CommandsFound.at(0).at(i);
			}
		}
	}
}