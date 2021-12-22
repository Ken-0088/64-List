/*
64list - Read the directory of a Commodore formatted .D64 image file

Compiler:  Microsoft (R) C/C++ Optimizing Compiler Version 19.29.30133 for x86
*/

#include <stdio.h>
#include <string.h>

#include "64list.h"

FILE *stream;

int read_directory(char *filename);
unsigned char petscii(unsigned char c);

int main(int argc, char *argv[])
{
	char ext[5] = {0};
	if(argc<2)
	{
		fprintf(stderr, "\nSyntax: 64list <filename>\n\n");
		return 1;
	}
	int x = strlen(argv[1])-1;
	for(int i=0; i<4; i++)
		ext[i]=argv[1][x-i];
	if( strcmp(ext,"46D.")==0 || strcmp(ext,"46d.")==0 )
		read_directory(argv[1]);
	else
		fprintf(stdout, "File must have .D64 extension!\n");
	return 0;
}

int read_directory(char *filename)
{
	int i, j, blocks, free=0, s=1;
	unsigned char x, buf[4864];
	bool qt, last = false;

	int dir_sect[20] = { 0, 0, 256, 512, 768, 1024, 1280, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4096, 4352, 4608 };

	if((stream=fopen(filename, "rb"))== NULL)
	{
		fprintf(stderr, "Cannot open input file!\n");
		return 1;
	}
	// Check for correct image size.  This is a simple check for a
	// a valid D64 image.  There are better ways to validate.
	fseek(stream,0L,SEEK_END);
    int length=ftell(stream);
    rewind(stream);
    if(length == 174848) // size is correct, continue
    {
		// Line zero is hard coded
		fprintf(stdout, "\n0 \"");

		// Get free blocks from BAM
		fseek( stream, 91396, 0 );
		fread( &buf, 140, 1, stream );
		for( i=0; i<35; i++ )
		{
			if( i != 17 )
				free += buf[i*4];
		}
		// Get disk name, type, and ID
		fread(&buf, 26, 1, stream); // offset: 91536
        for(i=0; i<26; i++)
		{
			if(buf[i]==0)
				break;
			fprintf(stdout, "%c", buf[i]==160 ? 32 : petscii(buf[i]));
			if(i==16)
				fprintf(stdout, "\"");
		}
		// Read directory entries
		fseek(stream, 86, SEEK_CUR); // offset: 91648
		fread(&buf, 4864, 1, stream);
		while(!last)
		{
			if(s==1 && buf[1]==0)
                last = true;
			if(buf[dir_sect[s]+1]==255)
                last = true;
			for(i=dir_sect[s]; i<dir_sect[s]+256; i+=32)
			{
				if ( buf[i+5]==0 || buf[i+5]==1) // || buf[i+2]== 0 )
					continue;
				// Size of file in blocks
				blocks = buf[i+30]+buf[i+31]*256;
				fprintf(stdout, "\n%-3d  \"", blocks);
                qt = false;
                for(j=5; j<21; j++)
				{
					if (buf[i+j] == 192)
						buf[i+j] = 45;
                    if (buf[i+j] == 160 && !qt) {
                        fprintf(stdout, "\"");
                        qt = true;
                    }
                    else
                        fprintf(stdout, "%c", buf[i+j] == 160 ? 32 : petscii(buf[i+j]));
				}
				// Type of file (PRG, SEQ etc.) and if locked
				int a = buf[i+2];
				if(a == 0)
					fprintf(stdout, "   scratched");
				else
				{
					if( a & 128 )
						fprintf(stdout, "   %s%s", fileType[a & 7], (a & 64) ? "< " : ""); // closed file
					else
						fprintf(stdout, "  *%s%s", fileType[a & 7], (a & 64) ? "< " : ""); // unclosed file
				}
			}
			s=buf[dir_sect[s]+1];
			if(s <= 1)
				break;
		}
		fprintf(stdout, "\n%d blocks free\n", free);
	}
	else
		fprintf(stdout, "Incorrect size for a D64 image!  Aborting.\n");

	fclose(stream);
	return 0;
}

unsigned char petscii(unsigned char c)
{
	if(c >  64 && c <  91) c+=32;
	if(c > 192 && c < 219) c-=128;
	return c;
}
