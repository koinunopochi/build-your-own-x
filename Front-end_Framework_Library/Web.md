元ネタ：https://medium.com/@sweetpalma/gooact-react-in-160-lines-of-javascript-44e0742ad60f

## はじめに
***重要なお知らせ***： このチュートリアルを行うには、少なくともReactの基本的な知識が必要です。

**React**は素晴らしいライブラリです。多くの開発者がその単純性、パフォーマンス、宣言的な方法論に惚れ込んでいます。しかし、私に特別な理由があります。それがReactのアーキテクチャを理解することです。React の核となる理念は単純ながらも不思議と魅力的に思えます。Reactの基本原理を理解することで、より高速で安全なコーディングができるようになるでしょう。

このチュートリアルでは、React のコンポーネントAPI や独自のバーチャルDOM（VDOM）実装を含む、完全に機能するReactクローンを作成する方法を説明します。このチュートリアルは4つのセクションに分かれており、それぞれ1つの主要なトピックを扱います:

* **要素**: このセクションでは、JSXブロックがDOMの軽量版であるVDOMにどのように変換されるかを学びます。 
* **レンダリング**: このセクションでは、VDOMを通常のDOMに変換する方法を示します。
* **パッチ処理**: このセクションでは、「key」プロパティがなぜ重要か、そしてVDOMを使ってどのように既存のDOMを効率的にパッチ処理するかを説明します。
* **コンポーネント**: 最終セクションでは、Reactコンポーネントの作成、ライフサイクル、レンダリング手順について説明します。

各セクションの最後には、これまでの進捗を即座に確認できるライブ例が用意されています。それでは始めましょう。

## 要素
要素とは、実際のDOMの軽量なオブジェクト表現です。ノードタイプ、属性、子要素リストなどの重要な情報を保持しているため、後で簡単にレンダリングできます。要素のツリー状の構成を「仮想DOM（VDOM）」と呼びます。以下に、VDOMの一例を示します。

```json
{
    "type": "ul",
    "props": {
        "className": "some-list"
    },
    "children": [
        {
            "type": "li",
            "props": {
                "className": "some-list__item"
            },
            "children": [
                "One"
            ]
        },
        {
            "type": "li",
            "props": {
                "className": "some-list__item"
            },
            "children": [
                "Two"
            ]
        }
    ]
}
```

通常、Reactの開発者はJSXを使って上記のようなモンスターオブジェクトを記述するのではなく、よりシンプルな構文を使います。JSXは、JavaScriptコードとHTMLタグが洗練されたように見えるものです。

```js
/** @jsx createElement */
const list = <ul className="some-list">
    <li className="some-list__item">One</li>
    <li className="some-list__item">Two</li>
</ul>;
```

JSXを実行するには、通常の関数呼び出しに変換する必要があります。そのためのpragmaコメントで、使用する関数を定義します。

```js
const list = createElement('ul', {className: 'some-list'},
    createElement('li', {className: 'some-list__item'}, 'One'),
    createElement('li', {className: 'some-list__item'}, 'Two'),
);
```

最終的に、ランタイム中にこの関数が呼び出され、上述のVDOM構造を返すことが期待されます。私たちの実装は簡潔ですが、プリミティブに見えながらも目的を完璧に果たします。

```js
const createElement = (type, props, ...children) => {
    if (props === null) props = {};
    return {type, props, children};
};
```

最初のライブプレイグラウンドは[**こちら**](https://jsbin.com/gizejat/edit?html,js,output)で公開されています。上述の関数と、それによって生成されたVDOMツリーが含まれています。

### まとめ
要は、実際のDOMの軽量なオブジェクト（上記のJSON形式）で示されたが、これを生で触るのは難易度が高いため、

実際は次のように記載する：
```js
/** @jsx createElement */
const list = <ul className="some-list">
    <li className="some-list__item">One</li>
    <li className="some-list__item">Two</li>
</ul>;
```
このようなJSXで記載できるが、内部的な処理としては次のように実装される：
```js
const list = createElement('ul', {className: 'some-list'},
    createElement('li', {className: 'some-list__item'}, 'One'),
    createElement('li', {className: 'some-list__item'}, 'Two'),
);
```
最終的には次のように実装して実態にすることができる：
```js
const createElement = (type, props, ...children) => {
    if (props === null) props = {};
    return {type, props, children};
};
```


## レンダリング
**レンダリング**とは、VDOMを実際の表示可能なDOMに変換する過程です。一般的には、VDOMツリーを下っていきながら、各ノードに対応するDOMエレメントを作成する比較的単純なアルゴリズムです。

```js
const render = (vdom, parent=null) => {
    const mount = parent ? (el => parent.appendChild(el)) : (el => el);
    if (typeof vdom == 'string' || typeof vdom == 'number') {
        return mount(document.createTextNode(vdom));
    } else if (typeof vdom == 'boolean' || vdom === null) {
        return mount(document.createTextNode(''));
    } else if (typeof vdom == 'object' && typeof vdom.type == 'function') {
        return Component.render(vdom, parent);
    } else if (typeof vdom == 'object' && typeof vdom.type == 'string') {
        const dom = mount(document.createElement(vdom.type));
        for (const child of [/* flatten */].concat(...vdom.children))
            render(child, dom);
        for (const prop in vdom.props)
            setAttribute(dom, prop, vdom.props[prop]);
        return dom;
    } else {
        throw new Error(`Invalid VDOM: ${vdom}.`);
    }
}

const setAttribute = (dom, key, value) => {
    if (typeof value == 'function' && key.startsWith('on')) {
        const eventType = key.slice(2).toLowerCase();
        dom.__gooactHandlers = dom.__gooactHandlers || {};
        dom.removeEventListener(eventType, dom.__gooactHandlers[eventType]);
        dom.__gooactHandlers[eventType] = value;
        dom.addEventListener(eventType, dom.__gooactHandlers[eventType]);
    } else if (key == 'checked' || key == 'value' || key == 'className') {
        dom[key] = value;
    } else if (key == 'style' && typeof value == 'object') {
        Object.assign(dom.style, value);
    } else if (key == 'ref' && typeof value == 'function') {
        value(dom);
    } else if (key == 'key') {
        dom.__gooactKey = value;
    } else if (typeof value != 'object' && typeof value != 'function') {
        dom.setAttribute(key, value);
    }
};
```

上記のコードは見ためが複雑ですが、いくつかの smaller parts に分割することで、より簡単に理解できます。

* **カスタムAttributeセッター**: VDOMに渡されるプロパティは、DOMの観点から必ずしも有効ではありません。イベントハンドラ、キー識別子、値などは個別に処理する必要があります。
* **プリミティブVDOMレンダリング**: 文字列、数値、ブール値、nullなどのプリミティブは、プレーンなテキストノードに変換されます。
* **複雑VDOMレンダリング**: 文字列タグを持つノードは、子要素を再帰的にレンダリングしたDOMエレメントに変換されます。
* **コンポーネントVDOMレンダリング**: 関数タグを持つノードは別途処理されます。これについては後ほど実装します。

2番目のライブプレイグラウンドは[**こちら**](https://jsbin.com/nohapeg/edit?html,js,output)で公開されています。レンダリングプロセスの小さな例が示されています。


## パッチング
**パッチング**とは、既存のDOMと新しく構築したVDOMツリーを調整する過程です。

深くネストされた、頻繁に更新されるVDOMがあると考えてみましょう。何かが変更されると、たとえ小さな部分であっても表示される必要があります。単純な実装では、毎回完全に再レンダリングする必要があります:

* 既存のDOMノードを削除する
* 全て再レンダリングする

これは効率性の観点から問題があります。DOMの構築と適切なペイントは非常に高価な操作です。そこで、パッチングアルゴリズムを書いて、DOMの変更を最小限に抑えることができます:

```js
const patch = (dom, vdom, parent=dom.parentNode) => {
    const replace = parent ? el => (parent.replaceChild(el, dom) && el) : (el => el);
    if (typeof vdom == 'object' && typeof vdom.type == 'function') {
        return Component.patch(dom, vdom, parent);
    } else if (typeof vdom != 'object' && dom instanceof Text) {
        return dom.textContent != vdom ? replace(render(vdom, parent)) : dom;
    } else if (typeof vdom == 'object' && dom instanceof Text) {
        return replace(render(vdom, parent));
    } else if (typeof vdom == 'object' && dom.nodeName != vdom.type.toUpperCase()) {
        return replace(render(vdom, parent));
    } else if (typeof vdom == 'object' && dom.nodeName == vdom.type.toUpperCase()) {
        const pool = {};
        const active = document.activeElement;
        [/* flatten */].concat(...dom.childNodes).map((child, index) => {
            const key = child.__gooactKey || `__index_${index}`;
            pool[key] = child;
        });
        [/* flatten */].concat(...vdom.children).map((child, index) => {
            const key = child.props && child.props.key || `__index_${index}`;
            dom.appendChild(pool[key] ? patch(pool[key], child) : render(child, dom));
            delete pool[key];
        });
        for (const key in pool) {
            const instance = pool[key].__gooactInstance;
            if (instance) instance.componentWillUnmount();
            pool[key].remove();
        }
        for (const attr of dom.attributes) dom.removeAttribute(attr.name);
        for (const prop in vdom.props) setAttribute(dom, prop, vdom.props[prop]);
        active.focus();
        return dom;
    }
}
```

* 新しいVDOMを構築する
* 既存のDOMと再帰的に比較する
* 追加、削除、変更されたノードを特定する
* それらをパッチする

ただし、計算量の複雑さという別の問題が発生します。2つのツリーを比較するのは O(n³) の複雑さがあります。例えば、1000要素をパッチする場合、10億の比較が必要になります。これはあまりにも多すぎます。代わりに、2つの大きな仮定に基づいた開発的な O(n) アルゴリズムを実装します:

* 異なるタイプの2つの要素は、異なるツリーを生成する
* 開発者が「key」プロパティを使って、レンダリングが異なっても安定した子要素を示すことができる

実際のユースケースでは、この仮定はほとんど常に有効です。次のコードを見ていきましょう:

すべての組み合わせを調べます:

* **プリミティブVDOM + テキストDOM:** VDOMの値とDOM のテキストコンテンツを比較し、違いがあれば完全に再レンダリングする
* **プリミティブVDOM + 要素DOM:** 完全再レンダリング
* **複雑VDOM + テキストDOM:** 完全再レンダリング
* **複雑VDOM + 異なるタイプの要素DOM:** 完全再レンダリング 
* **複雑VDOM + 同じタイプの要素DOM:** 最も興味深い組み合わせ。子要素の調整が行われる部分です。
* **コンポーネントVDOM + 任意のDOM:** 前のセクションと同様に、別途処理され、後で実装します。

テキストノードと複雑なノードは一般に互換性がなく、完全な再レンダリングが必要です。ただし、これは比較的まれな変更です。興味深いのは子要素の調整です。これは以下のように行われます:

* 現在アクティブな要素がメモ化される - 調整によってフォーカスが乱れることがある
* DOMの子要素がそれぞれのキーでテンポラリプールに移動される - デフォルトではプレフィックス付きのインデックスがキーとして使われる
* VDOMの子要素がプールのDOMノードにキーでペアリングされ、再帰的にパッチされる - ペアが見つからない場合は新規レンダリング
* ペアリングされなかったDOMノードはドキュメントから削除される
* 最終的な親DOMに新しい属性が適用される
* 以前のアクティブ要素にフォーカスが戻される

3番目のライブプレイグラウンドは[**こちら**](https://jsbin.com/poyayij/edit?html,js,output)で公開されています。リストの調整の小さな例が含まれています。


## コンポーネント
**コンポーネント**は概念的にはJavaScriptの関数に似ています - 「props」と呼ばれる任意の入力を受け取り、画面に表示される要素のセットを返します。stateless関数で定義することもできますし、独自の内部状態やメソッド、ライフサイクルフックを持つ派生クラスで定義することもできます。理論は簡単にいきましょう - コードが語ります:

静的メソッドは内部的に呼び出されることを想定しています:
```js
class Component {
    constructor(props) {
        this.props = props || {};
        this.state = null;
    }

    static render(vdom, parent=null) {
        const props = Object.assign({}, vdom.props, {children: vdom.children});
        if (Component.isPrototypeOf(vdom.type)) {
            const instance = new (vdom.type)(props);
            instance.componentWillMount();
            instance.base = render(instance.render(), parent);
            instance.base.__gooactInstance = instance;
            instance.base.__gooactKey = vdom.props.key;
            instance.componentDidMount();
            return instance.base;
        } else {
            return render(vdom.type(props), parent);
        }
    }

    static patch(dom, vdom, parent=dom.parentNode) {
        const props = Object.assign({}, vdom.props, {children: vdom.children});
        if (dom.__gooactInstance && dom.__gooactInstance.constructor == vdom.type) {
            dom.__gooactInstance.componentWillReceiveProps(props);
            dom.__gooactInstance.props = props;
            return patch(dom, dom.__gooactInstance.render(), parent);
        } else if (Component.isPrototypeOf(vdom.type)) {
            const ndom = Component.render(vdom, parent);
            return parent ? (parent.replaceChild(ndom, dom) && ndom) : (ndom);
        } else if (!Component.isPrototypeOf(vdom.type)) {
            return patch(dom, vdom.type(props), parent);
        }
    }

    setState(nextState) {
        if (this.base && this.shouldComponentUpdate(this.props, nextState)) {
            const prevState = this.state;
            this.componentWillUpdate(this.props, nextState);
            this.state = nextState;
            patch(this.base, this.render());
            this.componentDidUpdate(this.props, prevState);
        } else {
            this.state = nextState;
        }
    }

    shouldComponentUpdate(nextProps, nextState) {
        return nextProps != this.props || nextState != this.state;
    }

    componentWillReceiveProps(nextProps) {
        return undefined;
    }

    componentWillUpdate(nextProps, nextState) {
        return undefined;
    }

    componentDidUpdate(prevProps, prevState) {
        return undefined;
    }

    componentWillMount() {
        return undefined;
    }

    componentDidMount() {
        return undefined;
    }

    componentWillUnmount() {
        return undefined;
    }
}
```

* **Render**: 初期レンダリングを行います。statelessコンポーネントは通常の関数として呼び出され、結果がすぐに表示されます。classコンポーネントはインスタンス化され、DOMに接続された後にレンダリングされます。
* **パッチング**: 更新を行います。既にDOMノードにコンポーネントインスタンスが接続されている場合は、新しいプロパティを渡して差分をパッチします。そうでない場合は完全な再レンダリングを行います。

インスタンスメソッドはユーザーが定義する派生クラスで上書きされたり、呼び出されることを想定しています:

* **コンストラクタ:** プロパティを処理し、初期状態を定義して自身に格納します。
* **状態修正:** 新しい状態を処理し、必要なライフサイクルフックを発火し、パッチサイクルを開始します。
* **ライフサイクルフック:** コンポーネントのライフサイクル中に呼び出される一連のメソッド - マウント時、更新時、削除直前など。

レンダリングメソッドは子クラスで定義されることを想定しています。最後のライブプレイグラウンドは[**こちら**](https://jsbin.com/moziles/edit?js%2Coutput=)で公開されています。これまで作成したすべてのコードと、シンプルなToDoアプリの例が含まれています。


## おわりに
これでみなさん、完全に機能するReactのクローンを作ることができました。私はこれを「Gooact」と呼びたいと思います - 私の親しい友人への小さな敬意を表したいと思います。結果を詳しく見ていきましょう:

* Gooactは、VDOMを参照してコンプレックスなDOMツリーを効率的に構築し、パッチ処理できます。
* Gooactは関数コンポーネントとclassコンポーネントの両方をサポートしており、適切な内部状態の管理と完全なライフサイクルフックセットが備わっています。
* Gooactは、Babelによって生成されたトランスパイルコードを使用します。
* Gooactは非圧縮のJavaScript160行で収まっています。

この記事の主な目的は、React内部構造のコアプリンシプルを深く掘り下げることなく示すことでした。そのため、いくつかの機能が Gooact には含まれていません:

* Gooactは、最新バージョンで導入されたフラグメント、ポータル、コンテキスト、参照などをサポートしていません。
* GooactはコンプレックスなReact Fiberを実装していません - 将来的にこれについて書くかもしれません。
* Gooactは重複キーを追跡しないため、時々バグが発生する可能性があります。
* Gooactは一部のメソッドの追加のコールバックをサポートしていません。

新しい機能と改善の余地がたくさんあるのがわかります。[**リポジトリはこちら**](https://github.com/sweetpalma/gooact)にありますので、ぜひフォークして実験してみてください。npmでもインストールできます!

素晴らしいライブラリを作ってくれたReactチームに感謝します。Preactの創造者であるJason Millerにも特別な敬意を表したいと思います - この記事はその極小主義的なアプローチに多くの影響を受けています。

この記事が気に入っていただけましたら、ぜひ共有やおすすめをお願いします。これからも同様の記事を書いていきたいと思います。ご覧いただき、ありがとうございました!

次の記事では、ブロックチェーンの内部構造についてさらに知りたい方向けに、「Goothereum: JavaScriptで160行のcryptocurrency」を書きます。