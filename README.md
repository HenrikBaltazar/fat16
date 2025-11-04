# FAT16
## Implementação de Funções de Manipulação de Sistema de Arquivos FAT16

### Execucao
Executar o binario fat16 disponivel na raiz deste repositorio. Discos .img disponiveis no diretorio `disk_images`

### Objetivo
Neste trabalho vocês deverão desenvolver um programa em uma linguagem de programação de sua
escolha para manipular um sistema de arquivos FAT16. O objetivo principal é implementar operações
básicas para gerenciamento de arquivos, possibilitando, por exemplo, a criação, leitura, escrita e
remoção de arquivos. Observação: por questões de simplicidade, não estaremos considerando a
criação e manipulação de subdiretórios.

---

### Requisitos
Implementação de Operações Fundamentais: O programa deve ser capaz de realizar as seguintes
operações básicas sobre o sistema de arquivos FAT16:
- [x] **Listar o conteúdo do disco (10%):** exibir em uma lista os nomes dos arquivos (e seus respectivos tamanhos)
  existentes no diretório raiz.
- [x] **Listar o conteúdo de um arquivo (10%):** mostrar (pode ser na tela) o conteúdo de um arquivo do diretório
  raiz.
- [x] **Exibir os atributos de um arquivo (10%):** mostrar data/hora da criação/última modificação e os seguintes
  atributos: se é somente leitura; se é oculto; se é arquivo de sistema
- [x] **Renomear um arquivo (10%):** trocar o nome de um arquivo existente
- [x] **Inserir/criar um novo arquivo (40%):** permitir que se armazene no diretório raiz um novo arquivo
  externo.
- [x] **Apagar/remover um arquivo (20%):** apagar um arquivo do diretório raiz.

---

*Todas as operações devem ser realizadas sobre um arquivo que contenha uma imagem de disco
formatado como FAT16 (um exemplo de arquivo está disponibilizado no AVA Univali). O programa
deve ser capaz de ler essa imagem, interpretar sua estrutura e realizar as operações de acordo com as
especificações do sistema de arquivos FAT16. Observação: a solução dada deverá funcionar para
qualquer arquivo contendo imagem de disco no formato FAT16, sem considerar a existência de
subdiretórios.*


***Cada uma das operações será avaliada de acordo com a sua corretude:** As operações devem ser
implementadas de forma correta, seguindo as especificações do sistema de arquivos FAT16.*
