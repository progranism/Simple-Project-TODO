#include <stdio.h>

void output_file(FILE *rp, char *name)
{
	unsigned int len = 0;
	unsigned char c;

	printf("{\"%s\", \"", name);
	while(1) {
		if(fread(&c, 1, 1, rp) == 0)
			break;

		printf("\\x%02x", (unsigned int)c);
		len++;
	}

	printf("\", %u},\n", len);
}

int main(int argc, char *argv[])
{
	FILE *rp;
	int i = 0;

	printf("struct DATAPACK datapacks[] = {\n");

	for(i = 1; i < argc; ++i)
	{
		rp = fopen(argv[i], "rb");
		if(rp != NULL)
		output_file(rp, argv[i]);
		fclose(rp);
	}

	printf("{NULL, NULL, 0}};\n");
	
	return 0;
}

