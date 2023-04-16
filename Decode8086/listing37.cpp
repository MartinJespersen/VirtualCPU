#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sstream>

int main(){
    std::ifstream file("listing38", std::ios::binary);

    if (!file.is_open()) {
        // Handle error if file cannot be opened
        return 1;
    }

    // Get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read bytes into a buffer
    char* buffer = new char[fileSize];
    file.read(buffer, fileSize);

    std::cout << "bits 16\n";

    std::stringstream output_stream;
    // Process bytes in the buffer
    for (int i = 0; i < fileSize; i++) {
        char buffer_char = buffer[i];
        if (i % 2 == 0){
            char op = buffer_char & 0b11111100;
            if (op ^ 0b10001000 == 0)
                output_stream << "mov ";
        }
        else{
            char buffer_prev = buffer[i-1];
            int reg_index;
            int rm_index;
            std::string command_list_16[8] = {"ax", "cx", "dx", "bx", "sp", "bp","si" ,"di"};
            std::string command_list_8[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
            std::string *command_list_ptr;
            if (buffer_prev & 0b00000001){
                command_list_ptr = command_list_16;
            }
            else{
                command_list_ptr = command_list_8;
            }

            reg_index = int((buffer_char >> 3) & 0b00000111);
            rm_index = int(buffer_char & 0b00000111);

            if (buffer_prev & 0b00000010){
                output_stream << command_list_ptr[reg_index] << ", " << command_list_ptr[rm_index] << "\n";
            }
            else{
                output_stream << command_list_ptr[rm_index] << ", " << command_list_ptr[reg_index] << "\n";
            }
        }
    }
    std::cout << output_stream.str();
    // Clean up
    delete[] buffer;
    file.close();
}
