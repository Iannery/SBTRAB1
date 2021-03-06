/****************************************
 * Trabalho 1 - Software Basico         *
 *                                      *
 * Ian Nery Bandeira                    *
 * 170144739                            *
 *                                      *
 * Versao do compilador:                *
 * g++ (Ubuntu 9.3.0-10ubuntu2) 9.3.0   *
 *                                      *
 ****************************************/

#ifndef MONTADOR_BIB
#define MONTADOR_BIB

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class Montador {
public:
    Montador(string asm_path_to_file);
    ~Montador();
    void inicializar_processo(string command);

private:
    void preprocess();
    void mount();
    void macro_handler();
    void macro_identifier();
    void macro_expander();
    void macro_argument_finder(string declaration_line, int macro_count);
    void if_equ_handler();
    void first_passage();
    void second_passage();
    string asm_path, preprocessed_path, mounted_path, line;
    vector<string> directive_list;
    vector<pair<string, int>> symbol_table, opcode_list;
    vector<string> macro_command_list1, macro_command_list2;
    string macro_label1, macro_label2, macro_command;
    string macro1_arg1, macro1_arg2, macro2_arg1, macro2_arg2;
};


#endif