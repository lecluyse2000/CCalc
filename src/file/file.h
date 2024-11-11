// Author: Caden LeCluyse

#ifndef FILE_H
#define FILE_H

#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace File {

void output_history(const std::vector<std::pair<std::string, std::string> >& history, 
                    std::ofstream& output_file) noexcept;
void initiate_file_mode() noexcept;

}  // namespace File

#endif
