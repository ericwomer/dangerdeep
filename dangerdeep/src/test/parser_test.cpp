#include "parser.h"
#include <iostream>
int main()
{
	parser p("/home/tj/subsim/dangerdeep/data/texts/common.csv");
	unsigned l = 0;
	do {
		unsigned o = 0;
		do {
			std::string c = p.get_cell();
			unsigned n = 0;
			bool ok = p.get_cell_number(n);
			std::cout << "line " << l << " col " << o << " is \"" << c << "\" is nr: " << ok << " n=" << n << "\n";
			++o;
		} while (p.next_column());
		++l;
	} while (p.next_line());
}
