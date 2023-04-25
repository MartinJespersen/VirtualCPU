#include <iostream>
#include <fstream>
#include "sim86_shared.h"
#pragma comment (lib, "sim86_shared_debug.lib")

#define MSB_SET 32768

typedef struct cpu_register {
    union{
        u8 lRegister[10];
        u16 xRegister[9];
    };
} cpu_register;

typedef struct flags{
    bool S;
    bool Z;
} flags;

typedef struct sim_context{
    u16 ValOp0;
    u16 ValOp1;
    u16 ValFinal;
} sim_context;

size_t fill_buffer(std::string filename, u8*& buffer){
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        // Handle error if file cannot be opened
        return -1;
    }

    // Get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read bytes into a buffer
    buffer = new u8[fileSize];
    file.read(reinterpret_cast<char*>(buffer), fileSize);
    
    return fileSize;
}

char const *OpcodeMnemonics[] =
{
    "",

#define INST(Mnemonic, ...) #Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"
};

static char const *GetMnemonic(operation_type Op)
{
    char const *Result = "";
    if(Op < Op_Count)
    {
        Result = OpcodeMnemonics[Op];
    }
    
    return Result;
}

static char const *GetRegName(register_access Reg)
{
    char const *Names[][3] =
    {
        {"", "", ""},
        {"al", "ah", "ax"},
        {"bl", "bh", "bx"},
        {"cl", "ch", "cx"},
        {"dl", "dh", "dx"},
        {"sp", "sp", "sp"},
        {"bp", "bp", "bp"},
        {"si", "si", "si"},
        {"di", "di", "di"},
        {"es", "es", "es"},
        {"cs", "cs", "cs"},
        {"ss", "ss", "ss"},
        {"ds", "ds", "ds"},
        {"ip", "ip", "ip"},
        {"flags", "flags", "flags"}
    };
    
    char const *Result = Names[Reg.Index % ArrayCount(Names)][(Reg.Count == 2) ? 2 : Reg.Offset&1];
    return Result;
}

u32 EightBitIndexConvertion(register_access RegisterAccess){
    return RegisterAccess.Count * RegisterAccess.Index + RegisterAccess.Offset;
}

u16 GetRegisterValue(instruction_operand& operand, cpu_register& Register){
    register_access RegisterAccessOp0 = operand.Register;
    if (operand.Register.Count == 2){
        return Register.xRegister[RegisterAccessOp0.Index];
    }
    else{
        u32 index = EightBitIndexConvertion(operand.Register);
        return Register.lRegister[index];
    }
}

sim_context MovSim(instruction& Decoded, cpu_register& Register, sim_context& SimContext){
    register_access RegisterAccessOp0 = Decoded.Operands[0].Register;
    u32 RegisterIndexOp0 = Decoded.Operands[0].Register.Index;
    u32 RegisterCountOp0 = Decoded.Operands[0].Register.Count;
    u32 RegisterOffsetOp0 = Decoded.Operands[0].Register.Offset;
    u32 RegisterIndexOp1 = Decoded.Operands[1].Register.Index;
    u32 RegisterCountOp1 = Decoded.Operands[1].Register.Count;
    u32 RegisterOffsetOp1 = Decoded.Operands[1].Register.Offset;

    if(Decoded.Operands[0].Register.Count == 2){

        if (Decoded.Operands[1].Type == Operand_Immediate)
            Register.xRegister[RegisterIndexOp0] = Decoded.Operands[1].Immediate.Value;
        else if (Decoded.Operands[1].Type == Operand_Register)
            Register.xRegister[RegisterIndexOp0] = Register.xRegister[RegisterIndexOp1];
        SimContext.ValOp1 = Register.xRegister[RegisterIndexOp0];
        SimContext.ValFinal = Register.xRegister[RegisterIndexOp0];
    }
    else{
        u32 IndexLOp0 = RegisterCountOp0 * RegisterIndexOp0 + RegisterOffsetOp0;
        u32 IndexLOp1 = RegisterCountOp1 * RegisterIndexOp1 + RegisterOffsetOp1;
        
        if (Decoded.Operands[1].Type == Operand_Immediate)
            Register.lRegister[IndexLOp0] = Decoded.Operands[1].Immediate.Value;
        else if (Decoded.Operands[1].Type == Operand_Register)
            Register.lRegister[IndexLOp0] = Register.lRegister[IndexLOp1];

        SimContext.ValOp1 = Register.lRegister[IndexLOp0];
        SimContext.ValFinal = Register.lRegister[IndexLOp0];

    }

    return SimContext;
}

sim_context SubSim(instruction& Decoded, cpu_register& Register, flags& Flags, sim_context& SimContext){


    if (Decoded.Operands[1].Type == Operand_Immediate)
        SimContext.ValOp1 = Decoded.Operands[1].Immediate.Value;
    else if (Decoded.Operands[1].Type == Operand_Register)
        SimContext.ValOp1 = Register.xRegister[Decoded.Operands[1].Register.Index];
    
    SimContext.ValFinal = SimContext.ValOp0 - SimContext.ValOp1;
    Flags.S = (SimContext.ValFinal & MSB_SET) != 0;
    Flags.Z = SimContext.ValFinal == 0;
    

    return SimContext;
}

sim_context CmpSim(instruction& Decoded, cpu_register& Register, flags& Flags, sim_context& SimContext){

    if (Decoded.Operands[1].Type == Operand_Immediate)
        SimContext.ValOp1 = Decoded.Operands[1].Immediate.Value;
    else if (Decoded.Operands[1].Type == Operand_Register)
        SimContext.ValOp1 = Register.xRegister[Decoded.Operands[1].Register.Index];
    
    SimContext.ValFinal = SimContext.ValOp0;
    u16 CmpVal = SimContext.ValOp0 - SimContext.ValOp1;
    Flags.S = (CmpVal & MSB_SET) != 0;
    Flags.Z = CmpVal == 0;

    return SimContext;
}

sim_context AddSim(instruction& Decoded, cpu_register& Register, flags& Flags, sim_context& SimContext){

    if (Decoded.Operands[1].Type == Operand_Immediate)
        SimContext.ValOp1 = Decoded.Operands[1].Immediate.Value;
    else if (Decoded.Operands[1].Type == Operand_Register)
        SimContext.ValOp1 = Register.xRegister[Decoded.Operands[1].Register.Index];
    
    SimContext.ValFinal = SimContext.ValOp0 + SimContext.ValOp1;
    Flags.S = (SimContext.ValFinal & MSB_SET) != 0;
    Flags.Z = SimContext.ValFinal == 0;

    return SimContext;
}

int main(){

    u32 Version = Sim86_GetVersion();
    printf("Sim86 Version: %u (expected %u)\n", Version, SIM86_VERSION);
    if(Version != SIM86_VERSION)
    {
        printf("ERROR: Header file version doesn't match DLL.\n");
        return -1;
    }

    u8* buffer;
    size_t fileSize = fill_buffer("listing46", buffer);
    if (fileSize == -1){
        printf("File cannot be opend");
    }

    cpu_register Register = {0};
    flags Flags = {0};
    
    instruction_table Table;
    Sim86_Get8086InstructionTable(&Table);
    printf("8086 Instruction Instruction Encoding Count: %u\n", Table.EncodingCount);
    
    u32 Offset = 0;
    while(Offset < fileSize)
    {
        instruction Decoded;
        Sim86_Decode8086Instruction(fileSize - Offset, buffer + Offset, &Decoded);
        if(Decoded.Op)
        {
            Flags = {0};
            Offset += Decoded.Size;

            printf("%s ", GetMnemonic(Decoded.Op));
            printf("%s, ", GetRegName(Decoded.Operands[0].Register));
            
            if (Decoded.Operands[1].Type == Operand_Immediate)
                printf("%d; ", Decoded.Operands[1].Immediate.Value);
            else if (Decoded.Operands[1].Type == Operand_Register)
                printf("%s; ", GetRegName(Decoded.Operands[1].Register));
            printf("%s:", GetRegName(Decoded.Operands[0].Register));

            sim_context SimContext;

            SimContext.ValOp0 = GetRegisterValue(Decoded.Operands[0], Register);
            switch(Decoded.Op){
                case Op_mov:
                {
                    MovSim(Decoded, Register, SimContext);
                } break;
                case Op_sub:
                {
                    SubSim(Decoded, Register, Flags, SimContext);
                } break;
                case Op_add:
                {
                    AddSim(Decoded, Register, Flags, SimContext);
                } break;
                case Op_cmp:
                {
                    CmpSim(Decoded, Register, Flags, SimContext);
                } break;
                default:
                {
                    return 0;
                } break;
            }
            Register.xRegister[Decoded.Operands[0].Register.Index] = SimContext.ValFinal;
            printf("%x->", SimContext.ValOp0);
            printf("%x", SimContext.ValFinal);
            if(Flags.S)
                printf(" flags:->SF");
            if(Flags.Z)
                printf(" flags:->PZ");
            printf("\n");
        }
        else
        {
            printf("Unrecognized instruction\n");
            break;
        }
    }
    
    return 0;

}