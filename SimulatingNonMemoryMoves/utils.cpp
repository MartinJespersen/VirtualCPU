#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <tuple>

std::string command_list_16[8] = {"ax", "cx", "dx", "bx", "sp", "bp","si" ,"di"};
std::string command_list_8[8] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
std::string command_list_mod[8] = {"bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"};

enum op_type {MOV, ADD, SUB, CMP};

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