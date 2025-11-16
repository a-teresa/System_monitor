#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
 #include <fstream>
#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpu_values;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    string line;
    std::getline(filestream, line);
    std::istringstream linestream(line);

    string cpu_label;
    linestream >> cpu_label;  

    string value;
    while (linestream >> value) {
      cpu_values.push_back(value);
    }
  }
  return cpu_values;
}
// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std:: ifstream stream(kProcDirectory + kMeminfoFilename);
  if(!stream.is_open())return 0.0f;
  string k;
  long value =0;
    long total_m = 0, free_m = 0, buf = 0, cach = 0, srecl = 0, sh_m = 0;

    string line;
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> k >> value;
      if (k == "MemTotal:") total_m = value;
      else if (k == "MemFree:") free_m = value;
      else if (k == "Buffers:") buf = value;
      else if (k == "Cached:") cach = value;
      else if (k == "SReclaimable:") srecl = value;
      else if (k == "Shmem:") sh_m = value;
    }

  if (total_m == 0) return 0.0f;
  long used = total_m - free_m - buf - cach - srecl + sh_m;
  return static_cast<float>(used) / static_cast<float>(total_m);
 }

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (!stream.is_open()) return 0;
  double up = 0.0, idle = 0.0;
  stream >> up >> idle;
  return static_cast<long>(up);
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  auto cpu = CpuUtilization();
  long sum = 0;
  for (const auto& f : cpu) {
    if (!f.empty()) sum += stol(f);
  }
  return sum;
}
// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (!stream.is_open()) return 0;
  string line;
  std::getline(stream, line);
  std::istringstream linestream(line);
  // /proc/[pid]/stat has many fields; we need 14-17
  // Consume tokens up to 17.
  string token;
  int idx = 0;
  long utime = 0, stime = 0, cutime = 0, cstime = 0;
  while (linestream >> token) {
    ++idx;
    if (idx == 14) utime = stol(token);
    else if (idx == 15) stime = stol(token);
    else if (idx == 16) cutime = stol(token);
    else if (idx == 17) { cstime = stol(token); break; }
  }
  return utime + stime + cutime + cstime;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto cpu = CpuUtilization();
  if (cpu.size() < 8) return 0;

  long user = stol(cpu[CPUStates::kUser_]);
  long nice = stol(cpu[CPUStates::kNice_]);
  long system = stol(cpu[CPUStates::kSystem_]);
  long irq = stol(cpu[CPUStates::kIRQ_]);
  long softirq = stol(cpu[CPUStates::kSoftIRQ_]);
  long steal = stol(cpu[CPUStates::kSteal_]);
  // guest fields are already included in user/nice in Linux accounting (we ignore here)
  return user + nice + system + irq + softirq + steal;
}



// TODO: Read and return CPU utilization
long LinuxParser::IdleJiffies() {
  auto cpu = CpuUtilization();
  if (cpu.size() < 5) return 0;

  long idle = stol(cpu[CPUStates::kIdle_]);
  long iowait = stol(cpu[CPUStates::kIOwait_]);
  return idle + iowait;
}


// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (!stream.is_open()) return 0;
  string key;
  int value = 0;
  string line;
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    if (linestream >> key >> value) {
      if (key == "processes") return value;
    }
  }
  return 0;
}


int LinuxParser::RunningProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (!stream.is_open()) return 0;
  string key;
  int value = 0;
  string line;
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    if (linestream >> key >> value) {
      if (key == "procs_running") return value;
    }
  }
  return 0;
}

// Command associated with a process: /proc/[pid]/cmdline (NUL-separated)
string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (!stream.is_open()) return {};
  string cmd;
  std::getline(stream, cmd, '\0');  // read the whole NUL-terminated string
  return cmd;
}

// Memory used by a process (kB). Use VmSize from /proc/[pid]/status
string LinuxParser::Ram(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (!stream.is_open()) return "0";
  string line, key;
  long value = 0;
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    if (linestream >> key >> value) {
      if (key == "VmSize:") return to_string(value);  // value already in kB
    }
  }
  return "0";
}

// UID of a process from /proc/[pid]/status (key "Uid:")
string LinuxParser::Uid(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (!stream.is_open()) return {};
  string line, key, uid;
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    if (linestream >> key >> uid) {
      if (key == "Uid:") return uid;
    }
  }
  return {};
}

// Username matching UID in /etc/passwd (format: name:x:uid:...)
string LinuxParser::User(int pid) {
  const string uid = Uid(pid);
  if (uid.empty()) return {};
  std::ifstream stream(kPasswordPath);
  if (!stream.is_open()) return {};
  string line;
  while (std::getline(stream, line)) {
    std::replace(line.begin(), line.end(), ':', ' ');
    std::istringstream linestream(line);
    string name, x, id;
    if (linestream >> name >> x >> id) {
      if (id == uid) return name;
    }
  }
  return {};
}

// Uptime of a process: system_uptime - (starttime / Hertz)
// starttime is field 22 in /proc/[pid]/stat
long LinuxParser::UpTime(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (!stream.is_open()) return 0;

  string line, token;
  std::getline(stream, line);
  std::istringstream linestream(line);

  long starttime_ticks = 0;
  for (int i = 1; i <= 22; ++i) {
    linestream >> token;
    if (i == 22) starttime_ticks = stol(token);
  }

  const long hertz = sysconf(_SC_CLK_TCK);
  long starttime_seconds = starttime_ticks / hertz;
  long system_uptime = UpTime();
  if (system_uptime < starttime_seconds) return 0;
  return system_uptime - starttime_seconds;
}