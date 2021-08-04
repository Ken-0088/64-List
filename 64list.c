/* 64list - Read the directory of a Commodore formatted .D64 image file */


#include <stdio.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

FILE *stream;

int read_directory(char *filename);

int main(int argc, char *argv[])
{
	char ext[5];
	if(argc<2)
	{
		fprintf(stderr, "\nSyntax: 64list <filename>\n");
		return 1;
	}
	int x = strlen(argv[1]) - 1;
	if(x<4)
		return 1;
	for(int i=0; i<4; i++)
		ext[i]=argv[1][x-i];
	if(strcmp("46d.", ext)==0 || strcmp("46D.", ext)==0)
		read_directory(argv[1]);
	else
		fprintf(stdout, "File must have .D64 extension!\n");
	return 0;
}

int read_directory(char *filename)
{
	int i, j, blocks, free=0, s=1, last=FALSE;
	unsigned char x, buf[4864];

	char *fileType[] = { "DEL", "SEQ", "PRG", "USR", "REL" };
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
		fprintf(stdout, "0 \"");

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
			fprintf(stdout, "%c", buf[i]==160 ? 32 : buf[i]);
			if(i==16)
				fprintf(stdout, "\"");
		}

		// Read directory entries
		fseek(stream, 86, SEEK_CUR); // offset: 91648
		fread(&buf, 4864, 1, stream);
		while(!last)
		{
			if(s==1 && buf[1]==0)
				last=TRUE;
			if(buf[dir_sect[s]+1]==255)
				last=TRUE;
			for(i=dir_sect[s]; i<dir_sect[s]+256; i+=32)
			{
				if ( buf[i+5]==0 || buf[i+5]==1 || buf[i+2]== 0 )
					continue;
				// Size of file in blocks
				blocks = buf[i+30]+buf[i+31]*256;
				fprintf(stdout, "\n%-3d  ", blocks);
                
				// File name in standard ASCII.  Will get inaccurate characters if
				// SHIFTED PETSCII characters are in the filename.
				// GEOS disks will display OK.
				for(j=5; j<21; j++)
				{
					if (buf[i+j] == 192)
						buf[i+j] = 45;
					fprintf(stdout, "%c", buf[i+j] == 160 ? 32 : buf[i+j]);
				}
				// Type of file (PRG, SEQ etc.) and if locked
				int a=buf[i+2] & 7;
				if (a>4)
					continue;
				fprintf(stdout, "    %s%s", fileType[a], (buf[i+2] & 64) ? " < " : "");
			}
			s=buf[dir_sect[s]+1];
			if(s <= 1)
				break;
		}
		fprintf(stdout, "\n%d BLOCKS FREE\n", free);
	}
	else
		fprintf(stdout, "Incorrect size for a D64 image!  Aborting.\n");

	fclose(stream);
	return 0;
}
