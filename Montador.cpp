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

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <bits/stdc++.h>
#include "Montador.h"
using namespace std;

Montador::Montador(string asm_path_to_file){
    this->asm_path = asm_path_to_file;
    this->preprocessed_path = this->asm_path.substr(0, this->asm_path.find(".")) + ".pre"; // utiliza o asm_path para criar um path do arquivo .pre
    this->mounted_path = this->asm_path.substr(0, this->asm_path.find(".")) + ".obj"; // utiliza o asm_path para criar um path do arquivo .obj
    // inicializacao das strings de macro
    this->macro_label1  = "";
    this->macro_label2  = "";
    this->macro_command = "";
    this->macro1_arg1   = "";
    this->macro1_arg2   = "";
    this->macro2_arg1   = "";
    this->macro2_arg2   = "";
}

/* > incializar_processo(command)
 * Recebe o comando "-p" ou "-o" para determinar se o programa ira pre-processar ou montar
 */
void Montador::inicializar_processo(string command){
    this->directive_list.push_back("SPACE");
    this->directive_list.push_back("CONST");
    // opcode de valor 0 necessário para o indice da lista bater com o codigo do opcode
    this->opcode_list.push_back(make_pair("NEVERCALLED", 0));
    this->opcode_list.push_back(make_pair("ADD", 2));
    this->opcode_list.push_back(make_pair("SUB", 2));
    this->opcode_list.push_back(make_pair("MULT", 2));
    this->opcode_list.push_back(make_pair("DIV", 2));
    this->opcode_list.push_back(make_pair("JMP", 2));
    this->opcode_list.push_back(make_pair("JMPN", 2));
    this->opcode_list.push_back(make_pair("JMPP", 2));
    this->opcode_list.push_back(make_pair("JMPZ", 2));
    this->opcode_list.push_back(make_pair("COPY", 3));
    this->opcode_list.push_back(make_pair("LOAD", 2));
    this->opcode_list.push_back(make_pair("STORE", 2));
    this->opcode_list.push_back(make_pair("INPUT", 2));
    this->opcode_list.push_back(make_pair("OUTPUT", 2));
    this->opcode_list.push_back(make_pair("STOP", 1));
    if (command == "-o"){
        this->mount();
    }
    else if (command == "-p"){
        this->preprocess();
    }
    else{
        cerr << "Erro no comando!" << endl;
    }
}

/* > preprocess()
 * Abre o arquivo .asm, cria o arquivo .pre, retira os comentarios, substitui os char's por maiusculas,
 * troca tabs por espacos, remove linhas vazias e chama os handlers de macro e if/equ.
 */
void Montador::preprocess(){
    ifstream asm_file;
    ofstream preprocessed_file;

    asm_file.open(this->asm_path);
    preprocessed_file.open(this->preprocessed_path);
    if (!asm_file.is_open()){
        cerr << "Erro na abertura do arquivo .asm";
    }
    else if (!preprocessed_file.is_open()){
        cerr << "Erro na abertura do arquivo .pre";
    }
    else{
        while (asm_file.good()){
            getline(asm_file, this->line); // pega linha do arquivo .asm
            this->line = this->line.substr(0, this->line.find(";")); // retira os comentarios
            transform(
                this->line.begin(),
                this->line.end(),
                this->line.begin(),
                ::toupper); // substitui todos os char por maiusculas
            replace(this->line.begin(), this->line.end(), '\t', ' '); // retira tabs
            if(this->line[this->line.length() - 1] == ' '){ // retira espacos no final de comandos
                this->line = this->line.substr(0, this->line.length() -1);
            }
            if (!this->line.empty()){ // coloca no arquivo pre-processado caso a linha nao seja vazia
                preprocessed_file << this->line + "\n";
            }
        }
        asm_file.close();
        preprocessed_file.close();
        macro_handler();
        if_equ_handler();
    }
}

/* > macro_argument_finder(declaration_line, macro_count)
 * Recebe a primeira linha de declaracao do macro e um contador que determina se ele eh o primeiro ou
 * o segundo macro do programa. Acha quais sao os argumentos do determinado macro e os coloca nas 
 * variaveis de classe.
 */
void Montador::macro_argument_finder(string declaration_line, int macro_count){
    int macro_arg1_pos = 0, 
        macro_arg2_pos = 0, 
        occurrences = 0,
        flag_space_comma = 0;
    string arg_substr; // string com tudo depois da substring "MACRO ", que sao os argumentos.
    arg_substr = declaration_line.substr(declaration_line.find("MACRO") + 5, declaration_line.length());
    if(arg_substr.length() > 1){ // se existe algo depois da substring "MACRO ", ou seja, argumento
        for(size_t i = 1; i < arg_substr.length(); i++){
            if(arg_substr.at(i) == '&'){ // se acha algum '&' significa que achou um argumento
                occurrences++; // determina quantos argumentos tem no macro
                if(occurrences == 1){
                    macro_arg1_pos = i; // determina a posicao do inicio do argumento 1 na string.
                }
                else if(occurrences == 2){
                    macro_arg2_pos = i; // determina a posicao do inicio do argumento 2 na string, caso exista
                }
            }
        }
        for(int i = macro_arg1_pos; i <= macro_arg2_pos; i++){ // roda a string entre as posicoes do arg1 e arg2
            // subrotina para determinar se os dois argumentos estao separados por ',' ou por ", "
            if(arg_substr.at(i) == ','){
                flag_space_comma++;
                if(arg_substr.at(i+1) == ' '){
                    flag_space_comma++; // acresce a flag que vai auxiliar na separacao entre os dois argumentos
                }
            }
        }
        if(macro_count == 1){ // se for o primeiro macro do programa
            switch(occurrences){ // dependendo do numero de argumentos
                case 1: // seta o primeiro argumento na variavel de classe
                    this->macro1_arg1 = arg_substr.substr(macro_arg1_pos, arg_substr.length());
                    break;
                case 2: // seta os dois argumentos nas respectivas variaveis de classe
                    this->macro1_arg1 = arg_substr.substr(macro_arg1_pos, macro_arg2_pos - (flag_space_comma + 1));
                    this->macro1_arg2 = arg_substr.substr(macro_arg2_pos, arg_substr.length());
                    break;
            }
        }
        else if(macro_count == 2){ // faz o mesmo processo que foi feito no if acima
            switch(occurrences){
                case 1:
                    this->macro2_arg1 = arg_substr.substr(macro_arg1_pos, arg_substr.length());
                    break;
                case 2:
                    this->macro2_arg1 = arg_substr.substr(macro_arg1_pos, macro_arg2_pos - (flag_space_comma + 1));
                    this->macro2_arg2 = arg_substr.substr(macro_arg2_pos, arg_substr.length());
                    break;
            }
        }
    }
}

/* > macro_identifier()
 * Abre o arquivo pre-processado e identifica onde existem as declaracoes de macro, para adiciona-las a
 * listas de strings, para depois serem expandidos onde forem chamadas
 */
void Montador::macro_identifier(){
    int macro_count = 0;
    ifstream preprocessed_file;
    preprocessed_file.open(this->preprocessed_path);
    if (preprocessed_file.is_open()){
        while (preprocessed_file.good()){
            getline(preprocessed_file, this->line); // recebe linha do arquivo pre-processado
            if (this->line.find("MACRO") != std::string::npos){ // se nessa linha existir a string "MACRO"
                macro_count++; // acresce a quantidade de macros presentes no programa

                if (macro_count == 1){ // se ainda so existe um macro na funcao
                    macro_argument_finder(this->line, macro_count); // acha os argumentos do macro 1
                    this->macro_label1 = this->line.substr(0, this->line.find(":")); // recebe a label do macro 1
                    while (this->line.find("ENDMACRO") == std::string::npos){ // enquanto nao acha a substring "ENDMACRO"
                        getline(preprocessed_file, this->line); // recebe linha do arquivo preprocessado
                        this->macro_command = this->line;
                        this->macro_command_list1.push_back(this->macro_command); // coloca a linha na lista de comandos da macro 1
                    }
                    this->macro_command_list1.pop_back(); // retira a linha que possui o "ENDMACRO" da lista
                }
                else if (macro_count == 2){ // comportamento analogo a quando o macro_count == 1
                    macro_argument_finder(this->line, macro_count);
                    this->macro_label2 = this->line.substr(0, this->line.find(":"));
                    while (this->line.find("ENDMACRO") == std::string::npos){
                        getline(preprocessed_file, this->line);
                        this->macro_command = this->line;
                        this->macro_command_list2.push_back(this->macro_command);
                    }
                    this->macro_command_list2.pop_back();
                }
                else{
                    cerr << "Existem mais do que dois macros no programa." << endl;
                }
            }
        }
    }
    preprocessed_file.close();
}


/* > macro_expander()
 * Expande os macros cujas labels foram identificadas pelo macro_identifier
 */
void Montador::macro_expander(){
    int macro_position_begin, macro_position_end, macro_begin_flag = 0, macro_label_position;
    ifstream preprocessed_file_in;
    ofstream preprocessed_file_out;
    string command_line, macro_label_arg1 = "", macro_label_arg2 = "";
    vector<string> command_list, macro_command_list1_local, macro_command_list2_local;
    preprocessed_file_in.open(this->preprocessed_path);
    if (preprocessed_file_in.is_open()){
        while (preprocessed_file_in.good()){
            getline(preprocessed_file_in, this->line);
            if(!this->line.empty()){
                command_list.push_back(this->line); // coloca as linhas do arquivo em uma lista de strings
            }
        }
        preprocessed_file_in.close();

        // subrotina que retira as diretivas de declaracao de macro da lista command_list
        for (size_t i = 0; i < command_list.size(); i++){
            command_line = command_list.at(i);
            // procura a diretiva "MACRO" e marca sua posicao
            if (command_line.find("MACRO") != std::string::npos 
            && !macro_begin_flag){
                macro_position_begin = i;
                macro_begin_flag = 1;
            }
            // procura a diretiva "ENDMACRO" e remove tudo entre elas
            else if (command_line.find("ENDMACRO") != std::string::npos){ 
                macro_position_end = i;
                command_list.erase(
                    command_list.begin() + macro_position_begin,
                    command_list.begin() + macro_position_end + 1);
                macro_begin_flag = 0;
                i = 0;
            }
        }
        // fim da subrotina      
        for (size_t i = 0; i < command_list.size(); i++){
            command_line = command_list.at(i);
            // EXPANDE AS MACROS DA LABEL 1
            // se existe a substring de label, se nao e uma declaracao de label, ou seja, utiliza ":" e se
            // existe alguma label salva na macro_identifier
            if (command_line.substr(0, command_line.find(" ")) == this->macro_label1
            && command_line.find(":") == std::string::npos
            && !this->macro_label1.empty()){
                macro_label_position = this->macro_label1.length() + 1; // posicao da substring que contem os argumentos, se houver algum
                if(!macro1_arg2.empty()){ // se o argumento 2 existe, ou seja, se a macro_argument_finder achou dois argumentos
                    // separa os argumentos passados pela chamada da primeira macro
                    macro_label_arg1 = command_line.substr(macro_label_position, command_line.find(" "));
                    macro_label_arg2 = macro_label_arg1.substr(macro_label_arg1.find(",") + 1, macro_label_arg1.length());
                    macro_label_arg1 = macro_label_arg1.substr(0, macro_label_arg1.find(","));
                    if(macro_label_arg1.at(0) == ' '){ // retira espacos caso haja algum no inicio da string do argumento
                        macro_label_arg1.erase(0, 1);
                    }
                    if(macro_label_arg2.at(0) == ' '){ // retira espacos caso haja algum no inicio da string do argumento
                        macro_label_arg2.erase(0, 1);
                    }
                    // recebe a lista de comandos do macro1 para uma lista local, para nao alterar nada na lista original
                    macro_command_list1_local = this->macro_command_list1; 
                    for(size_t j = 0; j < macro_command_list1_local.size(); j++){
                        // procura o primeiro argumento original declarado no macro
                        if(macro_command_list1_local.at(j).find(macro1_arg1) != std::string::npos){
                            // substitui o primeiro argumento original pelo primeiro argumento utilizado pela chamada do macro
                            macro_command_list1_local.at(j).replace(
                                macro_command_list1_local.at(j).find(macro1_arg1),
                                macro1_arg1.length(),
                                macro_label_arg1
                            );
                        }
                        // comportamento analogo ao if acima, para o segundo argumento da macro
                        if(macro_command_list1_local.at(j).find(macro1_arg2) != std::string::npos){
                            macro_command_list1_local.at(j).replace(
                                macro_command_list1_local.at(j).find(macro1_arg2),
                                macro1_arg2.length(),
                                macro_label_arg2
                            );
                        }
                    }
                }
                else if(!macro1_arg1.empty()){ // se somente o argumento 1 existe
                    // separa o argumento passado pela chamada da primeira macro
                    macro_label_arg1 = command_line.substr(macro_label_position, command_line.length());
                    // recebe a lista de comandos do macro1 para uma lista local, para nao alterar nada na lista original
                    macro_command_list1_local = this->macro_command_list1;
                    for(size_t j = 0; j < macro_command_list1_local.size(); j++){
                        // procura o argumento original declarado no macro
                        if(macro_command_list1_local.at(j).find(macro1_arg1) != std::string::npos){
                            // substitui na lista de comandos o argumento original pelo argumento utilizado pela chamada do macro
                            macro_command_list1_local.at(j).replace(
                                macro_command_list1_local.at(j).find(macro1_arg1),
                                macro1_arg1.length(),
                                macro_label_arg1
                            );
                        }
                    }
                }
                else{
                    // caso nao haja argumentos, somente passa a lista de comandos da macro para a lista local
                    macro_command_list1_local = this->macro_command_list1;
                }
                command_list.erase(command_list.begin() + i); // remove a linha de chamada da macro no programa
                // coloca no lugar da linha removida a lista de comandos da macro, com os argumentos ja substituidos
                command_list.insert(
                    command_list.begin() + i,
                    macro_command_list1_local.begin(),
                    macro_command_list1_local.end()
                );
            }

            // EXPANDE AS MACROS DA LABEL 2
            // O COMPORTAMENTO DE EXPANSAO DAS MACROS DA LABEL 2 E IDENTICO AO COMPORTAMENTO
            // DE EXPANSAO DAS MACROS DA LABEL 1, PORTANTO NAO SERAO DOCUMENTADOS
            if (command_line.substr(0, command_line.find(" ")) == this->macro_label2
            && command_line.find(":") == std::string::npos
            && !this->macro_label2.empty()){
                macro_label_position = this->macro_label2.length() + 1;
                if(!macro2_arg2.empty()){
                    macro_label_arg1 = command_line.substr(macro_label_position, command_line.find(","));
                    macro_label_arg2 = macro_label_arg1.substr(macro_label_arg1.find(",") + 1, macro_label_arg1.length());
                    macro_label_arg1 = macro_label_arg1.substr(0, macro_label_arg1.find(","));
                    if(macro_label_arg1.at(0) == ' '){
                        macro_label_arg1.erase(0, 1);
                    }
                    if(macro_label_arg2.at(0) == ' '){
                        macro_label_arg2.erase(0, 1);
                    }
                    macro_command_list2_local = this->macro_command_list2;
                    for(size_t j = 0; j < macro_command_list1_local.size(); j++){
                        if(macro_command_list2_local.at(j).find(macro2_arg1) != std::string::npos){
                            macro_command_list2_local.at(j).replace(
                                macro_command_list2_local.at(j).find(macro2_arg1),
                                macro2_arg1.length(),
                                macro_label_arg1
                            );
                        }
                        if(macro_command_list2_local.at(j).find(macro2_arg2) != std::string::npos){
                            macro_command_list2_local.at(j).replace(
                                macro_command_list2_local.at(j).find(macro2_arg2),
                                macro2_arg2.length(),
                                macro_label_arg2
                            );
                        }
                    }
                }
                else if(!macro2_arg1.empty()){
                    macro_label_arg1 = command_line.substr(macro_label_position, command_line.length());
                    macro_command_list2_local = this->macro_command_list2;
                    for(size_t j = 0; j < macro_command_list2_local.size(); j++){
                        if(macro_command_list2_local.at(j).find(macro2_arg1) != std::string::npos){
                            macro_command_list2_local.at(j).replace(
                                macro_command_list2_local.at(j).find(macro2_arg1),
                                macro2_arg1.length(),
                                macro_label_arg1
                            );
                        }
                    }
                }
                else{
                    macro_command_list2_local = this->macro_command_list2;
                }
                command_list.erase(command_list.begin() + i);
                command_list.insert(
                    command_list.begin() + i,
                    macro_command_list2_local.begin(),
                    macro_command_list2_local.end()
                );
            }
        }

        remove(this->preprocessed_path.c_str()); // remove o arquivo .pre existente anteriormente
        preprocessed_file_out.open(this->preprocessed_path);
        // reescreve o arquivo .pre com a remocao de declaracao de macro, e expansao destes nas suas chamadas
        for (size_t i = 0; i < command_list.size(); i++){
            preprocessed_file_out << command_list.at(i) << "\n";
        }
        preprocessed_file_out.close();
    }
}

/* > if_equ_handler()
 * trata as chamadas das diretivas de EQU's e IF's presentes no codigo, e as remove do arquivo .pre
 */
void Montador::if_equ_handler(){
    vector<string> command_list;
    vector< pair <string,string> > equ_statements;
    string  equ_label = "",
            equ_value = "",
            if_value  = "";
    ifstream preprocessed_file_in;
    ofstream preprocessed_file_out;

    preprocessed_file_in.open(this->preprocessed_path);
    if (preprocessed_file_in.is_open()){
        while (preprocessed_file_in.good()){
            getline(preprocessed_file_in, this->line);
            command_list.push_back(this->line); // passa as linhas do arquivo .pre para uma lista de comandos
        }
    }
    preprocessed_file_in.close();
    //IDENTIFICACAO DO EQU
    //Inicializa a lista de pares {string,string} para lidar com as declaracoes de EQU e seus valores
    for(size_t i = 0; i < command_list.size(); i++){
        if(command_list.at(i).find("EQU") != std::string::npos){ // se acha a declaracao do "EQU"
            // guarda a label do EQU, para depois ser feita a substituicao onde ela for chamada
            equ_label = command_list.at(i).substr(
                0,
                command_list.at(i).find(":")
            );
            // guarda o valor do EQU, para ser feita a substituicao da label pelo valor onde for chamada
            equ_value = command_list.at(i).substr(
                command_list.at(i).find("EQU") + 4,
                command_list.at(i).length()
            );
            // coloca na lista de pares a label com o seu respectivo valor
            equ_statements.push_back(make_pair(equ_label, equ_value));
            // retira da lista de comandos a declaracao do EQU
            command_list.erase(command_list.begin() + i);
            i--;
        }
    }
    // SUBSTITUICAO DAS LABELS PELO VALOR DO EQU
    for(size_t i = 0; i < command_list.size(); i++){
        for(size_t j = 0; j < equ_statements.size(); j++){
            // se acha alguma label presente na lista de pares em alguma linha da lista de comandos
            if(command_list.at(i).find(equ_statements.at(j).first) != std::string::npos){
                // substitui a label presente pelo seu respectivo valor
                command_list.at(i).replace(
                    command_list.at(i).find(equ_statements.at(j).first),
                    equ_statements.at(j).first.length(),
                    equ_statements.at(j).second
                );
            }
        }
    }
    // CHECA SE VAI SER COMPUTADO O IF OU NAO 
    for(size_t i = 0; i < command_list.size(); i++){
        if(command_list.at(i).find("IF") != std::string::npos){ // se achar a diretiva IF
            // recebe o valor do IF
            if_value = command_list.at(i).substr(
                command_list.at(i).find("IF") + 3,
                command_list.at(i).length()
            );
            // caso o valor do IF for diferente de 1 ou variacoes
            if(if_value != "1" 
            && if_value != "1 " 
            && if_value != " 1"
            && if_value != " 1 "){
                // deleta o IF junto com a linha abaixo dele
                command_list.erase(
                    command_list.begin() + i,
                    command_list.begin() + i+2 
                );
            }
            // caso contrario
            else{
                // deleta apenas a linha do IF
                command_list.erase(command_list.begin() + i);
            }
            i--;
        }
    }
    remove(this->preprocessed_path.c_str());
    preprocessed_file_out.open(this->preprocessed_path);
    // reescreve o arquivo .pre com as diretivas de IF e EQU devidamente tratadas
    for (size_t i = 0; i < command_list.size(); i++){
        preprocessed_file_out << command_list.at(i) << "\n";
    }
    preprocessed_file_out.close();
}

/* > macro_handler()
 * Chama as funcoes de identificacao e expansao dos macros
 */
void Montador::macro_handler(){
    macro_identifier();
    macro_expander();
}

/* > first_passage()
 * Faz a primeira passagem do montador, que consiste em identificar as labels presentes no codigo e 
 * popular a tabela de simbolos, que neste algoritmo eh uma lista de pares {string,int}, onde o primeiro
 * elemento eh a label e o segundo elemento eh a sua posicao da memoria do programa em assembly.
 */
void Montador::first_passage(){
    int superscription_error_flag = 0,
        position_counter = 0;
    pair <string,int> aux_pair;
    vector<string> command_list, command_line;
    string  symbol_label = "";
    ifstream preprocessed_file_in;
    preprocessed_file_in.open(this->preprocessed_path);
    if (preprocessed_file_in.is_open()){
        while (preprocessed_file_in.good()){
            getline(preprocessed_file_in, this->line);
            command_list.push_back(this->line); // coloca as linhas do arquivo em uma lista de strings
        }
    }
    preprocessed_file_in.close();
    for(size_t i = 0; i < command_list.size(); i++){
        superscription_error_flag = 0;
        // subrotina para separar todas as substrings de uma linha da lista de comandos
        istringstream iss(command_list.at(i));
        command_line.clear();
        for(string s; iss >> s;){
            command_line.push_back(s);
        }
        // fim da subrotina
        for(auto& s: command_line){ // para cada substring, ou "token", da linha de comando
            if(s.find(":") != std::string::npos){ // se achar um :, quer dizer que existe uma declaracao de label
                // retira o : da substring e coloca numa string separada
                symbol_label = s.substr(
                    0,
                    s.length() - 1
                );
                // caso essa label ja exista na tabela de simbolos, sobe uma flag de erro para nao sobrescreve-la
                for(size_t j = 0; j < this->symbol_table.size(); j++){
                    if(this->symbol_table.at(j).first == symbol_label){
                        superscription_error_flag = 1;
                    }
                }
                // caso nao tenha dado o erro de sobrescricao da label, coloca a label na tabela de simbolos,
                // junto com a sua posicao de memoria do programa
                if(!superscription_error_flag){
                    aux_pair = make_pair(symbol_label, position_counter);
                    this->symbol_table.push_back(aux_pair);
                }
            }
            // se a substring for um opcode, soma no contador de posicao de memoria do programa o "valor de memoria" desse opcode
            for(size_t j = 0; j < this->opcode_list.size(); j++){
                if(this->opcode_list.at(j).first == s){
                    position_counter += this->opcode_list.at(j).second;
                }
            }
            // se a substring for uma diretiva, ou seja, "SPACE" ou "CONST", soma um ao contador de posicao.
            for(size_t j = 0; j < this->directive_list.size(); j++){
                if(this->directive_list.at(j) == s){
                    position_counter += 1;
                }
            }
        }
    }
    // for(size_t i = 0; i < this->symbol_table.size(); i++){
    //     cout << this->symbol_table.at(i).first << '\t' << this->symbol_table.at(i).second << endl;
    // }
}

/* > second_passage()
 * A segunda passagem substitui as chamadas de label pela sua posicao de memoria, 
 * transforma os opcodes em seus respectivos codigos e passa todos os codigos para o arquivo .obj
 */
void Montador::second_passage(){
    int flag_const          = 0,
        flag_section_text   = 0,
        flag_section_data   = 0;
    vector<int> object_list;
    vector<string> command_list, command_line;
    string  symbol_label = "";
    ifstream preprocessed_file;
    ofstream mounted_file;
    preprocessed_file.open(this->preprocessed_path);
    if (preprocessed_file.is_open()){
        while (preprocessed_file.good()){
            getline(preprocessed_file, this->line);
            command_list.push_back(this->line); // passa as linhas do arquivo .pre para uma lista de comandos
        }
    }
    preprocessed_file.close();
    // para cada linha da lista de comandos
    for(size_t i = 0; i < command_list.size(); i++){
        // se a linha mostrar que esta na secao de texto, sobe a flag que esta nela ate ir para a secao de dados
        if(command_list.at(i) == "SECTION TEXT"){
            flag_section_text = 1;
            flag_section_data = 0;
        }
        // se a linha mostrar que esta na secao de dados, sobe a flag que esta nela
        else if(command_list.at(i) == "SECTION DATA"){
            flag_section_text = 0;
            flag_section_data = 1;
        }
        // subrotina para eliminar a virgula entre os argumentos do opcode COPY
        if(command_list.at(i).find("COPY") != std::string::npos){
            command_list.at(i).erase(
                remove(
                    command_list.at(i).begin(), 
                    command_list.at(i).end(), 
                    ','), 
                command_list.at(i).end()
            );
        }
        // fim da subrotina
        // subrotina para separar todas as substrings de uma linha da lista de comandos
        istringstream iss(command_list.at(i));
        command_line.clear();
        for(string s; iss >> s;){
            command_line.push_back(s);
        }
        // fim da subrotina
        for(auto& s: command_line){ // para cada substring da linha de comando
            // se estiver dentro da secao de texto
            if(flag_section_text){
                for(size_t j = 1; j < this->opcode_list.size(); j++){
                    // se a substring for um opcode, coloca na lista de objetos o codigo do opcode
                    if(s == this->opcode_list.at(j).first){
                        int converter = static_cast<int>(j);
                        object_list.push_back(converter);
                    }
                }
                for(size_t j = 0; j < this->symbol_table.size(); j++){
                    // se a substring for uma label, varre a tabela de simbolos ate encontrar, e entao
                    // coloca na lista de objetos a posicao de memoria gravada na tabela
                    if(s == this->symbol_table.at(j).first){
                        object_list.push_back(this->symbol_table.at(j).second);
                    }
                }
            }
            else if(flag_section_data){
                // se a substring anterior tivesse sido CONST
                // coloca na lista de objetos o valor inteiro do valor desse const
                if(flag_const){
                    object_list.push_back(stoi(s));
                    flag_const = 0;
                }
                // se a substring for a diretiva SPACE coloca um "0" na lista de objetos
                else if(s == this->directive_list.at(0)){
                    object_list.push_back(0);
                }
                // se a substring for a diretiva CONST,
                // sobe a flag para armazenar a proxima substring na lista de objetos
                else if(s == this->directive_list.at(1)){
                    flag_const = 1;
                }
            }
        }
    }
    // cria o arquivo .obj e coloca nele a lista de objetos criada acima, separando os objetos com " "
    mounted_file.open(this->mounted_path);
    if (!mounted_file.is_open()){
        cerr << "Erro na abertura do arquivo .obj";
    }
    else{    
        for(size_t i = 0; i < object_list.size(); i++){
            if(i < object_list.size() - 1){
                mounted_file << object_list.at(i) << " ";
            }
            else{
                mounted_file << object_list.at(i) << "\n";
            }
        }
    }
    mounted_file.close();

    // for(size_t i = 0; i < object_list.size(); i++){
    //     if(i < object_list.size() - 1){
    //         cout << object_list.at(i) << " ";
    //     }
    //     else{
    //         cout << object_list.at(i) << "\n";
    //     }
    // }
}

/* > mount()
 * Chama a primeira passagem e segunda passagem do montador
 */
void Montador::mount(){
    first_passage();
    second_passage();
}