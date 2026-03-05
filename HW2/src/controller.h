#include <string>

#ifndef CONTROLLER_H
#define CONTROLLER_H
unsigned char* controller_process_pass_fixed(const std::string &imagePath,
                                               int &width, int &height,
                                               int M, int Q, double B);
#endif