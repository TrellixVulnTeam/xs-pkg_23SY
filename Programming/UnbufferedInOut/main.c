#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main (void) {
	char* Buffer[255];
	int CurrentSize = 0;
	while ( ( CurrentSize = read(STDIN_FILENO, Buffer, 255 )) > 0 )
		   write(STDOUT_FILENO, Buffer, CurrentSize );	
		
	exit(EXIT_SUCCESS);
}
