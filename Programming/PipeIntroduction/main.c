#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>

//****************************************************
int main (int argc, char* argv[] ) {
	//******** Handler input data ********
	if ( argc == 1 ) {
		fprintf ( stderr, "No arguments.\n" );
		exit(EXIT_FAILURE);
	}
	FILE* FilePointer = fopen( argv[1], "r" );
	if ( FilePointer == NULL ) {
		fprintf ( stderr, "File not exist.\n" );
		exit(EXIT_FAILURE);	
	}
	//============================================
	int fd[2];
	pipe(fd);

	fseek ( FilePointer, 0, SEEK_END );
	int FileLength = ftell ( FilePointer );
	rewind ( FilePointer);

	int pid = fork();
	if ( pid == -1 ) {
		fprintf(stderr, "Can't create proccess!\n");
		exit(EXIT_FAILURE);
	}	
	if ( pid == 0 ) {
		//Child
		close(fd[0]); //Close unused write end

		char* Buffer = (char*) calloc ( 4096, sizeof(*Buffer) );

		int CurrentBuf = 0;
		if ( FileLength >= 4096 ) {
			CurrentBuf = 4096;
			FileLength -= CurrentBuf;
		} else
			CurrentBuf = FileLength;

		while ( fread( Buffer, CurrentBuf, 1, FilePointer ) > 0 ) {
			write(fd[1], Buffer, CurrentBuf );
			if ( FileLength >= 4096 ) {
				CurrentBuf = 4096;
				FileLength -= CurrentBuf;
			} else
				CurrentBuf = FileLength;	 
		}
		close(fd[1]);
		fclose(FilePointer);
		free(Buffer);
		exit(EXIT_SUCCESS);
	} else {
		//Parent
		close(fd[1]);
		int buf = 0;
		char* Buffer = (char*) calloc (4096, sizeof(*Buffer) );
		while ( ( buf=read(fd[0],Buffer,4096)) > 0 ) {
			write( STDOUT_FILENO, Buffer, buf );
		}
		wait(NULL);
		close(fd[0]);
		free(Buffer);
	}
	fclose(FilePointer);
	exit(EXIT_SUCCESS);
}
