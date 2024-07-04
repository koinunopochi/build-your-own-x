// 元ネタ：https://github.com/brenns10/lsh/blob/master/src/main.c

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// learn-c: これらは必要なヘッダファイルのインクルードです。つまり、JSなどでいうところのimport文です。
// <sys/wait.h>: プロセスの待機関数を提供
// <sys/types.h>: 様々なデータ型の定義を提供
// <unistd.h>: UNIX標準関数を提供
// <stdlib.h>: 標準ユーティリティ関数を提供
// <stdio.h>: 標準入出力関数を提供
// <string.h>: 文字列操作関数を提供

/*
  シェルの組み込みコマンド用の関数宣言:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

// learn-c: これらは関数のプロトタイプ宣言です。関数の戻り値の型と引数の型を事前に宣言しています。
// 事前に型を宣言することで、コンパイラが正しく解釈できるようになります。
// TSなどの静的型付け言語でいうところの型アノテーションを別途記述しているイメージです。

/*
  組み込みコマンドのリストと、対応する関数のリスト。
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

// learn-c: builtin_str は組み込みコマンドの名前を格納する文字列の配列です。
// builtin_func は対応する関数ポインタの配列です。関数ポインタは、関数を指す変数で、
// ここでは char ** を引数にとり int を返す関数を指しています。

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

// learn-c: この関数は組み込みコマンドの数を返します。
// sizeof(builtin_str) は配列全体のサイズを、sizeof(char *) は配列の1要素のサイズを返します。
// その除算で要素数が得られます。

/*
  組み込み関数の実装。
*/

/**
   @brief 組み込みコマンド: ディレクトリを変更する。
   @param args 引数のリスト。args[0] は "cd"。args[1] は移動先のディレクトリ。
   @return 常に1を返し、実行を継続する。
 */
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

// learn-c: この関数はcdコマンドを実装しています。
// args[1]がNULLの場合（引数が与えられていない場合）、エラーメッセージを出力します。
// それ以外の場合、chdir関数を使用してディレクトリを変更します。
// エラーが発生した場合はperror関数でエラーメッセージを出力します。

/**
   @brief 組み込みコマンド: ヘルプを表示する。
   @param args 引数のリスト。調査されません。
   @return 常に1を返し、実行を継続する。
 */
int lsh_help(char **args)
{
  int i;
  printf("Stephen Brennan's LSH\n");
  printf("プログラム名と引数を入力し、Enterを押してください。\n");
  printf("以下のコマンドが組み込まれています:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    // note: %s で文字列を置き換えている。今回は実際のコマンドを表示している。
    printf("  %s\n", builtin_str[i]);
  }

  printf("他のプログラムについての情報は 'man' コマンドを使用してください。\n");
  return 1;
}

// learn-c: この関数はhelpコマンドを実装しています。
// シェルの使用方法と組み込みコマンドのリストを表示します。
// for ループを使用して、すべての組み込みコマンドを表示しています。

/**
   @brief 組み込みコマンド: シェルを終了する。
   @param args 引数のリスト。調査されません。
   @return 常に0を返し、実行を終了する。
 */
int lsh_exit(char **args)
{
  return 0;
}

// learn-c: この関数はexitコマンドを実装しています。
// 単に0を返すだけですが、これによりメインループが終了します。

/**
  @brief プログラムを起動し、終了するまで待機する。
  @param args 引数のNULL終端リスト（プログラムを含む）。
  @return 常に1を返し、実行を継続する。
 */
int lsh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // 子プロセス
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // フォークエラー
    perror("lsh");
  } else {
    // 親プロセス
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

// learn-c: この関数は外部コマンドを実行するためのものです。
// fork() を使用して新しいプロセスを作成します。
// 子プロセスでは execvp() を使用して新しいプログラムを実行します。
// 親プロセスは waitpid() を使用して子プロセスの終了を待ちます。

/**
   @brief シェルの組み込みコマンドを実行するか、プログラムを起動する。
   @param args 引数のNULL終端リスト。
   @return シェルの実行を継続する場合は1、終了する場合は0を返す。
 */
int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // 空のコマンドが入力された。
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

// learn-c: この関数は入力されたコマンドを実行します。
// まず組み込みコマンドかどうかをチェックし、一致すれば対応する関数を呼び出します。
// 組み込みコマンドでなければ、lsh_launch() を呼び出して外部コマンドとして実行します。

/**
   @brief 標準入力から1行の入力を読み取る。
   @return 標準入力からの行。
 */
char *lsh_read_line(void)
{
#ifdef LSH_USE_STD_GETLINE
  char *line = NULL;
  ssize_t bufsize = 0; // getline に バッファを割り当ててもらう
  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // EOF を受け取った
    } else  {
      perror("lsh: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
#define LSH_RL_BUFSIZE 1024
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // 文字を読み取る
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // バッファを超えた場合、再割り当てする。
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}

// learn-c: この関数は標準入力から1行を読み取ります。
// #ifdef を使用して2つの実装を提供しています。LSH_USE_STD_GETLINEが定義されている場合は
// getline() 関数を使用し、そうでない場合は独自の実装を使用します。
// 独自の実装では、文字を1つずつ読み取り、必要に応じてバッファを拡張しています。

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief 行をトークンに分割する（非常に単純な方法で）。
   @param line 分割する行。
   @return トークンのNULL終端配列。
 */
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

// learn-c: この関数は入力行をトークン（単語）に分割します。
// strtok() 関数を使用して、指定された区切り文字でラインを分割します。
// トークンは動的に割り当てられた配列に格納され、必要に応じて拡張されます。

/**
   @brief 入力を取得し実行するループ。
 */
void lsh_loop(void)
{
  char *line;
  char **args;
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

// learn-c: これはシェルのメインループです。
// プロンプトを表示し、コマンドを読み取り、解析し、実行します。
// このプロセスは status が 0 になるまで（つまり exit コマンドが実行されるまで）繰り返されます。

/**
   @brief メインエントリーポイント。
   @param argc 引数の数。
   @param argv 引数のベクトル。
   @return ステータスコード
 */
int main(int argc, char **argv)
{
  // 設定ファイルがあれば読み込む。

  // コマンドループを実行。
  lsh_loop();

  // シャットダウン/クリーンアップを実行。

  return EXIT_SUCCESS;
}

// learn-c: これはプログラムのメイン関数です。
// ここでlsh_loop() を呼び出してシェルのメインループを開始します。
// プログラムは正常終了時に EXIT_SUCCESS を返します。
