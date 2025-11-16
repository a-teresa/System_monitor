#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"
using std::string;
using std::to_string;
using std::vector;

// TODO: Return this process's ID
int Process::Pid() { return pid_; }
Process::Process(int pid) : pid_(pid) {} 

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() { 
    long full_amount_time = LinuxParser::ActiveJiffies(pid_);
    long secs = LinuxParser::UpTime()-LinuxParser::UpTime(pid_);
    if (secs>0){
        cpu_utilz_ = static_cast<float>(full_amount_time)/sysconf(_SC_CLK_TCK)/secs;
    }else{
        cpu_utilz_=0.0;
    }
    return cpu_utilz_;
 }

string Process::Command() { 
    return LinuxParser::Command(pid_); 
}

string Process::Ram() { 
    // Return memory in MB
    string ram_kb = LinuxParser::Ram(pid_);
    if (!ram_kb.empty()) {
        try {
            int ram_mb = std::stoi(ram_kb) / 1024;
            return to_string(ram_mb);
        } catch (...) {
            return "0";
        }
    }
    return "0";
}

string Process::User() { 
    return LinuxParser::User(pid_); 
}

long int Process::UpTime() { 
    return LinuxParser::UpTime(pid_);
}

// Sort processes by CPU utilization in descending order
bool Process::operator<(Process const& a) const { 
    return this->cpu_utilz_ > a.cpu_utilz_;
}