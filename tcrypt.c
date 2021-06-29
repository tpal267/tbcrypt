#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_KEY_SIZE 1025
#define CHUNK_SIZE 524288	//Half-mebibyte, for the curious. I tried a full mebibyte, but that 
							//causes Windows to treat the program like Visa treats a credit card  
							//trying to pull 10k out of an ATM in Latvia.

typedef struct key_str {
	char* str;
	size_t keylen;
}key_str;

inline int cipher(char* str, key_str* key, size_t count);

long int file_size(FILE* fp);

int main(int argc, char * argv[])
{

	FILE* fin = NULL;
	FILE* fout = NULL;
	//char key[MAX_KEY_SIZE];

	key_str key;

	if (argc == 1)
	{
		char iname[128];
		char oname[128];
		char tmpstr[MAX_KEY_SIZE];
		
		printf("Enter the name of your input file. \n(Enter ABORT to cancel): ");
		scanf_s("%s", iname, 127);
		if (!strncmp(iname, "ABORT", 5)) { return -5; }
		
		printf("Enter the name of your output file. \n(Same named preexisting file will be overwritten) \n(Enter ABORT to cancel): ");
		scanf_s("%s", oname, 127);
		if (!strncmp(oname, "ABORT", 5)) { return -5; }
		
		printf("Enter your encryption key. \n(Enter ABORT to cancel): ");
		scanf_s("%s", tmpstr, MAX_KEY_SIZE-1);
		if (!strncmp(tmpstr, "ABORT", 5)) { return -5; }
		else 
		{ 
			key.str = strdup(tmpstr); 
			key.keylen = strlen(tmpstr)-1; // Less one to avoid using the null character
		}

		fopen_s(&fin, iname, "rb");
		if (!fin) { perror("Failed opening to read"); return -2; }
		fopen_s(&fout, oname, "wb");
		if (!fout) { perror("Failed opening to write"); return -2; }
	}
	else if (argc != 4) {
		printf("Improper number of arguments.\nTerminating.\n");
		return -1;
	}
	else
	{
		//OPEN_FILESTREAMS 
		fopen_s(&fin, argv[1], "rb"); 
		if (!fin) { perror("Failed opening to read"); return -2; }
		fopen_s(&fout, argv[2], "wb"); 
		if (!fout) { perror("Failed opening to write"); return -2; }

		//strcpy_s(key, MAX_KEY_SIZE - 1, argv[3]);
		//key[MAX_KEY_SIZE - 1] = '\0';
		key.str = strdup(argv[3]);
		key.keylen = strlen(argv[3])-1;
	}

	size_t i = 0;
	size_t lastpos = 0;
	char data[CHUNK_SIZE];

	long int sz = file_size(fin);

	do
	{
		fread(data, 1, CHUNK_SIZE, fin);
		if (ferror(fin)) { perror("Error performing file read"); return -3; }
		else
		{
			i = ftell(fin) - lastpos;				//fread's return value gets stuck if it sees \x25 or \x26, so I'm using this.
			cipher(data, &key, i);
			fwrite(data, sizeof data[0], i, fout);	//Is trusting fwrite to not fail more important than removing function calls?
			lastpos = ftell(fin);					//Heck if I know.

			printf("Current progress: %ld KB of %ld KB\r", lastpos / 1000, sz / 1000);	//Display progress as fraction
			//printf("Current progress: %%%.0f\r", (float)lastpos / (float)sz * 100);	//Display progress as percent (seems slower)
			fflush(stdout);
		}
	} while (!feof(fin));

	printf("\n");
	free(key.str);
	return 0;
}

inline int cipher(char* str, key_str* key, size_t str_length)
{
	for (unsigned int i = 0; i < str_length; i++)
	{
		str[i] ^= key->str[i%key->keylen];
	}

	return 0;
}

long int file_size(FILE* fp)
{
	fseek(fp, 0L, SEEK_END); // Not universally portable, but the major x86 compilers will support it.
	long int sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	return sz;
}