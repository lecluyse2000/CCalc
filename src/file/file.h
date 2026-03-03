// Author: Caden LeCluyse

#ifndef FILE_H
#define FILE_H

#include <fstream>

namespace File {

void output_history(std::ofstream& output_file);
void write_history(std::ofstream& output_file);
void read_history(std::ifstream& output_file);
void initiate_file_mode();

}  // namespace File

#endif
