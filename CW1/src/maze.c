#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 结构体定义
typedef struct {
    char **grid;       // 网格存储
    int rows;          // 总行数
    int cols;          // 总列数
    int player_row;    // 玩家当前行
    int player_col;    // 玩家当前列
    int start_row;     // 起始点行
    int start_col;     // 起始点列
    int exit_row;      // 出口行
    int exit_col;      // 出口列
} Maze;

// 错误码
typedef enum {
    OK = 0,
    ERR_INVALID_ARGS = 1,
    ERR_FILE_IO = 2,
    ERR_INVALID_MAZE = 3,
    ERR_MEMORY = 4,
    ERR_GAME_OVER = 5
} ErrorCode;

/* 文件操作 模块 */
Maze* load_maze(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    Maze *maze = malloc(sizeof(Maze));
    if (!maze) {
        fclose(file);
        return NULL;
    }

    // 初始化迷宫
    maze->rows = 0;
    maze->cols = 0;
    char buffer[1024];

    // 确定列数
    while (fgets(buffer, sizeof(buffer), file)) {
        size_t len = strlen(buffer);
        if (buffer[len-1] == '\n') buffer[--len] = '\0';
        
        if (maze->cols == 0) maze->cols = len;
        else if (len != maze->cols) {
            fclose(file);
            free(maze);
            return NULL;
        }
        maze->rows++;
    }

    // 重置文件指针
    fseek(file, 0, SEEK_SET);
    
    // 内存分配
    maze->grid = malloc(maze->rows * sizeof(char*));
    if (!maze->grid) {
        fclose(file);
        free(maze);
        return NULL;
    }

    // 读取迷宫数据
    for (int i = 0; i < maze->rows; i++) {
        maze->grid[i] = malloc(maze->cols + 1);
        if (!maze->grid[i] || !fgets(maze->grid[i], maze->cols + 2, file)) {
            // 内存分配失败或读取错误
            for (int j = 0; j < i; j++) free(maze->grid[j]);
            free(maze->grid);
            free(maze);
            fclose(file);
            return NULL;
        }
        // 找到起点和出口
        for (int j = 0; j < maze->cols; j++) {
            if (maze->grid[i][j] == 'S') {
                maze->start_row = i;
                maze->start_col = j;
                maze->player_row = i;
                maze->player_col = j;
            } else if (maze->grid[i][j] == 'E') {
                maze->exit_row = i;
                maze->exit_col = j;
            }
        }
    }
    
    fclose(file);
    return maze;
}

/* 验证模块 */
int validate_maze(const Maze *maze) {
    // 尺寸检查
    if (maze->rows < 5 || maze->rows > 100 ||
        maze->cols < 5 || maze->cols > 100) {
        return ERR_INVALID_MAZE;
    }

    int s_count = 0, e_count = 0;
    for (int i = 0; i < maze->rows; i++) {
        for (int j = 0; j < maze->cols; j++) {
            char c = maze->grid[i][j];
            if (c == 'S') s_count++;
            else if (c == 'E') e_count++;
            else if (c != '#' && c != ' ') {
                return ERR_INVALID_MAZE;
            }
        }
    }
    
    if (s_count != 1 || e_count != 1) {
        return ERR_INVALID_MAZE;
    }
    
    return OK;
}

/* 逻辑检查模块 */
void display_maze(const Maze *maze) {
    for (int i = 0; i < maze->rows; i++) {
        for (int j = 0; j < maze->cols; j++) {
            if (i == maze->player_row && j == maze->player_col) {
                putchar('X');
            } else {
                putchar(maze->grid[i][j]);
            }
        }
        putchar('\n');
    }
}

int move_player(Maze *maze, int dr, int dc) {
    int new_row = maze->player_row + dr;
    int new_col = maze->player_col + dc;
    
    // 边界检查
    if (new_row < 0 || new_row >= maze->rows ||
        new_col < 0 || new_col >= maze->cols) {
        return 0;
    }
    
    // 碰撞检查
    if (maze->grid[new_row][new_col] == '#') {
        return 0;
    }
    
    maze->player_row = new_row;
    maze->player_col = new_col;
    return 1;
}

/* 资源管理模块 */
void free_maze(Maze *maze) {
    if (!maze) return;
    
    for (int i = 0; i < maze->rows; i++) {
        free(maze->grid[i]);
    }
    free(maze->grid);
    free(maze);
}

/* 主程序模块 */
int main(int argc, char *argv[]) {
    // 参数验证
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <maze_file>\n", argv[0]);
        return ERR_INVALID_ARGS;
    }

    // 加载迷宫
    Maze *maze = load_maze(argv[1]);
    if (!maze) {
        fprintf(stderr, "Error loading maze file\n");
        return ERR_FILE_IO;
    }

    // 验证迷宫
    if (validate_maze(maze) != OK) {
        fprintf(stderr, "Invalid maze format\n");
        free_maze(maze);
        return ERR_INVALID_MAZE;
    }

    // 游戏循环
    char input;
    while (1) {
        printf("\nEnter move (WASD/M/Q): ");
        scanf(" %c", &input);
        input = toupper(input);

        switch(input) {
            case 'W':
                if (!move_player(maze, -1, 0)) 
                    printf("Invalid move!\n");
                break;
            case 'A':
                if (!move_player(maze, 0, -1))
                    printf("Invalid move!\n");
                break;
            case 'S':
                if (!move_player(maze, 1, 0))
                    printf("Invalid move!\n");
                break;
            case 'D':
                if (!move_player(maze, 0, 1))
                    printf("Invalid move!\n");
                break;
            case 'M':
                display_maze(maze);
                break;
            case 'Q':
                free_maze(maze);
                return OK;
            default:
                printf("Invalid input!\n");
        }

        // 胜利条件判断
        if (maze->player_row == maze->exit_row && 
            maze->player_col == maze->exit_col) {
            printf("\nCongratulations! You escaped the maze!\n");
            free_maze(maze);
            return OK;
        }
    }

    return OK;
}
