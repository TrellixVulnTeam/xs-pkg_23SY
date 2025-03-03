#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char* argv[]) {
	DIR* dp = NULL;
	struct dirent* dir = NULL; 
	if ( argc != 2 ) {
		fprintf(stderr,"Usage: main directory_name\n");
		exit(EXIT_FAILURE);
	}

	if ( (dp = opendir(argv[1]) ) == NULL ) {
		fprintf(stderr, "Can't open %s.", argv[1]);
		exit(EXIT_FAILURE);
	}
	while ( (dir = readdir(dp) ) != NULL ) 
		fprintf(stdout, "%s\n", dir->d_name);

	closedir(dp);
	exit(EXIT_SUCCESS);	
}
