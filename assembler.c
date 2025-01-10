#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 1024         // Maximum number of lines in the source
#define MAX_LABEL_LEN 32       // Maximum length of a label
#define MAX_MNEMONIC_LEN 32    // Maximum length of a mnemonic
#define MAX_OPERAND_LEN 64     // Maximum length of an operand
#define MAX_OBJECT_CODE_LEN 64 // Maximum length of object code
#define MAX_SYMBOLS 1024       // Maximum number of symbols in the symbol table

static int error_count = 0;

// Data structure to store one line (similar to the Python 'Line' class)
typedef struct {
    char address[8];
    char label[MAX_LABEL_LEN];
    char mnemonic[MAX_MNEMONIC_LEN];
    char operand[MAX_OPERAND_LEN];
    char object_code[MAX_OBJECT_CODE_LEN];
} Line;

// Data structure to store a symbol and its address
typedef struct {
    char symbol[MAX_LABEL_LEN];
    char address[8];
} Symbol;

// Data structure to store an opcode (mnemonic -> code)
typedef struct {
    char mnemonic[MAX_MNEMONIC_LEN];
    char opcode[4];
} Opcode;

// Data structure to store a register (name -> code)
typedef struct {
    char regName[MAX_LABEL_LEN];
    char regCode[2];
} RegisterMap;

// Data structure to store the Assembler context
typedef struct {
    Line lines[MAX_LINES];
    int line_count;

    Symbol symbol_table[MAX_SYMBOLS];
    int symbol_count;

    Opcode opcode_map[64];     // Enough to store all possible opcodes
    int opcode_count;

    RegisterMap register_map[16];
    int register_count;

    int start_addr;
    int program_length;
} Assembler;

/* 
 * Utility functions
 */

// Trim leading and trailing whitespace from a string
static void trim(char *str) {
    char *end;
    // Trim leading spaces
    while (isspace((unsigned char)*str)) {
        str++;
    }
    // All spaces?
    if (*str == 0) {
        return;
    }
    // Trim trailing spaces
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    end[1] = '\0';
}

// Check if a given mnemonic is a directive
static int is_directive(const char *mnemonic) {
    // For convenience, we store the known directives
    const char *directives[] = {
        "START", "END", "BYTE", "WORD", "RESW", "RESB",
        "ORG", "EQU", "CSECT"
    };
    int n = sizeof(directives) / sizeof(directives[0]);
    for (int i = 0; i < n; i++) {
        if (strcmp(mnemonic, directives[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Find opcode in the opcode_map (linear search)
static const char* find_opcode(Assembler *as, const char *mnemonic) {
    for (int i = 0; i < as->opcode_count; i++) {
        if (strcmp(as->opcode_map[i].mnemonic, mnemonic) == 0) {
            return as->opcode_map[i].opcode;
        }
    }
    return NULL;
}

// Find register code in the register_map
static const char* find_register_code(Assembler *as, const char *regName) {
    for (int i = 0; i < as->register_count; i++) {
        if (strcmp(as->register_map[i].regName, regName) == 0) {
            return as->register_map[i].regCode;
        }
    }
    return NULL;
}

// Check if the token is a mnemonic (either an opcode or a directive)
static int is_mnemonic(Assembler *as, const char *token) {
    if (find_opcode(as, token) != NULL) {
        return 1;
    }
    if (is_directive(token)) {
        return 1;
    }
    return 0;
}

// Add a symbol to the symbol table
static int add_symbol(Assembler *as, const char *symbol, const char *address) {
    // Check for duplicate
    for (int i = 0; i < as->symbol_count; i++) {
        if (strcmp(as->symbol_table[i].symbol, symbol) == 0) {
            // Duplicate symbol
            return 0;
        }
    }
    // Add new symbol
    strcpy(as->symbol_table[as->symbol_count].symbol, symbol);
    strcpy(as->symbol_table[as->symbol_count].address, address);
    as->symbol_count++;
    return 1;
}

// Find a symbol in the symbol table
static const char* find_symbol(Assembler *as, const char *symbol) {
    for (int i = 0; i < as->symbol_count; i++) {
        if (strcmp(as->symbol_table[i].symbol, symbol) == 0) {
            return as->symbol_table[i].address;
        }
    }
    return NULL;
}

/*
 * Assembler initialization
 */
static void initialize_opcode_map(Assembler *as) {
    // List of opcodes
    Opcode opcodes[] = {
        {"LDA", "00"}, {"LDX", "04"}, {"LDL", "08"}, {"STA", "0C"},
        {"STX", "10"}, {"STL", "14"}, {"ADD", "18"}, {"SUB", "1C"},
        {"MUL", "20"}, {"DIV", "24"}, {"COMP","28"}, {"TIX", "2C"},
        {"JEQ", "30"}, {"JGT", "34"}, {"JLT", "38"}, {"J",   "3C"},
        {"JSUB","48"}, {"RSUB","4C"}, {"AND", "40"}, {"OR",  "44"},
        {"LDCH","50"}, {"STCH","54"}, {"LDB", "68"}, {"STB", "78"},
        {"LDT", "74"}, {"LDS", "6C"}, {"STT", "84"}, {"RD",  "D8"},
        {"WD",  "DC"}, {"TD",  "E0"}, {"STSW","E8"}, 
        // Format 2
        {"ADDR","90"}, {"SUBR","94"}, {"COMPR","A0"}, {"MULR","98"},
        {"DIVR","9C"}, {"RMO", "AC"}, {"CLEAR","B4"}, {"TIXR","B8"},
        {"SHIFTL","A4"}, {"SHIFTR","A8"},
        // Additional
        {"STS","7C"},
    };
    int count = sizeof(opcodes) / sizeof(opcodes[0]);
    for (int i = 0; i < count; i++) {
        strcpy(as->opcode_map[i].mnemonic, opcodes[i].mnemonic);
        strcpy(as->opcode_map[i].opcode, opcodes[i].opcode);
    }
    as->opcode_count = count;
}

static void initialize_register_map(Assembler *as) {
    RegisterMap regs[] = {
        {"A",  "0"}, {"X",  "1"}, {"L",  "2"},
        {"B",  "3"}, {"S",  "4"}, {"T",  "5"},
        {"F",  "6"}, {"PC", "8"}, {"SW", "9"}
    };
    int count = sizeof(regs) / sizeof(regs[0]);
    for (int i = 0; i < count; i++) {
        strcpy(as->register_map[i].regName, regs[i].regName);
        strcpy(as->register_map[i].regCode, regs[i].regCode);
    }
    as->register_count = count;
}

// Initialize the assembler data structure
static void assembler_init(Assembler *as) {
    as->line_count = 0;
    as->symbol_count = 0;
    as->start_addr = 0;
    as->program_length = 0;
    // Initialize opcode and register maps
    initialize_opcode_map(as);
    initialize_register_map(as);
}

/*
 * PASS 1
 */
static void pass1(Assembler *as, char **raw_lines, int raw_count) {
    int LC = 0;
    int start_found = 0;
    int line_num = 0;
    
    for (int i = 0; i < raw_count; i++) {
        // Copy the line, then trim
        char buffer[256];
        strcpy(buffer, raw_lines[i]);
        trim(buffer);
        if (strlen(buffer) == 0) {
            continue; // skip empty line
        }
        
        // Prepare a new line structure
        Line current_line;
        memset(&current_line, 0, sizeof(Line));
        
        // Tokenize (split by spaces)
        char *tokens[8];
        int token_count = 0;
        char *p = strtok(buffer, " \t");
        while (p != NULL && token_count < 8) {
            tokens[token_count++] = p;
            p = strtok(NULL, " \t");
        }
        
        if (token_count == 0) {
            continue;
        }
        
        // Check first token: is it a mnemonic or label?
        if (is_mnemonic(as, tokens[0])) {
            // No label
            strcpy(current_line.label, "");
            strcpy(current_line.mnemonic, tokens[0]);
            // Remainder is operand
            if (token_count > 1) {
                int len = 0;
                // Join the rest as operand
                for (int k = 1; k < token_count; k++) {
                    if (k > 1) {
                        current_line.operand[len++] = ' ';
                    }
                    strcat(current_line.operand, tokens[k]);
                    len = strlen(current_line.operand);
                }
            }
        } else {
            // First token is label
            strcpy(current_line.label, tokens[0]);
            if (token_count > 1) {
                strcpy(current_line.mnemonic, tokens[1]);
            }
            if (token_count > 2) {
                int len = 0;
                for (int k = 2; k < token_count; k++) {
                    if (k > 2) {
                        current_line.operand[len++] = ' ';
                    }
                    strcat(current_line.operand, tokens[k]);
                    len = strlen(current_line.operand);
                }
            }
        }
        
        // Handle START
        if (!start_found && strcmp(current_line.mnemonic, "START") == 0) {
            // parse the hex address
            LC = (int)strtol(current_line.operand, NULL, 16);
            as->start_addr = LC;
            sprintf(current_line.address, "%04X", LC);
            as->lines[as->line_count++] = current_line;
            start_found = 1;
            continue;
        }
        
        // If there's a label, add to symbol table
        if (strlen(current_line.label) > 0) {
            char address_str[8];
            sprintf(address_str, "%04X", LC);
            if (!add_symbol(as, current_line.label, address_str)) {
                fprintf(stderr, "Error: Duplicate symbol '%s' at line %d\n",
                        current_line.label, i+1);
                        error_count+=1;
            }
        }
        
        // Set address
        sprintf(current_line.address, "%04X", LC);
        as->lines[as->line_count++] = current_line;
        
        // Update LC based on mnemonic
        if (is_directive(current_line.mnemonic)) {
            if (strcmp(current_line.mnemonic, "START") == 0 ||
                strcmp(current_line.mnemonic, "END") == 0 ||
                strcmp(current_line.mnemonic, "CSECT") == 0) {
                if (strcmp(current_line.mnemonic, "END") == 0) {
                    as->program_length = LC - as->start_addr;
                }
                continue;
            } else if (strcmp(current_line.mnemonic, "BYTE") == 0) {
                if (strncmp(current_line.operand, "C'", 2) == 0) {
                    // count chars
                    // e.g. C'EOF'
                    int len = (int)strlen(current_line.operand);
                    // minus 3 for C' '
                    int count = len - 3;
                    LC += count;
                } else if (strncmp(current_line.operand, "X'", 2) == 0) {
                    // e.g. X'F1'
                    int len = (int)strlen(current_line.operand);
                    // substring between X' and '
                    int hex_len = len - 3;
                    LC += hex_len / 2;
                }
            } else if (strcmp(current_line.mnemonic, "WORD") == 0) {
                LC += 3;
            } else if (strcmp(current_line.mnemonic, "RESW") == 0) {
                int reserve = atoi(current_line.operand);
                LC += 3 * reserve;
            } else if (strcmp(current_line.mnemonic, "RESB") == 0) {
                int reserve = atoi(current_line.operand);
                LC += reserve;
            } else if (strcmp(current_line.mnemonic, "ORG") == 0) {
                const char *addr_str = find_symbol(as, current_line.operand);
                if (addr_str) {
                    LC = (int)strtol(addr_str, NULL, 16);
                } else {
                    LC = (int)strtol(current_line.operand, NULL, 16);
                }
            } else if (strcmp(current_line.mnemonic, "EQU") == 0) {
                // e.g. LABEL EQU value or symbol
                char address_str[8];
                if (isdigit(current_line.operand[0])) {
                    int value = atoi(current_line.operand);
                    sprintf(address_str, "%04X", value);
                    add_symbol(as, current_line.label, address_str);
                } else {
                    const char *addr_str2 = find_symbol(as, current_line.operand);
                    if (addr_str2) {
                        // Update
                        add_symbol(as, current_line.label, addr_str2);
                    } else {
                        fprintf(stderr, "Error: Undefined symbol in EQU at line %d\n",
                                i+1);
                        // default
                        add_symbol(as, current_line.label, "0000");
                        error_count+=1;
                    }
                }
            }
        } else {
            // Format 2 instructions
            const char *format2_list[] = {
                "CLEAR", "TIXR", "ADDR", "SUBR", "COMPR",
                "MULR", "DIVR", "RMO", "SVC", "STPR"
            };
            int is_format2 = 0;
            for (int f = 0; f < 10; f++) {
                if (strcmp(current_line.mnemonic, format2_list[f]) == 0) {
                    is_format2 = 1;
                    break;
                }
            }
            if (is_format2) {
                LC += 2;
            } else {
                LC += 3;
            }
        }
    }
    // If there's no END or if END not updated the length
    if (as->program_length == 0) {
        as->program_length = LC - as->start_addr;
    }
}
    static int is_format2(const char *mnemonic) {
        const char *format2_list[] = {
            "CLEAR", "TIXR", "ADDR", "SUBR", "COMPR",
            "MULR", "DIVR", "RMO", "SVC", "STPR", "SHIFTL", "SHIFTR"
        };
        for (int i = 0; i < 10; i++) {
            if (strcmp(mnemonic, format2_list[i]) == 0) {
                return 1;  // found
            }
        }
        return 0;  // not found
    }
/*
 * PASS 2
 */
static void pass2(Assembler *as) {
    // 專門用來檢查某個 mnemonic 是否為 format2 指令 (純 C 寫法)


    for (int i = 0; i < as->line_count; i++) {
        Line *line = &as->lines[i];
        if (strlen(line->mnemonic) == 0) {
            continue;
        }
        if (is_directive(line->mnemonic)) {
            // Handle BYTE / WORD to fill object_code
            if (strcmp(line->mnemonic, "BYTE") == 0) {
                char *operand = line->operand;
                if (strncmp(operand, "C'", 2) == 0) {
                    // Convert each char to hex
                    char *val = operand + 2; // skip C'
                    int len = strlen(val);
                    if (len >= 1 && val[len-1] == '\'') {
                        val[len-1] = '\0'; // remove trailing '
                    }
                    // Construct object code
                    char obj[256];
                    obj[0] = '\0';
                    for (int c = 0; c < (int)strlen(val); c++) {
                        char tmp[8];
                        sprintf(tmp, "%02X", (unsigned char)val[c]);
                        strcat(obj, tmp);
                    }
                    strcpy(line->object_code, obj);
                } else if (strncmp(operand, "X'", 2) == 0) {
                    // Just copy what's inside X' '
                    char *val = operand + 2;
                    int len = strlen(val);
                    if (len >= 1 && val[len-1] == '\'') {
                        val[len-1] = '\0'; // remove trailing '
                    }
                    // Validate hex string if needed
                    // For now, just upper-case it
                    for (int c = 0; c < (int)strlen(val); c++) {
                        val[c] = (char)toupper(val[c]);
                    }
                    strcpy(line->object_code, val);
                }
            } else if (strcmp(line->mnemonic, "WORD") == 0) {
                // Convert decimal to 6 hex digits
                int value = atoi(line->operand);
                char obj[8];
                sprintf(obj, "%06X", value);
                strcpy(line->object_code, obj);
            }
            continue;
        }
        
        // Check if mnemonic exists
        const char *opcode = find_opcode(as, line->mnemonic);
        if (!opcode) {
            fprintf(stderr, "Error: Undefined mnemonic '%s' at line %d\n", 
                    line->mnemonic, i+1);
                    error_count+=1;
            strcpy(line->object_code, "000000");
            continue;
        }
        
        // Handle format 2
        if (is_format2(line->mnemonic)) {
            // parse registers
            char temp_op[MAX_OPERAND_LEN];
            strcpy(temp_op, line->operand);
            // split by comma
            char *r1 = strtok(temp_op, ",");
            char *r2 = strtok(NULL, ",");
            
            char obj[8];
            if (r1 && r2) {
                const char *reg_code1 = find_register_code(as, r1);
                const char *reg_code2 = find_register_code(as, r2);
                if (reg_code1 && reg_code2) {
                    sprintf(obj, "%s%s%s", opcode, reg_code1, reg_code2);
                } else {
                    fprintf(stderr, "Error: Invalid register(s) at line %d\n", i+1);
                    error_count+=1;
                    sprintf(obj, "0000");
                }
            } else if (r1) {
                // Only one register
                const char *reg_code1 = find_register_code(as, r1);
                if (reg_code1) {
                    sprintf(obj, "%s%s0", opcode, reg_code1);
                } else {
                    fprintf(stderr, "Error: Invalid register '%s' at line %d\n", r1, i+1);
                    error_count+=1;
                    sprintf(obj, "0000");
                }
            } else {
                fprintf(stderr, "Error: Invalid operands for format2 at line %d\n", i+1);
                error_count+=1;
                sprintf(obj, "0000");
            }
            strcpy(line->object_code, obj);
            continue;
        }
        
        // Format 1 (e.g. RSUB) or Format 3
        if (strcmp(line->mnemonic, "RSUB") == 0) {
            // RSUB
            strcpy(line->object_code, "4C0000");
            continue;
        }
        
        // Format 3
        // We will set "nixbpe" bits in a simplified manner
        // n,i bits in decimal: (# => immediate = 1, @ => indirect = 2, else => simple=4)
        
        int nix = 0;
        char address[4];
        strcpy(address, "000");
        
        char operand[MAX_OPERAND_LEN];
        strcpy(operand, line->operand);
        
        // Check immediate
        if (operand[0] == '#') {
            nix = 3; // # => i=1, n=0
            char *symbol = operand + 1; // skip #
            // check digit or symbol
            if (isdigit(symbol[0])) {
                int imm_val = atoi(symbol);
                if (imm_val > 0xFFF) {
                    fprintf(stderr, "Error: Immediate value out of range at line %d\n", i+1);
                    error_count+=1;
                    imm_val = 0;
                }
                sprintf(address, "%03X", imm_val);
            } else {
                const char *addr_str = find_symbol(as, symbol);
                if (addr_str) {
                    int tmp_val = (int)strtol(addr_str, NULL, 16);
                    tmp_val &= 0xFFF; // keep lower 3 hex digits
                    sprintf(address, "%03X", tmp_val);
                } else {
                    fprintf(stderr, "\033[1;31mError: Undefined symbol '%s' at line %d\033[0m\n", symbol, i+1);
                    error_count+=1;
                    strcpy(address, "000");
                }
            }
        }
        else if (operand[0] == '@') {
            nix = 2; // @ => i=0, n=1
            char *symbol = operand + 1; // skip @
            const char *addr_str = find_symbol(as, symbol);
            if (addr_str) {
                int tmp_val = (int)strtol(addr_str, NULL, 16);
                tmp_val &= 0xFFF;
                sprintf(address, "%03X", tmp_val);
            } else {
                fprintf(stderr, "Error: Undefined symbol '%s' at line %d\n", symbol, i+1);
                error_count+=1;
                strcpy(address, "000");
            }
        }
        else {
            // simple addressing => i=1, n=1 => 4 in decimal
            nix = 1;
            // check if there's ,X
            char *pos = strstr(operand, ",X");
            if (pos) {
                // we also need to set x=1 => +1
                nix += 1;
                *pos = '\0'; // separate the symbol from ",X"
            }
            const char *addr_str = find_symbol(as, operand);
            if (addr_str) {
                int tmp_val = (int)strtol(addr_str, NULL, 16);
                tmp_val &= 0xFFF;
                sprintf(address, "%03X", tmp_val);
            } else {
                fprintf(stderr, "Error: Undefined symbol '%s' at line %d\n", operand, i+1);
                error_count+=1;
                strcpy(address, "000");
            }
        }
        
        // Construct object code: opcode + (nix in hex) + address
        char obj_code[16];
        sprintf(obj_code, "%s%X%s", opcode, nix, address);
        strcpy(line->object_code, obj_code);
    }
}

/*
 * Generate Object File
 */
static void generate_object_file(Assembler *as, const char *obj_filename) {
    FILE *fp = fopen(obj_filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open %s for writing.\n", obj_filename);
        error_count+=1;
        return;
    }
    
    // Header record
    // Program name: use the label of the first line (up to 6 chars)
    char program_name[7];
    memset(program_name, 0, sizeof(program_name));
    strncpy(program_name, as->lines[0].label, 6);
    fprintf(fp, "H%-6s%06X%06X\n", program_name, as->start_addr, as->program_length);
    
    // Text records (max 30 bytes => 60 hex digits)
    int current_text_start = -1;
    int current_length = 0;
    char current_text[256];
    current_text[0] = '\0';
    
    for (int i = 0; i < as->line_count; i++) {
        if (strlen(as->lines[i].object_code) > 0) {
            // If we haven't started a text record
            if (current_text_start < 0) {
                current_text_start = (int)strtol(as->lines[i].address, NULL, 16);
            }
            int obj_len = (int)strlen(as->lines[i].object_code) / 2; // each 2 hex => 1 byte
            if (current_length + obj_len > 30) {
                // flush
                fprintf(fp, "T%06X%02X%s\n", current_text_start, current_length, current_text);
                // reset
                current_text_start = (int)strtol(as->lines[i].address, NULL, 16);
                current_length = 0;
                current_text[0] = '\0';
            }
            // append
            strcat(current_text, as->lines[i].object_code);
            current_length += obj_len;
        } else {
            // if there's no object code but we have some in buffer, flush it
            if (current_length > 0) {
                fprintf(fp, "T%06X%02X%s\n", current_text_start, current_length, current_text);
                current_text_start = -1;
                current_length = 0;
                current_text[0] = '\0';
            }
        }
    }
    // flush the last text record
    if (current_length > 0) {
        fprintf(fp, "T%06X%02X%s\n", current_text_start, current_length, current_text);
    }
    
    // End record
    fprintf(fp, "E%06X\n", as->start_addr);
    
    fclose(fp);
}

/*
 * Generate List File
 */
static void generate_list_file(Assembler *as, const char *lst_filename) {
    FILE *fp = fopen(lst_filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open %s for writing.\n", lst_filename);
        error_count+=1;
        return;
    }
    fprintf(fp, "Address\tLabel\tMnemonic\tOperand\tObject Code\n");
    for (int i = 0; i < as->line_count; i++) {
        fprintf(fp, "%s\t%s\t%s\t%s\t%s\n",
                as->lines[i].address,
                as->lines[i].label,
                as->lines[i].mnemonic,
                as->lines[i].operand,
                as->lines[i].object_code);
    }
    fclose(fp);
}

/*
 * The assemble function (main workflow)
 */
static void assemble(Assembler *as, const char *input_file, const char *obj_file, const char *lst_file) {
    // Read all lines from input_file
    FILE *fp = fopen(input_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open %s for reading.\n", input_file);
        error_count+=1;
        exit(1);
    }
    char *raw_lines[MAX_LINES];
    int raw_count = 0;
    char linebuf[256];
    
    while (fgets(linebuf, sizeof(linebuf), fp)) {
        // Allocate memory for line
        raw_lines[raw_count] = (char*)malloc(strlen(linebuf) + 1);
        strcpy(raw_lines[raw_count], linebuf);
        raw_count++;
        if (raw_count >= MAX_LINES) {
            break;
        }
    }
    fclose(fp);
    
    // PASS1
    pass1(as, raw_lines, raw_count);
    // PASS2
    pass2(as);
    // Generate object file
    generate_object_file(as, obj_file);
    // Generate list file
    generate_list_file(as, lst_file);
    
    // free memory
    for (int i = 0; i < raw_count; i++) {
        free(raw_lines[i]);
    }
}

/*
 * main function
 */
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <input_file> <output_obj> <output_lst>\n", argv[0]);
        return 1;
    }
    // Initializ*e assembler
    Assembler assembler;
    assembler_init(&assembler);

    // Assemble
    assemble(&assembler, argv[1], argv[2], argv[3]);
    if (error_count>0){
        printf("\033[1;31mAssembly failed.\033[0m\n");
        printf("\033[1;31m number of errors: %d\033[0m\n",error_count);
    }
    else{
        printf("\033[0;32mAssembly completed.\033[0m\n");
    }

    
    return 0;
}
