// MiniSQL.cpp : Defines the entry point for the console application.
//

#include "MiniSQL.h"
#include "interpret.h"
#include "catalog_manager.h"
#include "buffer_manager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

CCatalogManager Catalog;

using namespace std;

void AddSeperator(char *command)
{
	unsigned len = strlen(command);
	command[len] = ' ';
	command[len + 1] = '\0';
}

short int IsComEnd(char *input)
{
	unsigned int next = strlen(input) - 1;
	char prev = ' ';
	while(next >= 0 && (prev == '\t' || prev ==' '))
	{
		prev = input[next];
		next --;
	}
	if(prev == ';')
	{
		input[next + 1] ='\0';
		return 1;
	}
	return 0;
}

int main(int argc, const char* argv[], const char* envp[])
{
	char command[COMLEN] = "";
	char input[INPUTLEN] = "";
	char word[WORDLEN] = "";
	short int ComEnd = 0;
	CInterpret parsetree;

	cout << endl
	 << "         _       __  ______  __     ______  ____    __  ___  ______   " << endl
	 << "        | |     / / / ____/ / /    / ____/ / __ \\  /  |/  / / ____/  "  << endl
	 << "        | | /| / / / __/   / /    / /     / / / / / /|_/ / / __/      " <<   endl
	 << "        | |/ |/ / / /___  / /___ / /___  / /_/ / / /  / / / /___      " <<   endl
	 << "        |__/|__/ /_____/ /_____/ \\____/  \\____/ /_/  /_/ /_____/    " << endl << endl ;

	cout <<"  *************************************************************************" << endl
	 <<"  **                         Welcom to MiniSQL!                          **" << endl
	 <<"  **                    Copyright by                                     **" <<endl
	 <<"  **              Chukezhen honours College 2002 Mixedclass Team         **" << endl
	 <<"  *************************************************************************" << endl << endl;

	Catalog.ReadCatalog();
	CBufferManager::initiate_blocks();
	while(1)
	{
		strcpy(command, "");//command清零
		ComEnd = 0;

		while(!ComEnd)
		{
			printf("mini>> ");
			gets(input);
//			cout << "||" << input << "||" << endl;
			if(IsComEnd(input))
				ComEnd = 1;
			strcat(command, input);
			AddSeperator(command);
		}
		parsetree.Parse(command);
		parsetree.Execute();
	}
	return 0;
}
