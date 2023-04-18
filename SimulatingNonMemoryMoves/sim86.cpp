#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include "utils.cpp"



int main(){
    std::ifstream file("listing43", std::ios::binary);

    if (!file.is_open()) {
        // Handle error if file cannot be opened
        return 1;
    }

    // Get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read bytes into a buffer
    uint8_t* buffer = new uint8_t[fileSize];
    file.read(reinterpret_cast<char*>(buffer), fileSize);

    std::cout << "bits 16\n";
    int buffer_offset = 0;
    
    std::stringstream output_stream;

    
    // Process bytes in the buffer
    while(buffer_offset < fileSize) {
        uint8_t buffer_0 = buffer[buffer_offset];
        uint8_t buffer_1 = buffer[buffer_offset+1];
        int reg_index = int((buffer_1 >> 3) & 0b00000111);
        int rm_index = int(buffer_1 & 0b00000111);
        int acc_index = 0;
  
        int command_length;
        std::string reg_str;
        std::stringstream rm_str;
        

        if (!((buffer_0 >> 2) ^ 0b00100010)){ //mov command
            output_stream << "mov ";
            int mod_val = buffer_1 >> 6;
            uint8_t rm_val = (buffer_1 & 0b00000111);

            std::string *reg_list_ptr = get_reg_list_ptr(buffer_0);

            
            std::string output_rm_str;
            std::tie(output_rm_str, command_length) = command__length_and_set_rm_str(mod_val, rm_val, rm_str, buffer, buffer_offset, reg_list_ptr, rm_index);
            
            std::string output_reg_str = reg_list_ptr[reg_index];
            if (buffer_0 & 0b00000010){
                output_stream << output_reg_str << ", " << output_rm_str << "\n";
            }
            else{
                output_stream << output_rm_str << ", " << output_reg_str << "\n";
            }
        }
        else if (!((buffer_0 >> 4) ^ 0b00001011)){
            output_stream << "mov ";
            int reg_index = int(buffer_0 & 0b00000111);
            int register_val;
            int buffer_1 = buffer[buffer_offset + 1];
            int buffer_2 = buffer[buffer_offset + 2] ;
            
            if (buffer_0 & 0b00001000){
                register_val = (buffer_2 << 8) | buffer_1;
                command_length = 3;
                output_stream << command_list_16[reg_index] << ", " << register_val << "\n";
            }
            else{
                register_val = buffer_1;
                command_length = 2;
                output_stream << command_list_8[reg_index] << ", " << register_val << "\n";
            }
            
        }
        else if ((buffer_0 >> 2) == 0b100000){
            int mod_val = buffer_1 >> 6;
            uint8_t rm_val = (buffer_1 & 0b00000111);

            std::string *reg_list_ptr = get_reg_list_ptr(buffer_0);
            std::string output_rm_str;
            std::tie(output_rm_str, command_length) = command__length_and_set_rm_str(mod_val, rm_val, rm_str, buffer, buffer_offset, reg_list_ptr, rm_index);
            int immediate_val;
            int data_ext;
            if ((buffer_0 & 0b11) == 1){
                data_ext=2;
                immediate_val = (buffer[buffer_offset+command_length+1] << 8) | buffer[buffer_offset+command_length];
            }
            else{
                immediate_val = buffer[buffer_offset+command_length];
                data_ext=1;
            }


            std::string op_str = get_arithmetic_op(buffer_1);
            command_length = command_length + data_ext;

            std::string op_str_ext = mod_val != 3 ? buffer_0 & 0b00000001 ? "word": "byte": "";
            output_stream << op_str << " " << op_str_ext << " " << output_rm_str << ", " << immediate_val << "\n";
        }
        else if ((buffer_0 & 0b11000110) == 4){
            int immediate_val;
            
            std::string op_str;
            std::string mem_loc_str;

            if(buffer_0 & 1){
                immediate_val = (buffer[buffer_offset + 2] << 8) | buffer[buffer_offset + 1];
                mem_loc_str = command_list_16[acc_index];
                command_length = 3;
            }
            else{
                immediate_val = buffer[buffer_offset + 1];
                mem_loc_str = command_list_8[acc_index];
                command_length = 2;
            }

            op_str = get_arithmetic_op(buffer_0);

            output_stream << op_str << " " << mem_loc_str << ", " << immediate_val << "\n";
        }
        else if (!(buffer_0 & 0b11000100)){
            int mod_val = buffer_1 >> 6;
            uint8_t rm_val = (buffer_1 & 0b00000111);

            std::string *reg_list_ptr = get_reg_list_ptr(buffer_0);
            std::string output_rm_str;
            std::tie(output_rm_str, command_length) = command__length_and_set_rm_str(mod_val, rm_val, rm_str, buffer, buffer_offset, reg_list_ptr, rm_index);
            int immediate_val;
            int data_ext;

            std::string op_str = get_arithmetic_op(buffer_0);
            std::string output_reg_str = reg_list_ptr[reg_index];

            if (buffer_0 & 0b10){
                output_stream << op_str << " " << output_reg_str << ", " << output_rm_str << "\n";
            }
            else{
                output_stream << op_str << " " << output_rm_str << ", " << output_reg_str << "\n";
            }
        }
        else{
            std::cout << output_stream.str();
            throw std::runtime_error("Not implemented operation");
        }
        buffer_offset += command_length;
    }
    std::cout << output_stream.str();
    // Clean up
    delete[] buffer;
    file.close();
}
