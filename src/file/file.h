// Author: Caden LeCluyse

#ifndef FILE_H
#define FILE_H

#include <fstream>
#include <readline/history.h>
#include <string>
#include <unordered_map>

namespace File {

void output_history(std::ofstream& output_file);
void write_history(std::ofstream& output_file, const std::unordered_map<HIST_ENTRY*, std::string>& history);
std::unordered_map<HIST_ENTRY*, std::string> read_history(std::ifstream& input_file);
void initiate_file_mode();

}  // namespace File

#endif
