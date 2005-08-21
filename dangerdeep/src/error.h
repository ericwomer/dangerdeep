// Danger from the Deep, standard error/exception
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <exception>

// always throw this exception or a heir of it.
class error : public std::exception
{
 private:
	error();
	std::string msg;
 public:
	error(const std::string& s) : msg(s) {}
	virtual ~error() throw() {}
	const char* what() const throw() { return msg.c_str(); }
};

#endif
