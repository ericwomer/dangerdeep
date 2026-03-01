#include "parser.h"
#include <cstdlib>
#include <iostream>
#include <string>
int main(int argc, char **argv) {
    const char *data_env = std::getenv("DFTD_DATA");
    std::string csv_path = (argc >= 2) ? argv[1] : (data_env ? std::string(data_env) + "/texts/common.csv" : "");
    if (csv_path.empty()) {
        std::cerr << "Uso: parser_test <ruta_a_data> o DFTD_DATA=<dir_data>\n";
        return 1;
    }
    parser p(csv_path);
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
    return 0;
}
