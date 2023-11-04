Repositório com o código-fonte para o TP1 da disciplina de Grafos.

Este repositório é um fork de
[`chistopher/arbok`](https://github.com/chistopher/arbok). Fizemos apenas
algumas alterações para facilitara compilação do projeto. Mais especificamente,
a troca do build system CMake por Bazel e a remoção dos _sources_ desnecessários
para o nosso trabalho, como a suite _lemon_.

Executar:

```sh
bazel run //apps:arbok_cli -- \
  -input "$(pwd)/data/peq.wsoap" -algo gabow
```

É necessário instalar o [Bazel](https://bazel.build/). Sugerimos a utilização da
ferramenta [bazelisk](https://github.com/bazelbuild/bazelisk).
