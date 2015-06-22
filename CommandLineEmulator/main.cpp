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
unsigned int CursorXPosition;
unsigned int CommandPosition;
HANDLE hStdin;
DWORD fdwSaveOldMode;
std::vector<WCHAR> LineBuffer;
std::vector<std::vector<WCHAR>> CommandBuffer;
std::vector<std::string> Commands;

int main()
{
	// Get the standard input handle. 
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hStdin == INVALID_HANDLE_VALUE)
		return -1;

	// Save the current input mode, to be restored on exit. 
	if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
		return -1;
	CursorXPosition = 0;
	CommandPosition = 0;
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

	DWORD cNumRead;
	DWORD fdwMode;
	INPUT_RECORD irInBuf[128];
	int counter = 0;


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

		for (DWORD i = 0; i < cNumRead; i++)
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
		if (iswprint(ker.uChar.UnicodeChar))
		{
			std::wcout << ker.uChar.UnicodeChar /*<< ker.uChar.AsciiChar << ker.wVirtualKeyCode*/;
			LineBuffer.insert(LineBuffer.begin() + CursorXPosition, ker.uChar.UnicodeChar);
			CursorXPosition++;
		}
		else if (ker.wVirtualKeyCode == 13) //Return
		{
			std::wcout << std::endl;
			EnterCommand(LineBuffer);
			CommandBuffer.push_back(LineBuffer);
			LineBuffer.clear();
			CommandPosition = CommandBuffer.size();
			bIsInInputMode = false;
			CursorXPosition = 0;
		}
		else if (ker.wVirtualKeyCode == 9) //Tab
		{
			AttemptAutoComplete();
		}
		else if (ker.wVirtualKeyCode == 8) //Delete
		{
			if (LineBuffer.size() > 0)
			{
				SetConsoleMode(hStdin, ENABLE_PROCESSED_INPUT);
				std::cout << '\b' << " " << '\b'; //Add a space to the buffer so the original character disappears.
				LineBuffer.erase(LineBuffer.begin() + CursorXPosition - 1);
				CursorXPosition--;
			}
		}
		else if (ker.wVirtualKeyCode == 37) //Left Arrow
		{
			CONSOLE_SCREEN_BUFFER_INFO infoCon;
			if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &infoCon))
			{
				if (CursorXPosition > 0)
				{
					COORD NewPos;
					NewPos.X = infoCon.dwCursorPosition.X - 1;
					NewPos.Y = infoCon.dwCursorPosition.Y;
					SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), NewPos);
					CursorXPosition--;
				}
			}
		}
		else if (ker.wVirtualKeyCode == 39) //Right Arrow
		{
			CONSOLE_SCREEN_BUFFER_INFO infoCon;
			if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &infoCon))
			{
				if (CursorXPosition < LineBuffer.size())
				{
					COORD NewPos;
					NewPos.X = infoCon.dwCursorPosition.X + 1;
					NewPos.Y = infoCon.dwCursorPosition.Y;
					SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), NewPos);
					CursorXPosition++;
				}
			}
		}
		else if (ker.wVirtualKeyCode == 38) //Up Arrow
		{

			if (CommandPosition > 0 && CommandPosition - 1 < CommandBuffer.size())
			{
				//clear the buffer
				for (int i = 0; i < LineBuffer.size(); i++)
				{
					SetConsoleMode(hStdin, ENABLE_PROCESSED_INPUT);
					std::cout << '\b' << " " << '\b'; //Add a space to the buffer so the original character disappears.
					LineBuffer.erase(LineBuffer.begin() + CursorXPosition - 1);
					i--;
					CursorXPosition--;
				}

				for (char ComChar : CommandBuffer.at(CommandPosition - 1))
				{
					std::wcout << ComChar /*<< ker.uChar.AsciiChar << ker.wVirtualKeyCode*/;
					LineBuffer.insert(LineBuffer.begin() + CursorXPosition, ComChar);
					CursorXPosition++;
				}
				CommandPosition--;
			}
		}
		else if (ker.wVirtualKeyCode == 40) //Down Arrow
		{
			//clear the buffer
			for (int i = 0; i < LineBuffer.size(); i++)
			{
				SetConsoleMode(hStdin, ENABLE_PROCESSED_INPUT);
				std::cout << '\b' << " " << '\b'; //Add a space to the buffer so the original character disappears.
				LineBuffer.erase(LineBuffer.begin() + CursorXPosition - 1);
				i--;
				CursorXPosition--;
			}

			if (CommandPosition - 1 >= 0 && CommandPosition + 1 < CommandBuffer.size())
			{
				CommandPosition++;
				for (char ComChar : CommandBuffer.at(CommandPosition))
				{
					std::wcout << ComChar /*<< ker.uChar.AsciiChar << ker.wVirtualKeyCode*/;
					LineBuffer.insert(LineBuffer.begin() + CursorXPosition, ComChar);
					CursorXPosition++;
				}
			}
			else
			{
				CommandPosition = CommandBuffer.size();
			}
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
	//Put commands here
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

	//Search for commands that match the line buffer
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

				if (CurLine.length() == i + 1)
				{
					CommandsFound.push_back(Command);
				}
			}
		}
	}

	//If only one command found then obvious thats what we want.
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

	//To Do: Otherwise implement a smart system that autocompletes to the 
	//most used commands or something and cycles with every tab press.
}