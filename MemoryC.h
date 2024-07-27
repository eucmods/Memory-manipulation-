#include <iostream>
#include <fstream>
#include <string>
#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <cstdlib>

typedef unsigned long long kaddr;

// Função para encontrar o PID de um processo pelo nome
pid_t find_pid(const std::string& process_name) {
    DIR* dir;
    struct dirent* entry;
    pid_t pid = -1;
    std::ifstream cmdline;
    std::string line;
    
    dir = opendir("/proc");
    if (!dir) return -1;
    
    while ((entry = readdir(dir))) {
        pid = atoi(entry->d_name);
        if (pid > 0) {
            cmdline.open("/proc/" + std::to_string(pid) + "/cmdline");
            if (cmdline) {
                getline(cmdline, line);
                if (line.find(process_name) != std::string::npos) {
                    closedir(dir);
                    return pid;
                }
                cmdline.close();
            }
        }
    }
    closedir(dir);
    return -1;
}

// Função para obter a base do módulo
kaddr get_module_base(pid_t pid, const std::string& module_name) {
    std::ifstream maps("/proc/" + std::to_string(pid) + "/maps");
    std::string line;
    kaddr base = 0;
    
    while (getline(maps, line)) {
        if (line.find(module_name) != std::string::npos) {
            size_t pos = line.find('-');
            if (pos != std::string::npos) {
                base = std::stoull(line.substr(0, pos), nullptr, 16);
                break;
            }
        }
    }
    return base;
}

// Função para escrever na memória
bool write_memory(pid_t pid, kaddr address, int value) {
    struct iovec local_iov = {&value, sizeof(value)};
    struct iovec remote_iov = {(void*)address, sizeof(value)};
    ssize_t result = syscall(SYS_process_vm_writev, pid, &local_iov, 1, &remote_iov, 1, 0);
    return result == sizeof(value);
}

int main(int argc, char *argv[]) {
    static const char* process_name = "example_process";
    pid_t pid = find_pid(process_name);
    if (pid < 0) {
        ///Pid not found!
        return 1;
    }

    

    kaddr base = get_module_base(pid, "libil2cpp.so");
    if (base == 0) {
        //Module not found!
        return 1;
    }

    kaddr MemoryX_address = base + 0x123456; // Offset do endereço alvo
    if (write_memory(pid, memoryX_address, 123456789)) {
//success 
    } else {
  //failed 
    }
    return 0;
}
