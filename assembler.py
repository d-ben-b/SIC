import sys


# 定義每一行的結構
class Line:
    def __init__(
        self, address="0000", label="", mnemonic="", operand="", object_code=""
    ):
        self.address = address
        self.label = label
        self.mnemonic = mnemonic
        self.operand = operand
        self.object_code = object_code


# 定義組譯器類
class Assembler:
    def __init__(self):
        self.lines = []
        self.symbol_table = {}
        self.opcode_map = {}
        self.register_map = {}
        self.start_addr = 0
        self.program_length = 0
        self.pass2_errors = []

        self.initialize_opcode_map()
        self.initialize_register_map()

    def initialize_opcode_map(self):
        # 初始化 Opcode 映射
        # 格式1、格式3、格式4指令
        opcodes = [
            # 格式1 / 格式3/4 指令
            ("LDA", "00"),  # Load accumulator
            ("LDX", "04"),  # Load index register
            ("LDL", "08"),  # Load link register
            ("STA", "0C"),  # Store accumulator
            ("STX", "10"),  # Store index register
            ("STL", "14"),  # Store link register
            ("ADD", "18"),  # Add
            ("SUB", "1C"),  # Subtract
            ("MUL", "20"),  # Multiply
            ("DIV", "24"),  # Divide
            ("COMP", "28"),  # Compare
            ("TIX", "2C"),  # Test index register
            ("JEQ", "30"),  # Jump if equal
            ("JGT", "34"),  # Jump if greater
            ("JLT", "38"),  # Jump if less
            ("J", "3C"),  # Jump
            ("JSUB", "48"),  # Jump to subroutine
            ("RSUB", "4C"),  # Return from subroutine
            ("AND", "40"),  # Logical AND
            ("OR", "44"),  # Logical OR
            ("LDCH", "50"),  # Load character
            ("STCH", "54"),  # Store character
            ("LDB", "68"),  # Load base register
            ("STB", "78"),  # Store base register
            ("LDT", "74"),  # Load T register
            ("LDS", "6C"),  # Load S register
            ("STT", "84"),  # Store T register
            ("RD", "D8"),  # Read
            ("WD", "DC"),  # Write
            ("TD", "E0"),  # Test device
            ("STSW", "E8"),  # Store status word
            # 格式2 指令
            ("ADDR", "90"),  # Add register
            ("SUBR", "94"),  # Subtract register
            ("COMPR", "A0"),  # Compare register
            ("MULR", "98"),  # Multiply register
            ("DIVR", "9C"),  # Divide register
            ("RMO", "AC"),  # Register move
            ("CLEAR", "B4"),  # Clear register
            ("TIXR", "B8"),  # Test index register
            ("SHIFTL", "A4"),  # Shift left
            ("SHIFTR", "A8"),  # Shift right
            # 新增的指令（教材附錄提到的）
            ("STS", "7C"),  # Store S register
        ]

        for mnemonic, opcode in opcodes:
            self.opcode_map[mnemonic] = opcode

    def initialize_register_map(self):
        # 初始化 Register 映射
        registers = [
            ("A", "0"),
            ("X", "1"),
            ("L", "2"),
            ("B", "3"),
            ("S", "4"),
            ("T", "5"),
            ("F", "6"),
            ("PC", "8"),
            ("SW", "9"),
        ]
        for reg, code in registers:
            self.register_map[reg] = code

    def is_directive(self, mnemonic):
        directives = {
            "START",
            "END",
            "BYTE",
            "WORD",
            "RESW",
            "RESB",
            "ORG",
            "EQU",
            "CSECT",
        }
        return mnemonic in directives

    def is_mnemonic(self, token):
        # 判斷是否為助記符或控制指令
        return token in self.opcode_map or self.is_directive(token)

    def add_symbol(self, symbol, address, line_num):
        if symbol in self.symbol_table:
            print(f"Error: Duplicate symbol '{symbol}' at line {line_num}")
            return False
        self.symbol_table[symbol] = address
        return True

    def find_symbol(self, symbol):
        return self.symbol_table.get(symbol, None)

    def pass1(self, lines):
        LC = 0
        start_found = False
        program_name = ""
        line_num = 0

        for raw_line in lines:
            line_num += 1
            # 移除換行符和多餘的空格
            line_str = raw_line.strip()
            if not line_str:
                continue  # 跳過空行

            # 初始化當前行
            current_line = Line()

            # 使用分隔符解析行
            tokens = line_str.split()
            if not tokens:
                continue

            # 判斷第一個詞是否為助記符
            if self.is_mnemonic(tokens[0]):
                # 無標籤
                current_line.label = ""
                current_line.mnemonic = tokens[0]
                if len(tokens) > 1:
                    current_line.operand = " ".join(tokens[1:])
            else:
                # 有標籤
                current_line.label = tokens[0]
                if len(tokens) > 1:
                    current_line.mnemonic = tokens[1]
                if len(tokens) > 2:
                    current_line.operand = " ".join(tokens[2:])

            # 處理 START 指令
            if not start_found and current_line.mnemonic == "START":
                program_name = current_line.label[:6]
                self.start_addr = int(current_line.operand, 16)
                LC = self.start_addr
                current_line.address = f"{LC:04X}"
                self.lines.append(current_line)
                start_found = True
                continue

            # 如果有標籤，添加到符號表
            if current_line.label:
                if not self.add_symbol(current_line.label, f"{LC:04X}", line_num):
                    pass  # Duplicate symbol error already printed

            # 設定地址
            current_line.address = f"{LC:04X}"
            self.lines.append(current_line)

            # 處理指令來更新 LC
            mnemonic = current_line.mnemonic
            operand = current_line.operand

            if self.is_directive(mnemonic):
                if mnemonic in {"START", "END", "CSECT"}:
                    # 不影響 LC
                    if mnemonic == "END":
                        self.program_length = LC - self.start_addr
                    continue
                elif mnemonic == "BYTE":
                    if operand.startswith("C'") and operand.endswith("'"):
                        # C'EOF' -> 每個字符佔1 byte
                        value = operand[2:-1]
                        LC += len(value)
                    elif operand.startswith("X'") and operand.endswith("'"):
                        # X'F1' -> 每兩個十六進位數字佔1 byte
                        value = operand[2:-1]
                        if len(value) % 2 != 0:
                            print(
                                f"Error: Invalid hex string in BYTE at line {line_num}"
                            )
                        LC += len(value) // 2
                elif mnemonic == "WORD":
                    # WORD -> 3 bytes
                    LC += 3
                elif mnemonic == "RESW":
                    # RESW -> 3 * operand bytes
                    reserve = int(operand)
                    LC += 3 * reserve
                elif mnemonic == "RESB":
                    # RESB -> operand bytes
                    reserve = int(operand)
                    LC += reserve
                elif mnemonic == "ORG":
                    # ORG -> 設置 LC 為操作數的值
                    addr = self.find_symbol(operand)
                    if addr:
                        LC = int(addr, 16)
                    else:
                        LC = int(operand, 16)
                elif mnemonic == "EQU":
                    # EQU -> 定義符號的值
                    if operand.isdigit():
                        value = int(operand)
                    else:
                        addr = self.find_symbol(operand)
                        if addr:
                            value = int(addr, 16)
                        else:
                            print(f"Error: Undefined symbol in EQU at line {line_num}")
                            value = 0
                    # 更新符號表
                    self.symbol_table[current_line.label] = f"{value:04X}"
            else:
                # 判斷指令格式
                format2_mnemonics = {
                    "CLEAR",
                    "TIXR",
                    "ADDR",
                    "SUBR",
                    "COMPR",
                    "MULR",
                    "DIVR",
                    "RMO",
                    "SVC",
                    "STPR",
                }
                if mnemonic in format2_mnemonics:
                    LC += 2
                else:
                    LC += 3

        # 如果 END 指令未被處理，計算程序長度
        if self.program_length == 0:
            self.program_length = LC - self.start_addr

    def pass2(self):
        format2_mnemonics = {
            "CLEAR",
            "TIXR",
            "ADDR",
            "SUBR",
            "COMPR",
            "MULR",
            "DIVR",
            "RMO",
            "SVC",
            "STPR",
        }

        for idx, line in enumerate(self.lines):
            if not line.mnemonic:
                continue  # 跳過無助記符的行

            if self.is_directive(line.mnemonic):
                # 特殊處理 END 指令的 RSUB
                if line.mnemonic == "END":
                    continue
                elif line.mnemonic in {"BYTE", "WORD"}:
                    # 處理 BYTE 和 WORD
                    if line.mnemonic == "BYTE":
                        operand = line.operand
                        if operand.startswith("C'") and operand.endswith("'"):
                            value = operand[2:-1]
                            obj_code = "".join([f"{ord(c):02X}" for c in value])
                            line.object_code = obj_code
                        elif operand.startswith("X'") and operand.endswith("'"):
                            value = operand[2:-1]
                            # Validate hex string
                            if all(c in "0123456789ABCDEFabcdef" for c in value):
                                line.object_code = value.upper()
                            else:
                                print(
                                    f"Error: Invalid hex string in BYTE at line {idx+1}"
                                )
                    elif line.mnemonic == "WORD":
                        value = int(line.operand)
                        obj_code = f"{value:06X}"
                        line.object_code = obj_code
                continue

            if line.mnemonic in self.opcode_map:
                opcode = self.opcode_map[line.mnemonic]
            else:
                print(f"Error: Undefined mnemonic '{line.mnemonic}' at line {idx+1}")
                line.object_code = "000000"
                continue

            if line.mnemonic in format2_mnemonics:
                # 格式2指令
                operands = line.operand.split(",")
                if len(operands) == 2:
                    reg1 = operands[0].strip()
                    reg2 = operands[1].strip()
                    reg_code1 = self.register_map.get(reg1, None)
                    reg_code2 = self.register_map.get(reg2, None)
                    if reg_code1 and reg_code2:
                        obj_code = f"{opcode}{reg_code1}{reg_code2}"
                        line.object_code = obj_code
                    else:
                        print(
                            f"Error: Invalid register(s) '{reg1}, {reg2}' at line {idx+1}"
                        )
                        line.object_code = "0000"
                elif len(operands) == 1:
                    reg1 = operands[0].strip()
                    reg_code1 = self.register_map.get(reg1, None)
                    if reg_code1:
                        obj_code = f"{opcode}{reg_code1}0"
                        line.object_code = obj_code
                    else:
                        print(f"Error: Invalid register '{reg1}' at line {idx+1}")
                        line.object_code = "0000"
                else:
                    print(
                        f"Error: Invalid operands for format2 instruction at line {idx+1}"
                    )
                    line.object_code = "0000"
                continue

            # 格式1指令（如 RSUB）和格式3指令
            if line.mnemonic == "RSUB":
                line.object_code = "4C0000"
                continue

            # 處理格式3指令
            operand = line.operand
            nix = 0
            address = "000"

            if operand.startswith("#"):
                # 立即尋址
                nix += 1  # i=1, n=0
                symbol = operand[1:]
                if symbol.isdigit():
                    imm_val = int(symbol)
                    if imm_val > 0xFFF:
                        print(f"Error: Immediate value out of range at line {idx+1}")
                        imm_val = 0
                    address = f"{imm_val:03X}"
                else:
                    addr = self.find_symbol(symbol)
                    if addr:
                        address = (
                            addr[-3:].upper()
                            if len(addr) >= 3
                            else f"{int(addr,16):03X}"
                        )
                    else:
                        print(f"Error: Undefined symbol '{symbol}' at line {idx+1}")
                        address = "000"
            elif operand.startswith("@"):
                # 間接尋址
                nix += 2  # i=0, n=1
                symbol = operand[1:]
                addr = self.find_symbol(symbol)
                if addr:
                    address = (
                        addr[-3:].upper() if len(addr) >= 3 else f"{int(addr,16):03X}"
                    )
                else:
                    print(f"Error: Undefined symbol '{symbol}' at line {idx+1}")
                    address = "000"
            else:
                # 簡單尋址
                nix += 4  # i=1, n=1
                symbol = operand
                # 檢查是否有索引尋址
                if ",X" in symbol:
                    nix += 1  # x=1
                    symbol = symbol.replace(",X", "")
                addr = self.find_symbol(symbol)
                if addr:
                    address = (
                        addr[-3:].upper() if len(addr) >= 3 else f"{int(addr,16):03X}"
                    )
                else:
                    print(f"Error: Undefined symbol '{symbol}' at line {idx+1}")
                    address = "000"

            obj_code = f"{opcode}{nix:X}{address}"
            line.object_code = obj_code

    def generate_object_file(self, obj_filename):
        with open(obj_filename, "w") as f:
            # H-Record: Header
            program_name = self.lines[0].label[:6].ljust(6)
            f.write(f"H{program_name}{self.start_addr:06X}{self.program_length:06X}\n")

            # T-Records: Text
            current_text_start = None
            current_text = ""
            current_length = 0

            for line in self.lines:
                if line.object_code and line.object_code != "\t":
                    if current_text_start is None:
                        current_text_start = int(line.address, 16)
                    # 每個T記錄最多30字節
                    if current_length + len(line.object_code) // 2 > 30:
                        # 寫入當前T記錄
                        f.write(
                            f"T{current_text_start:06X}{current_length:02X}{current_text}\n"
                        )
                        # 重置
                        current_text_start = int(line.address, 16)
                        current_text = ""
                        current_length = 0
                    current_text += line.object_code
                    current_length += len(line.object_code) // 2
                else:
                    if current_length > 0:
                        # 寫入當前T記錄
                        f.write(
                            f"T{current_text_start:06X}{current_length:02X}{current_text}\n"
                        )
                        # 重置
                        current_text_start = None
                        current_text = ""
                        current_length = 0
            # 寫入最後一個T記錄
            if current_length > 0:
                f.write(
                    f"T{current_text_start:06X}{current_length:02X}{current_text}\n"
                )

            # E-Record: End
            f.write(f"E{self.start_addr:06X}\n")

    def generate_list_file(self, lst_filename):
        with open(lst_filename, "w") as f:
            # 標題
            f.write("Address\tLabel\tMnemonic\tOperand\tObject Code\n")
            for line in self.lines:
                address = line.address
                label = line.label if line.label else ""
                mnemonic = line.mnemonic if line.mnemonic else ""
                operand = line.operand if line.operand else ""
                object_code = line.object_code if line.object_code else ""
                f.write(f"{address}\t{label}\t{mnemonic}\t{operand}\t{object_code}\n")

    def assemble(self, input_file, obj_file, lst_file):
        with open(input_file, "r") as f:
            raw_lines = f.readlines()

        self.pass1(raw_lines)
        self.pass2()
        self.generate_object_file(obj_file)
        self.generate_list_file(lst_file)


# 主函數
def main():
    if len(sys.argv) != 4:
        print("Usage: python assembler.py <input_file> <output_obj> <output_lst>")
        sys.exit(1)

    input_file = sys.argv[1]
    obj_file = sys.argv[2]
    lst_file = sys.argv[3]

    assembler = Assembler()
    assembler.assemble(input_file, obj_file, lst_file)
    print("Assembly completed successfully.")


if __name__ == "__main__":
    main()
