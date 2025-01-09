#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 常數定義
#define MAX_LINES 1000
#define MAX_SYMBOLS 1000
#define MAX_OPCODE 100
#define MAX_REGISTERS 10
#define MAX_MNEMONIC_LENGTH 20
#define MAX_LABEL_LENGTH 20
#define MAX_OPERAND_LENGTH 20
#define MAX_OBJECT_CODE_LENGTH 10

// 定義每一行的結構
typedef struct {
    char address[7];
    char label[MAX_LABEL_LENGTH];
    char mnemonic[MAX_MNEMONIC_LENGTH];
    char operand[MAX_OPERAND_LENGTH];
    char object_code[MAX_OBJECT_CODE_LENGTH];
} Line;

// 定義符號表結構
typedef struct {
    char symbol[MAX_LABEL_LENGTH];
    char address[7];
} Symbol;

// 定義助記符和操作碼的映射結構
typedef struct {
    char mnemonic[MAX_MNEMONIC_LENGTH];
    char opcode[3];
} OpcodeMap;

// 定義寄存器映射結構
typedef struct {
    char reg_name[5];
    char reg_code[2];
} RegisterMap;

// 全域變數
Line lines[MAX_LINES];
int line_count = 0;

Symbol symbol_table[MAX_SYMBOLS];
int symbol_count = 0;

OpcodeMap opcode_map[MAX_OPCODE];
int opcode_map_count = 0;

RegisterMap register_map_list[MAX_REGISTERS];
int register_map_count = 0;

// 全域變數來存儲程式開始地址和長度
int start_addr = 0;
int program_length = 0;

// 函數宣告
void initialize_opcode_map();
void initialize_register_map();
int is_directive(const char *mnemonic);
int is_mnemonic(const char *token);
int add_symbol(const char *symbol, const char *address, int line_num);
int find_symbol(const char *symbol, char *address);
int find_opcode(const char *mnemonic, char *opcode);
int find_register_code(const char *reg_name, char *reg_code);
void pass1(FILE *fp);
void pass2();
void generate_object_file(const char *obj_filename);
void generate_list_file(const char *lst_filename);
char* decToHex(int num, int width);

// 主函數
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: assembler <input_file> <output_obj> <output_lst>\n");
        return 1;
    }

    char *input_file = argv[1];
    char *output_obj = argv[2];
    char *output_lst = argv[3];

    // 初始化 Opcode 和 Register Map
    initialize_opcode_map();
    initialize_register_map();

    // 打開輸入檔案
    FILE *fp = fopen(input_file, "r");
    if (fp == NULL) {
        printf("Error: Cannot open input file %s\n", input_file);
        return 1;
    }

    // 第一遍：讀取輸入並分配地址
    pass1(fp);
    fclose(fp);
    program_length = lines[line_count - 1].address[0] ? 
                     (int)strtol(lines[line_count - 1].address, NULL, 16) + 0 - start_addr 
                     : 0;
    // 第二遍：生成物件碼
    pass2();

    // 生成物件檔案
    generate_object_file(output_obj);

    // 生成列表檔案
    generate_list_file(output_lst);

    printf("Assembly completed successfully.\n");
    return 0;
}

// 初始化 Opcode 映射
void initialize_opcode_map() {
    // 格式1、格式3、格式4指令
    strcpy(opcode_map[0].mnemonic, "LDA"); strcpy(opcode_map[0].opcode, "00");
    strcpy(opcode_map[1].mnemonic, "AND"); strcpy(opcode_map[1].opcode, "40");
    strcpy(opcode_map[2].mnemonic, "DIV"); strcpy(opcode_map[2].opcode, "24");
    strcpy(opcode_map[3].mnemonic, "SUB"); strcpy(opcode_map[3].opcode, "1C");
    strcpy(opcode_map[4].mnemonic, "ADD"); strcpy(opcode_map[4].opcode, "18");
    strcpy(opcode_map[5].mnemonic, "LDL"); strcpy(opcode_map[5].opcode, "08");
    strcpy(opcode_map[6].mnemonic, "RD");  strcpy(opcode_map[6].opcode, "D8");
    strcpy(opcode_map[7].mnemonic, "WD");  strcpy(opcode_map[7].opcode, "DC");
    strcpy(opcode_map[8].mnemonic, "LDCH"); strcpy(opcode_map[8].opcode, "50");
    strcpy(opcode_map[9].mnemonic, "STX"); strcpy(opcode_map[9].opcode, "10");
    strcpy(opcode_map[10].mnemonic, "JLT"); strcpy(opcode_map[10].opcode, "38");
    strcpy(opcode_map[11].mnemonic, "TIX"); strcpy(opcode_map[11].opcode, "2C");
    strcpy(opcode_map[12].mnemonic, "TD");  strcpy(opcode_map[12].opcode, "E0");
    strcpy(opcode_map[13].mnemonic, "STCH"); strcpy(opcode_map[13].opcode, "54");
    strcpy(opcode_map[14].mnemonic, "STL"); strcpy(opcode_map[14].opcode, "14");
    strcpy(opcode_map[15].mnemonic, "LDX"); strcpy(opcode_map[15].opcode, "04");
    strcpy(opcode_map[16].mnemonic, "RSUB"); strcpy(opcode_map[16].opcode, "4C");
    strcpy(opcode_map[17].mnemonic, "STA"); strcpy(opcode_map[17].opcode, "0C");
    strcpy(opcode_map[18].mnemonic, "J");   strcpy(opcode_map[18].opcode, "3C");
    strcpy(opcode_map[19].mnemonic, "JEQ"); strcpy(opcode_map[19].opcode, "30");
    strcpy(opcode_map[20].mnemonic, "COMP"); strcpy(opcode_map[20].opcode, "28");
    strcpy(opcode_map[21].mnemonic, "JSUB"); strcpy(opcode_map[21].opcode, "48");
    strcpy(opcode_map[22].mnemonic, "JGT"); strcpy(opcode_map[22].opcode, "34");
    strcpy(opcode_map[23].mnemonic, "MUL"); strcpy(opcode_map[23].opcode, "20");
    strcpy(opcode_map[24].mnemonic, "OR");   strcpy(opcode_map[24].opcode, "44");
    strcpy(opcode_map[25].mnemonic, "STSW"); strcpy(opcode_map[25].opcode, "E8");

    // 格式2指令
    strcpy(opcode_map[26].mnemonic, "CLEAR"); strcpy(opcode_map[26].opcode, "B4");
    strcpy(opcode_map[27].mnemonic, "TIXR");  strcpy(opcode_map[27].opcode, "B8");
    strcpy(opcode_map[28].mnemonic, "ADDR");  strcpy(opcode_map[28].opcode, "90");
    strcpy(opcode_map[29].mnemonic, "SUBR");  strcpy(opcode_map[29].opcode, "94");
    strcpy(opcode_map[30].mnemonic, "COMPR"); strcpy(opcode_map[30].opcode, "A0");
    strcpy(opcode_map[31].mnemonic, "MULR");  strcpy(opcode_map[31].opcode, "98");
    strcpy(opcode_map[32].mnemonic, "DIVR");  strcpy(opcode_map[32].opcode, "9C");
    strcpy(opcode_map[33].mnemonic, "RMO");   strcpy(opcode_map[33].opcode, "AC");
    strcpy(opcode_map[34].mnemonic, "SVC");   strcpy(opcode_map[34].opcode, "B0");
    strcpy(opcode_map[35].mnemonic, "STPR");  strcpy(opcode_map[35].opcode, "B8");

    // 新增的指令
    strcpy(opcode_map[36].mnemonic, "STB"); strcpy(opcode_map[36].opcode, "68");    
    strcpy(opcode_map[37].mnemonic, "STS"); strcpy(opcode_map[37].opcode, "78");    
    strcpy(opcode_map[38].mnemonic, "STT"); strcpy(opcode_map[38].opcode, "84");    
    strcpy(opcode_map[39].mnemonic, "SHIFTL"); strcpy(opcode_map[39].opcode, "A4"); 
    strcpy(opcode_map[40].mnemonic, "SHIFTR"); strcpy(opcode_map[40].opcode, "A6"); 

    opcode_map_count = 41;
}

// 初始化 Register 映射
void initialize_register_map() {
    strcpy(register_map_list[0].reg_name, "A");   strcpy(register_map_list[0].reg_code, "0");
    strcpy(register_map_list[1].reg_name, "X");   strcpy(register_map_list[1].reg_code, "1");
    strcpy(register_map_list[2].reg_name, "L");   strcpy(register_map_list[2].reg_code, "2");
    strcpy(register_map_list[3].reg_name, "B");   strcpy(register_map_list[3].reg_code, "3");
    strcpy(register_map_list[4].reg_name, "S");   strcpy(register_map_list[4].reg_code, "4");
    strcpy(register_map_list[5].reg_name, "T");   strcpy(register_map_list[5].reg_code, "5");
    strcpy(register_map_list[6].reg_name, "F");   strcpy(register_map_list[6].reg_code, "6");
    strcpy(register_map_list[7].reg_name, "PC");  strcpy(register_map_list[7].reg_code, "8");
    strcpy(register_map_list[8].reg_name, "SW");  strcpy(register_map_list[8].reg_code, "9");

    register_map_count = 9;
}

// 判斷是否為控制指令
int is_directive(const char *mnemonic) {
    const char *directives[] = {"START", "END", "BYTE", "WORD", "RESW", "RESB", "ORG", "EQU", "CSECT"};
    int num_directives = sizeof(directives) / sizeof(directives[0]);
    for(int i=0; i<num_directives; i++) {
        if(strcmp(mnemonic, directives[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// 判斷一個詞是否為已定義的助記符
int is_mnemonic(const char *token) {
    // 檢查是否為控制指令
    if(is_directive(token)) {
        return 1;
    }
    // 檢查是否在 Opcode 映射中
    for(int i=0; i<opcode_map_count; i++) {
        if(strcmp(opcode_map[i].mnemonic, token) == 0) {
            return 1;
        }
    }
    return 0;
}

// 添加符號到符號表
int add_symbol(const char *symbol, const char *address, int line_num) {
    // 檢查符號是否已存在
    for(int i=0; i<symbol_count; i++) {
        if(strcmp(symbol_table[i].symbol, symbol) == 0) {
            printf("Error: Duplicate symbol '%s' at line %d\n", symbol, line_num);
            return 0;
        }
    }
    // 添加符號
    strcpy(symbol_table[symbol_count].symbol, symbol);
    strcpy(symbol_table[symbol_count].address, address);
    symbol_count++;
    return 1;
}

// 查找符號的地址
int find_symbol(const char *symbol, char *address) {
    for(int i=0; i<symbol_count; i++) {
        if(strcmp(symbol_table[i].symbol, symbol) == 0) {
            strcpy(address, symbol_table[i].address);
            return 1;
        }
    }
    return 0;
}

// 查找助記符的操作碼
int find_opcode(const char *mnemonic, char *opcode) {
    for(int i=0; i<opcode_map_count; i++) {
        if(strcmp(opcode_map[i].mnemonic, mnemonic) == 0) {
            strcpy(opcode, opcode_map[i].opcode);
            return 1;
        }
    }
    return 0;
}

// 查找寄存器的編碼
int find_register_code(const char *reg_name, char *reg_code) {
    for(int i=0; i<register_map_count; i++) {
        if(strcmp(register_map_list[i].reg_name, reg_name) == 0) {
            strcpy(reg_code, register_map_list[i].reg_code);
            return 1;
        }
    }
    return 0;
}

// 將十進位整數轉換為大寫十六進位字串，至少指定寬度
char* decToHex(int num, int width) {
    static char hex_str[10];
    sprintf(hex_str, "%0*X", width, num);
    return hex_str;
}

// Pass1: 分配地址並建立符號表
void pass1(FILE *fp) {
    char line_str[100];
    int LC = 0;
    int start_found = 0;
    char program_name_temp[MAX_LABEL_LENGTH];
    char starting_address_temp[7];

    // 定義格式2助記符列表
    const char *format2_mnemonics[] = {"CLEAR", "TIXR", "ADDR", "SUBR", "COMPR", "MULR", "DIVR", "RMO", "SVC", "STPR"};
    int num_format2 = sizeof(format2_mnemonics) / sizeof(format2_mnemonics[0]);

    while(fgets(line_str, sizeof(line_str), fp)) {
        // 移除換行符
        
        line_str[strcspn(line_str, "\n")] = 0;

        // 跳過空行
        if(strlen(line_str) == 0) continue;

        // 分析行
        Line current_line;
        current_line.address[0] = '\0';
        strcpy(current_line.label, "\t");
        strcpy(current_line.mnemonic, "\t");
        strcpy(current_line.operand, "\t");
        strcpy(current_line.object_code, "\t");

        // 使用 strtok 解析行
        char *token = strtok(line_str, " \t");

        if(token == NULL) {
            // 空行或無效行
            continue;
        }

        // 判斷第一個詞是否為助記符
        if(is_mnemonic(token)) {
            // 無標籤
            strcpy(current_line.mnemonic, token);
            char *operand = strtok(NULL, " \t");
            if(operand != NULL) {
                strcpy(current_line.operand, operand);
            }
        }
        else {
            // 有標籤
            strcpy(current_line.label, token);
            char *mnemonic = strtok(NULL, " \t");
            if(mnemonic == NULL) {
                printf("Error: Missing mnemonic at line %d\n", line_count + 1);
                continue;
            }
            strcpy(current_line.mnemonic, mnemonic);
            char *operand = strtok(NULL, " \t");
            if(operand != NULL) {
                strcpy(current_line.operand, operand);
            }
        }

        // 設定地址
        current_line.address[0] = '\0';
        sprintf(current_line.address, "%04X", LC);
        lines[line_count++] = current_line;

        // 如果有標籤，加入符號表
        if(strcmp(current_line.label, "\t") != 0) {
            if(!add_symbol(current_line.label, current_line.address, line_count)) {
                // Duplicate symbol, 已在 add_symbol 中報錯
            }
        }

        // 處理 START 指令
        if(!start_found && strcmp(current_line.mnemonic, "START") == 0) {
            strcpy(program_name_temp, current_line.label);
            strcpy(starting_address_temp, current_line.operand);
            LC = (int)strtol(starting_address_temp, NULL, 16);
            start_addr = LC; // 記錄起始地址
            // 更新第一行的地址為起始地址
            sprintf(lines[0].address, "%04X", LC);
            start_found = 1;
            continue;
        }

        // 如果沒有找到 START，設定起始地址為 0000
        if(!start_found && strcmp(current_line.mnemonic, "START") != 0) {
            strcpy(starting_address_temp, "0000");
            LC = 0;
            start_addr = LC; // 設定起始地址
            start_found = 1;
        }

        // 根據助記符更新 LC
        char *mnemonic = current_line.mnemonic;
        char *operand = current_line.operand;

        if(is_directive(mnemonic)) {
            if(strcmp(mnemonic, "START") == 0 || strcmp(mnemonic, "END") == 0 || strcmp(mnemonic, "CSECT") == 0) {
                // 不影響 LC
                if(strcmp(mnemonic, "END") == 0) {
                    // END 指令後繼續處理後續的數據定義
                    // 不在此處設置 program_length
                }
                continue;
            }

            // 處理其他控制指令
            if(strcmp(mnemonic, "BYTE") == 0) {
                if(operand[0] == 'C') {
                    // C'EOF' -> 每個字符佔 1 byte
                    int len = 0;
                    for(int i=2; operand[i] != '\''; i++) len++;
                    LC += len;
                }
                else if(operand[0] == 'X') {
                    // X'F1' -> 每兩個十六進位數字佔 1 byte
                    int len = 0;
                    for(int i=2; operand[i] != '\''; i++) len++;
                    if(len %2 !=0) {
                        printf("Error: Invalid hex string in BYTE at line %d\n", line_count);
                    }
                    LC += len / 2;
                }
                continue;
            }
            else if(strcmp(mnemonic, "WORD") ==0) {
                // WORD -> 3 bytes
                LC +=3;
                continue;
            }
            else if(strcmp(mnemonic, "RESW") ==0) {
                // RESW -> 3 * operand bytes
                int reserve = atoi(operand);
                LC +=3 * reserve;
                continue;
            }
            else if(strcmp(mnemonic, "RESB") ==0) {
                // RESB -> operand bytes
                int reserve = atoi(operand);
                LC +=reserve;
                continue;
            }
            else if(strcmp(mnemonic, "ORG") ==0) {
                // ORG -> 設置 LC 為操作數的值
                char addr[7];
                if(find_symbol(operand, addr)) {
                    LC = (int)strtol(addr, NULL, 16);
                }
                else {
                    LC = (int)strtol(operand, NULL, 16);
                }
                continue;
            }
            else if(strcmp(mnemonic, "EQU") ==0) {
                // EQU -> 定義符號的值
                int value =0;
                if(isdigit(operand[0])) {
                    value = atoi(operand);
                }
                else {
                    char addr[7];
                    if(find_symbol(operand, addr)) {
                        value = (int)strtol(addr, NULL, 16);
                    }
                    else {
                        printf("Error: Undefined symbol in EQU at line %d\n", line_count);
                    }
                }
                // 更新符號表
                for(int i=0; i<symbol_count; i++) {
                    if(strcmp(symbol_table[i].symbol, current_line.label) ==0) {
                        sprintf(symbol_table[i].address, "%04X", value);
                        // Update the address in line
                        strcpy(current_line.address, symbol_table[i].address);
                        break;
                    }
                }
                continue;
            }
        }
        else {
            // 判斷指令格式
            const char *format2_mnemonics[] = {"CLEAR", "TIXR", "ADDR", "SUBR", "COMPR", "MULR", "DIVR", "RMO", "SVC", "STPR"};
            int num_format2 = sizeof(format2_mnemonics) / sizeof(format2_mnemonics[0]);
            int is_format2 = 0;
            for(int j=0; j<num_format2; j++) {
                if(strcmp(mnemonic, format2_mnemonics[j]) ==0) {
                    is_format2 =1;
                    break;
                }
            }

            if(is_format2) {
                // 格式2指令：2 bytes
                LC +=2;
                continue;
            }

            // 假設是格式3指令，長度為 3 bytes
            LC +=3;
            continue;
        }
    }
}
    // Pass2: 生成物件碼
    void pass2() {
        // 定義格式2助記符列表
        const char *format2_mnemonics[] = {"CLEAR", "TIXR", "ADDR", "SUBR", "COMPR", "MULR", "DIVR", "RMO", "SVC", "STPR"};
        int num_format2 = sizeof(format2_mnemonics) / sizeof(format2_mnemonics[0]);

        for(int i=0; i<line_count; i++) {
            Line *line = &lines[i];
            strcpy(line->object_code, "\t"); // 初始化
            if(is_directive(line->mnemonic)) {
                continue; // 不產生 object code
            }

            if(strcmp(line->mnemonic, "START") ==0 || strcmp(line->mnemonic, "END") ==0 || strcmp(line->mnemonic, "CSECT") ==0) {
                // 不生成物件碼
                if(strcmp(line->mnemonic, "END") ==0) {
                    // END 指令不生成物件碼
                }
                continue;
            }

            if(strcmp(line->mnemonic, "BYTE") ==0 || strcmp(line->mnemonic, "WORD") ==0) {
                // 處理 BYTE 和 WORD
                if(strcmp(line->mnemonic, "BYTE") ==0) {
                    char type = line->operand[0];
                    char value[50];
                    strcpy(value, line->operand +2); // 跳過 C' 或 X'
                    value[strlen(value)-1] = '\0'; // 移除結尾的 '
                    if(type == 'C') {
                        // 每個字符轉換為十六進位
                        char obj_code[100] = "";
                        for(int j=0; j<strlen(value); j++) {
                            sprintf(obj_code + strlen(obj_code), "%02X", (unsigned char)value[j]);
                        }
                        strcpy(line->object_code, obj_code);
                    }
                    else if(type == 'X') {
                        // 直接使用十六進位
                        strcpy(line->object_code, value);
                    }
                }
                else if(strcmp(line->mnemonic, "WORD") ==0) {
                    // 將操作數轉換為十六進位的3字節
                    int num = atoi(line->operand);
                    sprintf(line->object_code, "%06X", num);
                }
                continue;
            }

            // 判斷是否為格式2指令
            int is_format2 =0;
            for(int j=0; j<num_format2; j++) {
                if(strcmp(line->mnemonic, format2_mnemonics[j]) ==0) {
                    is_format2 =1;
                    break;
                }
            }

            if(is_format2) {
                // 格式2指令：opcode + reg1 + reg2
                char opcode[3];
                if(!find_opcode(line->mnemonic, opcode)) {
                    printf("Error: Undefined mnemonic '%s' at line %d\n", line->mnemonic, i+1);
                    strcpy(line->object_code, "0000");
                    continue;
                }

                char reg_code1[2], reg_code2[2];
                char operand_copy[MAX_OPERAND_LENGTH];
                strcpy(operand_copy, line->operand);
                char *reg1 = strtok(operand_copy, ",");
                char *reg2 = strtok(NULL, ",");

                if(reg2 != NULL) {
                    // 有兩個寄存器
                    if(find_register_code(reg1, reg_code1) && find_register_code(reg2, reg_code2)) {
                        sprintf(line->object_code, "%s%s%s", opcode, reg_code1, reg_code2);
                    }
                    else {
                        printf("Error: Invalid register(s) '%s, %s' in instruction at line %d\n", reg1, reg2, i+1);
                        strcpy(line->object_code, "0000");
                    }
                }
                else {
                    // 只有一個寄存器，假設 reg2 為 '0'
                    if(find_register_code(reg1, reg_code1)) {
                        sprintf(line->object_code, "%s%s0", opcode, reg_code1);
                    }
                    else {
                        printf("Error: Invalid register '%s' in instruction at line %d\n", reg1, i+1);
                        strcpy(line->object_code, "0000");
                    }
                }
                continue;
            }

            // 查找助記符的操作碼
            char opcode[3];
            if(!find_opcode(line->mnemonic, opcode)) {
                printf("Error: Undefined mnemonic '%s' at line %d\n", line->mnemonic, i+1);
                strcpy(line->object_code, "000000");
                continue;
            }

            // 處理格式3指令
            // 初始化 nix 位元
            int nix =0;
            char symbol[MAX_LABEL_LENGTH];
            strcpy(symbol, line->operand);

            int x =0; // 索引位元
            int n=1, i_bit=1; // 默認為簡單尋址

            // 處理尋址模式
            if(line->operand[0] == '#') {
                // 立即尋址
                i_bit =1;
                n=0;
                strcpy(symbol, line->operand +1);
            }
            else if(line->operand[0] == '@') {
                // 間接尋址
                i_bit =0;
                n=1;
                strcpy(symbol, line->operand +1);
            }

            // 檢查是否為索引尋址
            char *comma = strchr(symbol, ',');
            if(comma != NULL) {
                if(strcmp(comma +1, "X") ==0) {
                    x=1;
                    *comma = '\0'; // 移除 ',X'
                }
            }

            // 計算 nix 位元
            nix = (n ?4 :0) + (i_bit ?2 :0) + (x ?1 :0);

            // 獲取地址
            char addr[4] = "000";
            if(strcmp(line->mnemonic, "RSUB") !=0) {
                // 如果是立即尋址且操作數為數字
                if(line->operand[0] == '#') {
                    int imm_val = atoi(symbol);
                    if(imm_val > 0xFFF) {
                        printf("Error: Immediate value out of range at line %d\n", i+1);
                        imm_val =0;
                    }
                    sprintf(addr, "%03X", imm_val);
                }
                else {
                    // 符號尋址
                    if(find_symbol(symbol, addr)) {
                        // 確保 addr 為 3 位
                        int len = strlen(addr);
                        if(len >3) {
                            // 使用低 3 位
                            memmove(addr, addr + len -3, 3);
                            addr[3] = '\0';
                        }
                        else if(len <3) {
                            // 補零
                            char temp_addr[4] = "000";
                            strcpy(temp_addr + (3 - len), addr);
                            strcpy(addr, temp_addr);
                        }
                    }
                    else {
                        printf("Error: Undefined symbol '%s' at line %d\n", symbol, i+1);
                        strcpy(addr, "000");
                    }
                }
            }

            // 組合物件碼
            char temp_obj_code[7];
            if(strcmp(line->mnemonic, "RSUB") ==0) {
                strcpy(temp_obj_code, "4C0000");
            }
            else {
                // opcode (2) + nix (1) + addr (3) =6
                sprintf(temp_obj_code, "%s%1X%s", opcode, nix, addr);
            }
            strcpy(line->object_code, temp_obj_code);
        }
    }

// 生成物件檔案
void generate_object_file(const char *obj_filename) {
    FILE *fp = fopen(obj_filename, "w");
    if(fp == NULL) {
        printf("Error: Cannot open object file %s\n", obj_filename);
        return;
    }

    // H-Record: Header
    char program_name_trimmed[7];
    strncpy(program_name_trimmed, lines[0].label, 6);
    program_name_trimmed[6] = '\0';

    fprintf(fp, "H%-6s%06X%06X\n", program_name_trimmed, start_addr, program_length);

    // T-Records: Text
    int current_text_start = -1;
    char current_text[60] = "";
    int current_text_length =0;

    for(int i=0; i<line_count; i++) {
        if(strcmp(lines[i].object_code, "\t") !=0 && strcmp(lines[i].object_code, "000000") !=0) {
            if(current_text_start == -1) {
                current_text_start = (int)strtol(lines[i].address, NULL, 16);
            }
            if(current_text_length + strlen(lines[i].object_code)/2 >30) { // 每個T記錄最多30字節
                // 寫入當前T記錄
                fprintf(fp, "T%06X%02X%s\n", current_text_start, current_text_length, current_text);
                // 重置
                current_text_start = (int)strtol(lines[i].address, NULL, 16);
                current_text[0] = '\0';
                current_text_length =0;
            }
            strcat(current_text, lines[i].object_code);
            current_text_length += strlen(lines[i].object_code)/2;
        }
        else {
            if(current_text_length >0) {
                // 寫入當前T記錄
                fprintf(fp, "T%06X%02X%s\n", current_text_start, current_text_length, current_text);
                // 重置
                current_text_start = -1;
                current_text[0] = '\0';
                current_text_length =0;
            }
        }
    }

    // 如果有剩餘的T記錄
    if(current_text_length >0) {
        fprintf(fp, "T%06X%02X%s\n", current_text_start, current_text_length, current_text);
    }

    // E-Record: End
    fprintf(fp, "E%06X\n", start_addr);

    fclose(fp);
}

// 生成列表檔案
void generate_list_file(const char *lst_filename) {
    FILE *fp = fopen(lst_filename, "w");
    if(fp == NULL) {
        printf("Error: Cannot open list file %s\n", lst_filename);
        return;
    }

    // 標題
    fprintf(fp, "Address\tLabel\tMnemonic\tOperand\tObject Code\n");

    // 每一行的詳細信息
    for(int i=0; i<line_count; i++) {
        fprintf(fp, "%s\t", lines[i].address);
        if(strcmp(lines[i].label, "\t") !=0) {
            fprintf(fp, "%s\t", lines[i].label);
        }
        else {
            fprintf(fp, "\t");
        }

        if(strcmp(lines[i].mnemonic, "\t") !=0) {
            fprintf(fp, "%s\t", lines[i].mnemonic);
        }
        else {
            fprintf(fp, "\t");
        }

        if(strcmp(lines[i].operand, "\t") !=0) {
            fprintf(fp, "%s\t", lines[i].operand);
        }
        else {
            fprintf(fp, "\t");
        }

        if(strcmp(lines[i].object_code, "\t") !=0) {
            fprintf(fp, "%s\n", lines[i].object_code);
        }
        else {
            fprintf(fp, "\n");
        }
    }

    fclose(fp);
}
