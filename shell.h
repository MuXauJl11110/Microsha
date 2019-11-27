#ifndef INC_1_TASK_SHELL_H
#define INC_1_TASK_SHELL_H

#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <fcntl.h>
#include <sys/wait.h>
#include <fstream>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dirent.h>
#include <regex>
#include <signal.h>

using namespace std;

class shell {
private:
    // Парсинг
    string current_directory_name; // Текущее имя директории
    string current_directory_path; // Текущее путь к директории
    stringstream input_request_stream; // Поток для обработки запроса
    string input_request_string;

    // Флаги
    bool root_flag; // 0 - привелегированный пользователь

    // Имя файла для перенаправления вывода
    string redirect_output_path;
    string redirect_input_path;

    enum code_command {
        _CD_COMMAND_ = 0,
        _PWD_COMMAND_ = 1,
        _TIME_COMMAND_ = 2,
        _ECHO_COMMAND_ = 3,
        _SET_COMMAND_ = 4
    };
    // Словарь для определения внутренних команд
    map<string, code_command> internal_commands = {{"cd", _CD_COMMAND_},
                                           {"pwd", _PWD_COMMAND_},
                                           {"time", _TIME_COMMAND_},
                                           {"echo", _ECHO_COMMAND_},
                                           {"set", _SET_COMMAND_}};
    // Не обрабатывает то, что внутренние функции не могут быть на первом месте
    void input_analyzer();
    // Получение имени текущей директории
    void get_current_directory_name();
    void run_conveyor(vector<vector<string>>& conveyor, int num_of_conveyor_requests, bool input, bool output);
    void shell_cd(const string& path);
    void shell_pwd() const;
    void shell_echo(const vector<string>& print) const;
    void shell_set(const vector<string>& request);
    void shell_time(const vector<char *>& request);
    bool template_search(const string& path_template, vector<vector<string>>& requests);
public:
    explicit shell (){
        // Получение информации о привилегиях пользователя
        root_flag = getuid();

        get_current_directory_name();
        get_request();
    }
    void get_request();

    string make_template(const string& str);
    vector<string> make_queue(const string& path, const string& templ);
    vector<string> make_paths(vector<string>& paths, const string& str);
};

#endif //INC_1_TASK_SHELL_H