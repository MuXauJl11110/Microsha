#include "shell.h"

void shell::get_current_directory_name() {
    char cwd[256];

    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        perror("getcwd() error");
    } else {
        current_directory_path = current_directory_name = cwd;
    }
    if (root_flag) {
        current_directory_name += ">";
    } else {
        current_directory_name += "!";
    }
}

void shell::get_request() {
    cout << current_directory_name;
    while (getline(cin, input_request_string, '\n')) {
        // Очистка потока для дальнейшего использования
        input_request_stream.str("");
        input_request_stream.clear();
        input_request_stream << input_request_string;
        input_analyzer();
        cout << current_directory_name;
    }
    cout << endl;
}
void shell::input_analyzer() {
    string text, word;
    vector<string> words;
    set<string> quote_args;
    stringstream tmp_stream;

    getline(input_request_stream, text, '"');
    // В входном запросе нет двойных кавычек
    if (input_request_stream.eof()) {
        tmp_stream.str("");
        tmp_stream.clear();
        tmp_stream << text;
        while (tmp_stream >> word) {
            words.push_back(word);
        }
    } else {
        do {
            if (input_request_stream.eof()) {
                tmp_stream.str("");
                tmp_stream.clear();
                tmp_stream << text;
                while (tmp_stream >> word) {
                    words.push_back(word);
                }
                break;
            }
            tmp_stream.str("");
            tmp_stream.clear();
            tmp_stream << text;
            while (tmp_stream >> word) {
                words.push_back(word);
            }
            getline(input_request_stream, text, '"');
            // Если в потоке не осталось хотя бы символа перевода строки
            if (input_request_stream.eof()) {
                cout << "Wrong request! You have forgotten to close a quote!" << endl;
                return;
            }
            quote_args.insert(text);
            words.push_back(text);
            word.erase();
        } while (getline(input_request_stream, text, '"'));
    }
    vector<vector<string>> conveyor_requests(1, vector<string>());
    int num_of_conveyor_requests = 1, num_of_redirect_output_in_conveyor = 1;
    bool redirect_output = false;
    bool redirect_input = false;

    for (int i = 0; i < words.size(); i++) {
        // Если слово было в двойных кавычках, и должно быть передано как единый аргумент
        if (quote_args.find(words[i]) != quote_args.end()) {
            conveyor_requests[num_of_conveyor_requests - 1].push_back(words[i]);
        } else if (internal_commands.find(words[i]) == internal_commands.find("cd")) {
            if (i != 0) {
                cout << "Command 'cd' should be on the first place of request!" << endl;
                return;
            }
            i++;
            // Если за командой cd ничего не следует
            if (i == words.size()) {
                if (conveyor_requests.size() != 1) {
                    cout << "Command 'cd' should be single in request." << endl;
                    return;
                } else {
                    shell_cd("HOME"); // От домашней директории
                    return;
                }
            }
            if (((words[i][0] != '/') && (words[i][0] != '.')) || (i + 1 != words.size())) {
                cout << "Command cd have only one argument and it must be a path!" << endl;
                return;
            }
            shell_cd(words[i]);
            return;
        }   else if (internal_commands.find(words[i]) == internal_commands.find("set")) {
            if (i != 0) {
                cout << "Command 'set' should be on the first place of request!" << endl;
                return;
            }
            if (i + 1 == words.size()) { // Если после set нет аргументов
                cout << "You\'ve forgotten to specify argument!" << endl;
                return;
            } else {
                i++;
                while (i < words.size()) {
                    if (words[i].find('=') == string::npos) {
                        cout << "Wrong format of command 'set'" << endl;
                        return;
                    }
                    conveyor_requests[num_of_conveyor_requests - 1].push_back(words[i]);
                    i++;
                }
                shell_set(conveyor_requests[0]);
                return;
            }
        } else {
            switch(words[i][0]) {
                case '>': // redirection output
                    if (words[i].size() != 1) {
                        cout << "Error in redirection!" << endl;
                        return;
                    }
                    i++;
                    if (redirect_output) {
                        cout << "You can't use more than 1 redirection output!" << endl;
                        return;
                    } else if (i < words.size()) {
                        redirect_output_path = words[i];
                        redirect_output = true;
                        num_of_redirect_output_in_conveyor = num_of_conveyor_requests;
                    } else {
                        cout << "Wrong request! You should specify redirection file!" << endl;
                        return;
                    }
                    break;
                case '<': // redirection input
                    i++;
                    if (redirect_input) {
                        cout << "You can't use more than 1 redirection input!" << endl;
                        return;
                    } else if (num_of_conveyor_requests != 1) {
                        cout << "It\'s is forbidden to use redirect input not in first request of conveyor!" << endl;
                        return;
                    } else if (i < words.size()) {
                        redirect_input_path = words[i];
                        redirect_input = true;
                    } else {
                        cout << "Wrong request! You should specify redirection file!" << endl;
                        return;
                    }
                    break;
                case '|': // Конвейер
                    if (words[i].size() != 1) {
                        cout << "Error in conveyor!" << endl;
                        return;
                    }
                    num_of_conveyor_requests++;
                    if (redirect_output && (num_of_redirect_output_in_conveyor != num_of_conveyor_requests)) {
                        cout << "It\'s is forbidden to use redirect output not in last request of conveyor!" << endl;
                        return;
                    }
                    conveyor_requests.emplace_back(vector<string>());
                    break;
                case '/':
                    if (template_search(words[i], conveyor_requests)) {
                        cout << "Wrong request! Not such file or directory" << endl;
                        return;
                    }
                    break;
                case '*':
                    if (template_search(words[i], conveyor_requests)) {
                        cout << "Wrong request! Not such file or directory" << endl;
                        return;
                    }
                    break;
                case '.':
                    if (template_search(words[i], conveyor_requests)) {
                        cout << "Wrong request! Not such file or directory" << endl;
                        return;
                    }
                    break;
                default:
                    conveyor_requests[num_of_conveyor_requests - 1].push_back(words[i]);
                    break;
            }
        }
    }
    run_conveyor(conveyor_requests, num_of_conveyor_requests, redirect_input, redirect_output);
}
void shell::run_conveyor(vector<vector<string>>& conveyor, int num_of_conveyor_requests, bool input, bool output) {
    int fd_output = -1, fd_input = -1;

    if (output) { // Если перенаправление вывода (>)
        if ((fd_output = open(redirect_output_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666)) < 0) {
            //Если файл открыть не удалось
            cout << "Can\'t open file" << endl;
            return;
        }
    }
    if (input) { // Если перенаправление ввода (<)
        if ((fd_input = open(redirect_input_path.c_str(), O_RDONLY, 0666)) < 0) {
            //Если файл открыть не удалось
            cout << "Can\'t open file" << endl;
            return;
        }
    }

    vector<vector<char *>>conveyor_requests(num_of_conveyor_requests, vector<char *>());

    for (int i = 0; i < num_of_conveyor_requests; i++) {
        for (int j = 0; j < conveyor[i].size(); j++) {
            conveyor_requests[i].push_back((char *)conveyor[i][j].c_str());
        }
        conveyor_requests[i].push_back(nullptr);
    }

    int tmp_fd[2];
    vector<vector<int>> pipes_fd;
    pid_t children_pid = -1;

    // Создание пайпов
    for (int i = 0; i < num_of_conveyor_requests - 1; i++) {
        if (pipe(tmp_fd) == -1) {
            perror("pipe");
        } else {
            pipes_fd.emplace_back(vector<int>(2));
            pipes_fd[i][0] = tmp_fd[0];
            pipes_fd[i][1] = tmp_fd[1];
        }
    }
    for (int i = 0; i < num_of_conveyor_requests; i++) {
        children_pid = fork();

        // Если не удалось создать дочерний процесс
        if (children_pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (children_pid == 0) { // Процесс-ребенок
            if (input && (i == 0)) { // (<)
                close(STDIN_FILENO);
                dup2(fd_input, STDIN_FILENO);
            }
            if (output && (i == num_of_conveyor_requests - 1)) { // (>)
                close(STDOUT_FILENO);
                dup2(fd_output, STDOUT_FILENO);
            }
            if (i != num_of_conveyor_requests - 1) {
                close(STDOUT_FILENO);
                dup2(pipes_fd[i][1], STDOUT_FILENO);
            }
            for (int j = 0; j < num_of_conveyor_requests - 1; j++) {
                close(pipes_fd[j][1]);
            }
            if (i != 0) {
                close(STDIN_FILENO);
                dup2(pipes_fd[i - 1][0], STDIN_FILENO);
            }
            for (int j = 0; j < num_of_conveyor_requests - 1; j++) {
                close(pipes_fd[j][0]);
            }
            if (internal_commands.find(conveyor_requests[i][0]) == internal_commands.end()) {
                execvp(conveyor_requests[i][0], &conveyor_requests[i][0]);
                perror("execvp");
            } else {
                switch(internal_commands.find(conveyor_requests[i][0])->second) {
                    case _PWD_COMMAND_ : // pwd
                        shell_pwd();
                        break;
                    case _TIME_COMMAND_ : // time
                        shell_time(conveyor_requests[i]);
                        break;
                    case _ECHO_COMMAND_ : // echo
                        shell_echo(conveyor[i]);
                        break;
                    default :
                        break;
                }
                exit(0);
            }
        }
    }
    for (int i = 0; i < num_of_conveyor_requests - 1; i++) {
        close(pipes_fd[i][0]);
        close(pipes_fd[i][1]);
    }
    waitpid(children_pid, nullptr, 0);
}
void shell::shell_cd(const string& path) {
    int return_value;

    if (path == "HOME") {
        return_value = chdir(getenv("HOME"));
    } else {
        return_value = chdir(path.c_str());
    }
    if (return_value == -1) {
        perror("shell_cd() error");
        return;
    }
    get_current_directory_name();
}

void shell::shell_pwd() const {
    char cwd[256];

    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        perror("getcwd() error");
    } else {
        cout << cwd << endl;
    }
}

void shell::shell_echo(const vector<string>& request) const {
    if (!request.empty()) {
        int i;
        for (i = 1; i < request.size() - 1; i++) {
            // Переменная окружения
            if (request[i][0] == '$') {
                char *value;
                if ((value = getenv(request[i].substr(1, request[i].size() - 1).c_str())) == nullptr) {
                    cout << request[i] << " ";
                } else {
                    cout << value << " ";
                }
            } else {
                cout << request[i] << " ";
            }
        }
        if (request[i][0] == '$') {
            char *value;
            if ((value = getenv(request[i].substr(1, request[i].size() - 1).c_str())) == nullptr) {
                cout << request[i];
            } else {
                cout << value;
            }
        } else {
            cout << request[i];
        }
        cout << endl;
    } else {
        cout << "" << endl;
    }
}
void shell::shell_set(const vector<string>& request) {
    // Подразумевается, что вход уже корректный
    for (int i = 0; i < request.size(); i++) {
        string::size_type position;
        position = request[i].find('=');
        if (setenv(request[i].substr(0, position).c_str(), request[i].substr(position + 1, request[i].size() - 1).c_str(), 1) == -1) {
            perror("Setenv in function shell_set");
        }
    }
}
void shell::shell_time(const vector<char *>& request){
    pid_t children_pid = -1;
    clock_t start = clock();
    children_pid = fork();

    // Если не удалось создать дочерний процесс
    if (children_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (children_pid == 0) { // Процесс-ребенок
        execvp(request[1], &request[1]);
        perror("execvp");
    }
    waitpid(children_pid, nullptr, 0);
    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    struct rusage resource;
    if (getrusage(RUSAGE_SELF, &resource) == -1){
        perror("getrusage");
        return;
    }
    cout << "The walltime: " << seconds << " seconds" << endl;
    cout << "The sys/time: " << (double)resource.ru_stime.tv_sec + resource.ru_stime.tv_usec / 1000000.0 << endl;
    cout << "The user/time: " << (double)resource.ru_utime.tv_sec + resource.ru_utime.tv_usec / 1000000.0 << endl;
}
vector<string> shell::make_queue(const string& path, const string& templ) {
    const regex regexp(templ);
    vector<string> file_paths;
    string new_path;

    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        return vector<string>();
    }
    for (auto rd = readdir(dir); rd != nullptr; rd = readdir(dir)) {
        auto m = cmatch{};
        if (regex_match(rd->d_name, m, regexp)) {
            switch (rd->d_type) {
                case DT_REG: // file
                    //cout << "File:" << rd->d_name << endl;
                    if (path != "/") {
                        new_path = path + "/" + rd->d_name;
                    } else {
                        new_path = path + rd->d_name;
                    }
                    //cout << new_path;
                    file_paths.push_back(new_path);
                    break;
                case DT_DIR: // directory
                    //cout << "Directory:" << rd->d_name << endl;
                    if ((strcmp(rd->d_name, ".") != 0) && (strcmp(rd->d_name, "..") != 0) && (rd->d_name[0] != '.')) {
                        if (path != "/") {
                            new_path = path + "/" + rd->d_name;
                        } else {
                            new_path = path + rd->d_name;
                        }
                        file_paths.push_back(new_path);
                    }
                    break;
                default :
                    break;
            }
        }
    }
    closedir(dir);
    return file_paths;
}
string shell::make_template(const string& str) {
    string new_str;

    for (int i = 0; i < str.size(); i++){
        switch (str[i]) {
            case '*':
                new_str.push_back('.');
                new_str.push_back('*');
                break;
            case '.':
                new_str.push_back('\\');
                new_str.push_back('.');
                break;
            case '?':
                new_str.push_back('.');
                new_str.push_back('?');
                break;
            default:
                new_str.push_back(str[i]);
                break;
        }
    }
    return new_str;
}
vector<string> shell::make_paths(vector<string>& paths, const string& str) {
    vector<string> new_paths;

    for (auto & i : paths) {
        vector<string> tmp = make_queue(i, str);
        for (auto &j : tmp) {
            new_paths.push_back(j);
        }
    }
    return new_paths;
}
bool shell::template_search(const string& path_template, vector<vector<string>>& requests) {
    vector<string> file_paths;

    string temp, current_path;
    bool make_templ = false;

    for (int i = 0; i < path_template.size(); i++){
        switch (path_template[i]) {
            case '/': // Обнуление шаблона
                if (file_paths.empty()) { // Первый в запросе слэш
                    if (!temp.empty()) { // Если перед ним стоял не пустая часть пути
                        file_paths.emplace_back(current_directory_path);
                        if (make_templ) {
                            file_paths = make_paths(file_paths, make_template(temp));
                        } else {
                            file_paths = make_paths(file_paths, temp);
                        }
                        temp.clear();
                    } else {
                        file_paths.emplace_back("/");
                    }
                } else {
                    if (make_templ) {
                        file_paths = make_paths(file_paths, make_template(temp));
                    } else {
                        for (int j = 0; j < file_paths.size(); j++) {
                            file_paths[j].push_back('/');
                            file_paths[j] += temp;
                        }
                    }
                    temp.clear();
                }
                make_templ = false;
                break;
            case '*':
                temp.push_back(path_template[i]);
                make_templ = true;
                break;
            case '?':
                temp.push_back(path_template[i]);
                make_templ = true;
                break;
            default:
                temp.push_back(path_template[i]);
                break;
        }
    }
    if (!temp.empty()) {
        if (make_templ) {
            if (file_paths.empty()) {
                file_paths.emplace_back(current_directory_path);
            }
            file_paths = make_paths(file_paths, make_template(temp));
        } else {
            if (file_paths.empty()) {
                file_paths.emplace_back(current_directory_path);
            }
            file_paths = make_paths(file_paths, temp);
        }
    }
    if (file_paths.empty()) {
        return true;
    }
    for (auto& i : file_paths) {
        requests[requests.size() - 1].push_back(i);
    }
    return false;
}