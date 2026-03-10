// Author: Caden LeCluyse

#ifndef FILE_H
#define FILE_H

#include <fstream>
#include <readline/history.h>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace File {

void read_history(std::vector<std::pair<std::string, std::string> >& history, std::ifstream& input_file);
void read_vars(std::unordered_map<char, std::string>&  history, std::ifstream& input_file);
void write_history(const std::span<const std::pair<std::string, std::string> > history, 
                    std::ofstream& output_file);
void output_history(const std::span<const std::pair<std::string, std::string> > history, 
                    std::ofstream& output_file);
void write_vars(const std::unordered_map<char, std::string>& vars, std::ofstream& output_file);
void initiate_file_mode();

}  // namespace File

#endif
