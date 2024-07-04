#include <stdio.h>
#include <stdlib.h>

char* lsh_read_line(void);
char** lsh_split_line(char* line);
int lsh_execute(char** args);

void lsh_loop(void)
{
    char* line;
    char** args;
    int status;

    do {
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while (status);
}

// 以下の関数は仮の実装です。実際の処理は後で実装する必要があります。
char* lsh_read_line(void)
{
    char* line = malloc(100); // 簡単のため、固定サイズを確保
    fgets(line, 100, stdin);
    return line;
}

char** lsh_split_line(char* line)
{
    char** args = malloc(sizeof(char*) * 2);
    args[0] = line;
    args[1] = NULL;
    return args;
}

int lsh_execute(char** args)
{
    printf("Command executed: %s\n", args[0]);
    return 0; // 0を返すとループが終了します
}

int main(void)
{
    lsh_loop();
    return 0;
}