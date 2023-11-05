Repositório com o código-fonte para o TP1 da disciplina de Grafos.

Este repositório é um fork de
[`chistopher/arbok`](https://github.com/chistopher/arbok). Fizemos apenas
algumas alterações para facilitara compilação do projeto. Mais especificamente,
a troca do build system CMake por Bazel e a remoção dos _sources_ desnecessários
para o nosso trabalho, como a suite _lemon_.

### Execução básica

```sh
bazel run //apps:arbok_cli -- \
  -input "$(pwd)/data/peq.wsoap" -algo gabow
```

É necessário instalar o [Bazel](https://bazel.build/). Sugerimos a utilização da
ferramenta [bazelisk](https://github.com/bazelbuild/bazelisk).

Se você está no Windows, crie um arquivo `.bazelrc-win` com o seguinte conteúdo:

```
build --cxxopt='/std:c++20'
```

### Medição de performance

Compilar o medidor.

```sh
cargo build -p measure --release
# Binário estará em ./target/release/measure
```

Compilar a CLI:

```sh
bazel build -c opt //apps:arbok_cli
```

Executar o medidor, por exemplo:

```sh
./target/release/measure \
  -t test-girgs \
  --program-path="$(bazel cquery -c opt --output=files //apps:arbok_cli 2> /dev/null)" \
  -a atofigh -a gabow -a felerius 1> out.txt
```
