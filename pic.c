void generate_object_file(const char *obj_filename) {
    // 輸出 H-record
    fprintf(fp, "H%-6s%06X%06X\n", 
    program_name_trimmed, start_addr, program_length);

    // T-Record 切分
    // 累積每行的 object_code，
    //若達 30 bytes 或遇到空白即結束一個 T-Record

    // 最後輸出 E-record
    fprintf(fp, "E%06X\n", start_addr);
}
