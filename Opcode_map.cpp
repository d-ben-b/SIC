#include "Opcode_map.h"

std::string get_opcode(std::string str)
{
    std::map<std::string, std::string> opcode;
    opcode.insert(std::pair<std::string, std::string>("LDA", "00"));
    opcode.insert(std::pair<std::string, std::string>("AND", "40"));
    opcode.insert(std::pair<std::string, std::string>("DIV", "24"));
    opcode.insert(std::pair<std::string, std::string>("SUB", "1C"));
    opcode.insert(std::pair<std::string, std::string>("ADD", "18"));
    opcode.insert(std::pair<std::string, std::string>("LDL", "08"));
    opcode.insert(std::pair<std::string, std::string>("RD", "D8"));
    opcode.insert(std::pair<std::string, std::string>("WD", "DC"));
    opcode.insert(std::pair<std::string, std::string>("LDCH", "50"));
    opcode.insert(std::pair<std::string, std::string>("STX", "10"));
    opcode.insert(std::pair<std::string, std::string>("JLT", "38"));
    opcode.insert(std::pair<std::string, std::string>("TIX", "2C"));
    opcode.insert(std::pair<std::string, std::string>("TD", "E0"));
    opcode.insert(std::pair<std::string, std::string>("STCH", "54"));
    opcode.insert(std::pair<std::string, std::string>("STL", "14"));
    opcode.insert(std::pair<std::string, std::string>("LDX", "04"));
    opcode.insert(std::pair<std::string, std::string>("RSUB", "4C"));
    opcode.insert(std::pair<std::string, std::string>("STA", "0C"));
    opcode.insert(std::pair<std::string, std::string>("J", "3C"));
    opcode.insert(std::pair<std::string, std::string>("JEQ", "30"));
    opcode.insert(std::pair<std::string, std::string>("COMP", "26"));
    opcode.insert(std::pair<std::string, std::string>("JSUB", "48"));
    opcode.insert(std::pair<std::string, std::string>("JGT", "34"));
    opcode.insert(std::pair<std::string, std::string>("MUL", "20"));
    opcode.insert(std::pair<std::string, std::string>("OR", "44"));
    opcode.insert(std::pair<std::string, std::string>("STSW", "E8"));

    if (opcode.find(str) == opcode.end())
        return "-1";
    return opcode[str];
}
