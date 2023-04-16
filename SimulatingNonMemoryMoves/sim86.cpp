#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <bitset>

std::string command_list_16[8] = {"ax", "cx", "dx", "bx", "sp", "bp","si" ,"di"};
std::string command_list_8[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
std::string command_list_mod[8] = {"bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"};

std::tuple<std::string, int> command__length_and_set_rm_str(char mod_val, uint8_t rm_val, std::stringstream& rm_str, uint8_t* buffer, int buffer_offset, std::string* reg_list_ptr, int rm_index){
    int command_length;
    if (mod_val == 0){
                if(rm_val == 6)
                {
                    int register_val = (buffer[buffer_offset + 3] << 8) | buffer[buffer_offset + 2];
                    rm_str << "[" << register_val << "]";
                    command_length = 4;
                    
                }
                else{
                    rm_str << "[" << command_list_mod[rm_val] << "]";
                    command_length = 2;
                }
            }
            else if (mod_val == 1){
                int register_val = buffer[buffer_offset + 2];
                if(register_val == 0){
                    rm_str << 
                    "[" << 
                    command_list_mod[rm_val] << 
                    "]";
                }
                else{
                    rm_str << 
                    "[" << 
                    command_list_mod[rm_val] << 
                    " + " << 
                    register_val << 
                    "]";
                }
                
                command_length = 3;

            }
            else if (mod_val == 2){
                    
                    int register_val = (buffer[buffer_offset + 3] << 8) | buffer[buffer_offset + 2];
                    if(register_val == 0){
                        rm_str << 
                        "[" << 
                        command_list_mod[rm_val] << 
                        "]";
                    }
                    else{
                        rm_str << 
                        "[" << 
                        command_list_mod[rm_val] << 
                        " + " << 
                        register_val << 
                        "]";
                    }
                    
                    command_length = 4;
            }
            else if (mod_val == 3){
                rm_str << reg_list_ptr[rm_index];
                command_length = 2;
                
            }
            return std::make_tuple(rm_str.str(), command_length);
}

std::string* get_reg_list_ptr(uint8_t buffer_elem){
    std::string *reg_list_ptr;
    if(buffer_elem & 0b00000001)
        reg_list_ptr = command_list_16;
    else
        reg_list_ptr = command_list_8;

    return reg_list_ptr;
}

std::string get_arithmetic_op(uint8_t buffer_elem){
    uint8_t op = (buffer_elem >> 3) & 0b111;
    std::string op_str;
    switch(op)
    {
        case 0b000:
            op_str = "ADD";
            break;
        case 0b101:
            op_str = "SUB";
            break;
        case 0b111:
            op_str = "CMP";
            break;
        default:
            throw std::runtime_error("No arithmetic operation for this");
    }
    return op_str;
}

int main(){
    std::ifstream file("listing", std::ios::binary);

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
