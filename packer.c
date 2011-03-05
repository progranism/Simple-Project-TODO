#include <packer.h>
#include <windows.h>

struct DATAPACK **unpack_self()
{
	FILE *fp;
	char buf[1024];

	GetModuleFileName(NULL, buf, 1024);

	fp = fopen(buf, "rb");

	// Find magic number
	while(1) {
		if(fread(buf, 1, 1, fp) == 0) {
			fclose(fp);
			return {NULL};
		}

		if(*buf == 
	}

	fclose(fp);
}

