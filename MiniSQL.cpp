// MiniSQL.cpp : Defines the entry point for the console application.
//

#include "MiniSQL.h"
#include "interpret.h"
#include "catalog_manager.h"
#include "buffer_manager.h"
#include "index_manager.h"
#include "record_manager.h"

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

	HANDLE hOut;  
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	cout << endl
	<<	"       ¡ö    ¡ö     ¡ö   ¡ö    ¡ö   ¡ö      ¡ö¡ö      ¡ö¡ö       ¡ö             " 	<< endl 
	<<  "      ¡ö¡ö  ¡ö¡ö    ¡ö   ¡ö¡ö  ¡ö   ¡ö    ¡ö    ¡ö  ¡ö    ¡ö     ¡ö             "	<< endl
	<<	"     ¡ö  ¡ö¡ö  ¡ö   ¡ö   ¡ö ¡ö ¡ö   ¡ö     ¡ö¡ö     ¡ö    ¡ö     ¡ö             "	<< endl 
	<<	"    ¡ö    ¡ö    ¡ö  ¡ö   ¡ö  ¡ö¡ö   ¡ö   ¡ö    ¡ö   ¡ö    ¡ö     ¡ö             " 	<< endl
	<<  "    ¡ö    ¡ö    ¡ö  ¡ö   ¡ö    ¡ö   ¡ö    ¡ö¡ö¡ö     ¡ö¡ö¡ö¡ö¡ö  ¡ö¡ö¡ö¡ö       "	<< endl << endl;
	
	
	
	Catalog.ReadCatalog();
	CBufferManager::initiate_blocks();
	while(1)
	{
		strcpy(command, "");
		ComEnd = 0;		
		cout << "------------------------------------------------------------------------------------------------------------------------ " << endl;
		SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_INTENSITY ); 
		cout << "query >> " << endl;
		SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE ); 
	
		while(!ComEnd)
		{

			gets(input);
			if(IsComEnd(input))
				ComEnd = 1;
			strcat(command, input);
			AddSeperator(command);
		}
		parsetree.Parse(command);
		SetConsoleTextAttribute(hOut, FOREGROUND_GREEN | FOREGROUND_INTENSITY ); 
		cout << endl << "Result >> " << endl;		
		SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE ); 
		parsetree.Execute();

	}
	return 0;
}
