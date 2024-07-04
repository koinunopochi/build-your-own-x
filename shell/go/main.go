package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"strings"
)

// learn-memo: メイン関数。プログラムのエントリーポイントとなり、ユーザーの入力を無限ループで処理する
func main() {
	fmt.Print("Golang Shell\n")
	reader := bufio.NewReader(os.Stdin)
	for {
		fmt.Print("> ")
		// learn-memo: キーボードからの入力を読み取る
		input, err := reader.ReadString('\n')
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
		}

		// learn-memo: 入力から改行文字を削除
		input = strings.TrimSuffix(input, "\n")

		// learn-memo: 空の入力をスキップ
		if input == "" {
			continue
		}

		// learn-memo: 入力されたコマンドを実行
		if err = execInput(input); err != nil {
			fmt.Fprintln(os.Stderr, err)
		}
	}
}

// learn-memo: cdコマンドが引数なしで呼ばれた場合のエラー
var ErrNoPath = errors.New("path required")

// learn-memo: 入力されたコマンドを実行する関数
func execInput(input string) error {
	// learn-memo: 入力をコマンドと引数に分割
	args := strings.Split(input, " ")

	// learn-memo: ビルトインコマンドのチェック
	switch args[0] {
	case "cd":
		// learn-memo: cdコマンドの処理。引数が不足している場合はエラーを返す
		if len(args) < 2 {
			return ErrNoPath
		}
		// learn-memo: ディレクトリを変更
		return os.Chdir(args[1])
	case "exit":
		// learn-memo: プログラムを終了
		os.Exit(0)
	case "hello":
		fmt.Println("world")
		return nil
	}

	// learn-memo: 外部コマンドの実行準備
	cmd := exec.Command(args[0], args[1:]...)

	// learn-memo: 標準エラー出力と標準出力を設定
	cmd.Stderr = os.Stderr
	cmd.Stdout = os.Stdout

	// learn-memo: コマンドを実行し、エラーがあれば返す
	return cmd.Run()
}

// learn-memo: このプログラムは簡単なシェルの実装です。ユーザーからの入力を受け取り、
// コマンドを実行します。ビルトインコマンド（cd, exit）と外部コマンドの実行をサポートしています。
// 無限ループを使用して継続的に入力を受け付け、各コマンドを逐次実行します。

// learn-go: 
// 1. bufio.NewReaderは、効率的に入力を読み取るためのリーダーを作成します。
// 2. strings.TrimSuffixは、文字列から指定された接尾辞を削除します。
// 3. os.Chdirは、現在の作業ディレクトリを変更します。
// 4. exec.Commandは、指定されたプログラムを実行するための*Cmd構造体を返します。
// 5. os.Stderrとos.Stdoutは、標準エラー出力と標準出力を表す*File型の変数です。