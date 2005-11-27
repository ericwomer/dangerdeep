// Danger from the Deep, standard error/exception
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "error.h"

#if 1
error::error(const std::string& s)
	: msg(s)
{
}

#else

// debugging constructor...
// fixme: add stack trace printing here
error::error(const std::string& s)
	: msg(s)
{
	printf("exception: %s\n", msg.c_str()); char c = *(char*)0; printf("%c\n",c);
}
#endif
