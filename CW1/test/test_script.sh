#!/bin/bash
set -eo pipefail

# 配对文件
declare -A MAZE_INPUT_MAP=(
    ["maze1.txt"]="maze1_input.txt"
    ["maze2.txt"]="maze2_input.txt"
    ["maze3.txt"]="maze3_input.txt"
    ["maze4.txt"]="maze4_input.txt"
)

# 临时报告
TEMP_DIR=".testcases"
REPORT_FILE="test_report.log"
mkdir -p "$TEMP_DIR"

# 颜色代码
RED='\033[1;31m'
GREEN='\033[1;32m'
NC='\033[0m'

# 验证输入文件
validate_input() {
    local input_file=$1
    echo -n "校验输入文件: $input_file ... "
    
    # 确认格式
    grep -q '^>>>CASE' "$input_file" || { echo -e "${RED}缺失用例标记${NC}"; return 1; }
    awk '!/^(>>>CASE|#|$|[WASDQ])/' "$input_file" | grep -q . && { echo -e "${RED}含非法字符${NC}"; return 1; }
    
    echo -e "${GREEN}通过${NC}"
}

execute_test() {
    local maze=$1
    local input=$2
    
    csplit -s -z -f "$TEMP_DIR/case" "$input" '/^>>>CASE/' '{*}' 2>/dev/null
    for case_file in "$TEMP_DIR"/case*; do
        [ ! -f "$case_file" ] && continue
        
        local case_name=$(head -1 "$case_file" | sed 's/^>>>CASE[0-9]* //')
        tail -n +2 "$case_file" > "$TEMP_DIR/current_input"
        
        echo -n "测试 $maze → $case_name ... "
        if timeout 5s ./maze_program "$maze" < "$TEMP_DIR/current_input" >/dev/null 2>&1; then
            echo -e "${GREEN}成功${NC}"
        else
            echo -e "${RED}失败 (退出码 $?)${NC}"
        fi
    done
}

echo "========== 输入驱动测试 ==========" > "$REPORT_FILE"
for maze in "${!MAZE_INPUT_MAP[@]}"; do
    input="${MAZE_INPUT_MAP[$maze]}"
    
    # 验证输入文件
    if ! validate_input "$input" >> "$REPORT_FILE"; then
        echo -e "${RED}跳过无效输入文件: $input${NC}" | tee -a "$REPORT_FILE"
        continue
    fi
    
    # 执行测试
    echo "执行 $maze 测试集：" | tee -a "$REPORT_FILE"
    execute_test "$maze" "$input" | tee -a "$REPORT_FILE"
done

# 清理
rm -rf "$TEMP_DIR"
