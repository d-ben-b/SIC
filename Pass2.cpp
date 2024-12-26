#include "pass2.h"
#include "Opcode_map.h"

void generate_machine_code() {
    std::string size = decToHex(hexToDec(arr[arr.size() - 2].first) - hexToDec(starting_address));
    std::cout << "H " << program_name << " " << starting_address << " " << size << std::endl;

    size_t i = 0, save = 0, eternal_flag = 0;
    std::string firstExecutable;

    while (1) {
        int flag = 0, length = 0, codes = 0;
        i = save;

        // 生成文本記錄 (Text Record)
        while (i < arr.size() - 1 && object_code[i] != "\t" && codes < 30) {
            length++;
            if (!flag && get_opcode(arr[i].second.second.first) != "-1") {
                std::cout << "T ";
                flag = 1;
                std::cout << arr[i].first << " ";
                if (!eternal_flag) {
                    firstExecutable = arr[i].first;
                    eternal_flag = 1;
                }
            }
            if (object_code[i] != "\t") {
                codes += (object_code[i].size() / 2);
                if (codes > 30) {
                    codes -= (object_code[i].size() / 2);
                    break;
                }
            }
            i++;
        }

        if (!flag) {
            save++;
            continue;
        }

        // 輸出文本記錄長度
        std::cout << decToHex(codes) << " ";
        i = save;
        codes = 0;

        // 輸出文本記錄內容
        for (; codes < 30; i++) {
            if (i >= arr.size() - 1)
                break;
            if (object_code[i] != "\t") {
                codes += (object_code[i].size() / 2);
                if (codes > 30)
                    break;
                std::cout << object_code[i] << " ";
            } else {
                i++;
                break;
            }
        }

        save = i;

        if (i >= arr.size() - 1)
            break;
        std::cout << std::endl;
    }

    // 輸出尾記錄
    std::cout << std::endl << "E " << firstExecutable << std::endl;
}
