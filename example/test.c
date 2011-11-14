#include <stdio.h>
#include <string.h>
#include "stringswitch.h"


//stringswitch_gen add argv0 "hi"
//stringswitch_gen add argv0 "lo"
//stringswitch_gen add argv0 "hello"
//stringswitch_gen add argv0 "holla"
//stringswitch_gen add argv0 "hollu"
//stringswitch_gen add argv0 "hodlu"
//stringswitch_gen add argv0 "bye"

#include "stringswitch_impl_argv0.c"

int main(int argc, char** argv) {
#ifdef STATIC
	stringswitch_s(argv0, "hodlu") {
#else
	stringswitch_d(argv0, argv[1]) {
#endif
			stringcase(argv0, hi): stringcase (argv0, hello):
				puts("hi");
				break;
			stringcase(argv0, lo):
				puts("lo!!");
				break;
			stringcase (argv0, bye):
				puts("bye");
				break;
			stringcase_default(argv0):
				puts("unknown command!");
				break;
			stringcase(argv0, holla):
				puts("holla!");
				break;
			stringcase(argv0, hollu):
				puts("hollu!");
				break;
			stringcase(argv0, hodlu):
				puts("hodlu!");
				break;
				
	};
	
	return 0;
}
