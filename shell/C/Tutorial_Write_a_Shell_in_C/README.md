## 元ネタ

https://brennan.io/2015/01/16/write-a-shell-in-c/

## 環境構築

```sh
docker-compose up -d
```

## 操作

```sh
# コンテナに入る
docker-compose exec lsh sh
# makeでコンパイルする
make
# 実行 lsh
./dist/lsh
# 削除する
make clean
```

## 簡単にまとめ

やっていることは、Go のコードと大差はないが、1行を読み込む処理を自作している分より自作の割合は高い