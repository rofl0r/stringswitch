#include <string.h>
#if 0
void puts(char* str) {
	(void) str;
}
#else
#include <stdio.h>
#endif

int main(int argc, char** argv) {

size_t i = 0;
while(i < 10000000) {

	if(strcmp(argv[1], "hi") == 0 || strcmp(argv[1], "hello") == 0)
				puts("hi");
	else 
	if(strcmp(argv[1], "lo") == 0)
				puts("lo!!");
	else 
	if(strcmp(argv[1], "bye") == 0)
				puts("bye");
	else 
	if(strcmp(argv[1], "holla") == 0)
				puts("holla!");
	else 
	if(strcmp(argv[1], "hollu") == 0)
				puts("hollu!");
	else 
	if(strcmp(argv[1], "hodlu") == 0)
				puts("hodlu!");
	else 
	{
				puts("unknown command!");
				
	};
i++;
}
	return 0;
}
