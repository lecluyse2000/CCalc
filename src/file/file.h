// Author: Caden LeCluyse

#ifndef FILE_H
#define FILE_H

#include <fstream>
#include <span>
#include <string>
#include <utility>

namespace File {

void output_history(const std::span<const std::pair<std::string, std::string> > history, 
                    std::ofstream& output_file);
void initiate_file_mode();

}  // namespace File

#endif
