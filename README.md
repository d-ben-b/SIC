# Assembler README

## 簡介

本專案是一個針對 **SIC/SICXE** 教學用架構所實作的兩遍式（Two-Pass）組譯器。它能將組合語言程式（.asm 檔）轉換為符合 SIC/XE 規範的物件檔（.obj），並同時產生清單檔（.lst）列出每行的組譯結果。此組譯器可作為教學示範，協助了解組譯器的核心運作與組合語言基礎。

## 功能特色

1. **兩遍式（Two-Pass）組譯流程**

   - **Pass1**：建立符號表 (Symbol Table)，辨識每條指令或 Directive 的長度以更新 LC (Location Counter)。
   - **Pass2**：根據符號表解析標籤與位址，產生最終的機器碼 (Object Code)。

2. **支援多種 Directive**

   - `START`, `END`, `BYTE`, `WORD`, `RESW`, `RESB`, `ORG`, `EQU`, `CSECT` 等。
   - 正確計算 `BYTE` (含十六進位、字元常數) 與 `WORD` (3 bytes)，並為 `RESB`, `RESW` 分配空間。

3. **Format 2 & Format 3 指令**

   - **Format 2**：適用於寄存器操作（例如 `CLEAR A`, `ADDR R1,R2`），只需 2 Bytes。
   - **Format 3**：佔 3 Bytes，支援 `#`（立即）, `@`（間接）與 `,X`（索引）等尋址模式（n、i、x 位元組合）。

4. **物件檔與清單檔**

   - **Object File** (.obj)：含 H/T/E Records，可用標準 Loader 載入。
   - **List File** (.lst)：記錄每一行的 Address、Label、Mnemonic、Operand、Object Code，方便除錯。

5. **T Record 自動分段**
   - 每個 T Record 最多 30 Bytes 的限制，若機器碼長度超過則自動切分。

## 編譯與執行

1. **編譯**  
   使用 GCC 命令：
   ```bash
   gcc assembler.c -o assembler
   ```

### 執行

使用終端機輸入：

```bash
./assembler <input_file.asm> <output_file.obj> <output_file.lst>
```

- `<input_file.asm>`：輸入的組合語言程式檔案
- `<output_file.obj>`：輸出的物件檔案
- `<output_file.lst>`：輸出的清單檔案

#### 範例

```bash
./assembler test.asm test.obj test.lst
```

- `test.asm`：組合語言原始碼
- `test.obj`：輸出之物件檔 (含程式起始位址、程式長度、T Records、程式入口點)
- `test.lst`：輸出之清單檔 (列出每一行的組譯結果與機器碼)

### 查看輸出檔

- **物件檔 (.obj)**：可用來交由 Loader 載入；內含 `H` (Header)、`T` (Text)、`E` (End) 记录，以及程式總長與入口位址等。
- **清單檔 (.lst)**：詳細列出每一行的位址、標籤、助記符、操作數與最終的機器碼，方便學習與除錯。

---

## 程式架構

### `pass1(FILE *fp)`

#### 掃描原始碼

1. **建立符號表 (Symbol Table)**  
   記錄標籤與對應的位址 (LC)。
2. **更新 LC**  
   根據指令或 Directive（`BYTE`, `WORD`, `RESB`, `RESW` 等）更新 LC。
3. **設定程式起始位址**  
   若遇到 `START` 指令，設定程式起始位址 `start_addr`。
4. **處理結尾指令**  
   若遇到 `END`，通常停止程式主要段的長度計算，但可能仍繼續讀取剩餘指令或資料定義。

### `pass2()`

#### 再度掃描組合語言

1. **參考符號表**  
   每行參考符號表取得標籤對應位址。
2. **產生機器碼**
   - **Format 2**（兩個寄存器）：例如 `CLEAR A`
   - **Format 3**（`n`, `i`, `x` 位元）：例如 `LDA LENGTH` 或 `ADD #5`
3. **處理 Directive**  
   為 `BYTE`、`WORD` 等 Directive 產生對應的常數或字串的十六進位表現。

### `generate_object_file(const char *obj_filename)`

1. **寫入 H Record（Header）**  
   包含程式名稱、起始位址、程式長度。
2. **產生 T Record**  
   按照每行產生的機器碼進行累積，每滿 30 Bytes 便結束一個 T Record。
3. **寫入 E Record**  
   標示程式執行入口位址（通常為 `start_addr`）。

### `generate_list_file(const char *lst_filename)`

將每行的組譯結果以表格形式輸出：

- Address、Label、Mnemonic、Operand、Object Code
- 利於檢視組譯過程與除錯。

---

## 後續擴充

### Format 4 指令

- 支援 `+` 指令與 20-bit 位址空間。
- 在 `pass1` / `pass2` 中調整對指令長度與尋址計算的邏輯。

### Base / PC Relative

- 實作更進階的位址計算，加入 `BASE`/`NOBASE` 指令或 PC-relative 尋址機制。
- 支援 **Modification Record**，利於 Loader 做動態重定位。

### 錯誤處理提升

- 提供更多檢核與錯誤訊息，包含詳細行號與敘述，協助使用者快速定位問題。

---
