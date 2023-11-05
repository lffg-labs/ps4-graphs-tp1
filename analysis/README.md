# Sobre o medidor

O programa responsável por invocar a CLI disponibilizada no repositório
[`chistopher/arbok`][arbok] foi codificado na linguagem de programação [Rust].
Sua interface é simples e utilizada por nós para executar um conjunto de testes
para uma lista de algoritmos. Nas seções a seguir alguns exemplos de uso estão
em demonstração.

Como os grafos utilizados durante os testes são grandes, e os algoritmos
disponibilizados no repositório [`chistopher/arbok`][arbok] são programados de
modo a tirar vantagem de um único core por execução, consideramos oportuno fazer
com que nosso medidor fosse programado de modo a executar diversos testes em
paralelo, de modo a utilizar totalmente os recursos computacionais disponíveis
para otimizar o tempo de execução.

A excelente biblioteca [Rayon] foi utilizada para a criação de um programa para
medir as performances de modo paralelo. A interface da biblioteca, que garante
paralelismo livre de _race conditions_, é extremamente simples para uso pois
fornece uma interface similar a de iteradores seriais, que são então
paralelizados utilizando uma abordagem de work-stealing. [Mais
detalhes.][rayon-more]

[Rust]: https://rust-lang.org/
[Rayon]: https://github.com/rayon-rs/rayon
[rayon-more]:
  https://smallcultfollowing.com/babysteps/blog/2015/12/18/rayon-data-parallelism-in-rust/

# Girgs

Não utilizamos o de Girgs completo porque o tamanho das entradas eram diferente
entre eles.

Utilizamos os conjuntos Girgs-200, Girgs-400, Girgs-600 e Girgs-800.

Os grafos de testes foram utilizados pelo gerador disponibilizado no repositório
[`chistopher/girgs`][girgs-gen] e posteriormente processados pelos scripts do
repositório [`chistopher/arbok`][arbok] ([permalink para instruções no
Makefile][make-perm]).

[girgs-gen]: https://github.com/chistopher/girgs
[arbok]: https://github.com/chistopher/arbok
[make-perm]:
  https://github.com/chistopher/arbok/blob/d5ccef7667a11af110de772f3aaaea9d5e17616f/data/Makefile#L42-L44

Comando utilizado:

```bash
$ cargo build -p measure --release
$ bazel build -c opt //apps:arbok_cli
$ ./target/release/measure \
  -t test-girgs/girgs-0200 -t test-girgs/girgs-0400 -t test-girgs/girgs-0600 -t test-girgs/girgs-0800 \
  --program-path="bazel-out/darwin-opt/bin/apps/arbok_cli" \
  -a atofigh -a gabow -a felerius 1> test-girgs-out.json
```

Resultados da medição, em microssegundos.

```json
{
  "girgs-0800": {
    "Atofigh": [13084705, 11801264, 11433748, 13211209, 13336448],
    "Felerius": [6430860, 7433342, 6722569, 6896307, 6349473],
    "Gabow": [7724668, 7690026, 7220417, 7709098, 7315238]
  },
  "girgs-0400": {
    "Gabow": [
      3710436, 3794897, 4346602, 3703193, 3738638, 3714605, 3683849, 4108895
    ],
    "Felerius": [
      4622312, 4638410, 3406063, 4627250, 4670426, 4666088, 4713675, 3702714
    ],
    "Atofigh": [
      10464015, 10271334, 7615494, 10581804, 10502407, 10465151, 10495880,
      8995430
    ]
  },
  "girgs-0600": {
    "Atofigh": [
      12646490, 10666011, 15164340, 15152403, 14449166, 14967886, 14301485,
      11514079, 15277011, 14332798
    ],
    "Felerius": [
      5740997, 5740197, 6126164, 6137163, 6131213, 6157212, 5210221, 5730719,
      6157119, 6098937
    ],
    "Gabow": [
      7844752, 7817540, 5728694, 7851793, 5598417, 8014141, 5778602, 7734299,
      7966652, 5576363
    ]
  },
  "girgs-0200": {
    "Atofigh": [
      872886, 5372307, 5779832, 5979841, 5039225, 3453434, 5937864, 6050343,
      5425711, 5103444
    ],
    "Felerius": [
      1525803, 888796, 1403541, 1538864, 1322134, 1526688, 1055309, 1537072,
      1336697, 1141491
    ],
    "Gabow": [
      1634798, 1545453, 1681981, 1625870, 1630674, 1423675, 1547975, 651278,
      1559958, 1631740
    ]
  }
}
```
