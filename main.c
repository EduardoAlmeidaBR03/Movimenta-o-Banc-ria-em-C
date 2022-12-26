#include <stdio.h>
#include <stdlib.h>
struct clienteConta {
  unsigned int conta;
  char nome[15];
  char sobrenome[10];
  double saldo;
  double saque;
  double deposito;
  double limite;
};

struct transacoes {
  unsigned int conta;
  char tipo;
  double valor;
  double saldo;
};

unsigned int escolha(void);
void novoRegistro(FILE *fPtr);
void arquivoTexto(FILE *readPtr);
void alteraRegistro(FILE *fPtr, FILE *movimentacaoPtr);
void apagaRegistro(FILE *fPtr);
void extrato(FILE *readPtr, FILE *movimentacaoPtr);

int main(void) {
  FILE *cfPtr;
  FILE *movimentacaoPtr;

  if ((cfPtr = fopen("contas.dat", "rb+")) == NULL ||
      (movimentacaoPtr = fopen("movimentacao.dat", "rb+")) == NULL) {
    puts("não deu para abrir.");
  } else {
    unsigned int opcao; // opção do usuário
    while ((opcao = escolha()) != 6) {
      switch (opcao) {
      case 1:
        novoRegistro(cfPtr);
        break;
      case 2:
        alteraRegistro(cfPtr, movimentacaoPtr);
        break;
      case 3:
        arquivoTexto(cfPtr);
        break;
      case 4:
        apagaRegistro(cfPtr);
        break;
      case 5:
        extrato(movimentacaoPtr, cfPtr);
        break;
      default:
        puts("Opção Inválida");
        break;
      }
    }
    fclose(cfPtr);
    fclose(movimentacaoPtr);
  }
}

void extrato(FILE *movimentacaoPtr, FILE *readPtr) {

  FILE *extratotxt;
  extratotxt = fopen("extrato.txt", "w");

  unsigned int conta;
  puts("Conta para visualizar (1 - 100): ");
  scanf("%d", &conta);
  fseek(readPtr, (conta - 1) * sizeof(struct clienteConta), SEEK_SET);
  // cria conta default

  struct clienteConta cliente = {0, "", "", 0.0, 0.0};
  fread(&cliente, sizeof(struct transacoes), 1, readPtr);
  // escreve um registro em um arquivo texto
  fprintf(extratotxt, "Nome do cliente: %-10s\n", cliente.nome);
  fprintf(extratotxt, "Conta |Tipo     |Valor    |Saldo\n");
  rewind(movimentacaoPtr);
  while (!feof(movimentacaoPtr)) {
    // cria conta default
    struct transacoes trans = {0, ' ', 0.0, 0.0};
    fread(&trans, sizeof(struct transacoes), 1, movimentacaoPtr);

    // escreve um registro em um arquivo texto

    if (conta == trans.conta) {
      fprintf(extratotxt, "%-6d|%-9c|%-9.2f|%-9.2f\n", trans.conta, trans.tipo,
              trans.valor, trans.saldo);
    }
  }
  fclose(extratotxt);
}

// cria arquivo de texto formatado
void arquivoTexto(FILE *readPtr) {
  FILE *txtPtr;
  if ((txtPtr = fopen("contas.txt", "w")) == NULL) {
    puts("Não é possível ler");
  } else {
    rewind(readPtr); // aponta para o início do arquivo
    fprintf(txtPtr, "%-5s|%-15s|%-10s|%-9s|%-10s\n", "Conta", "Nome",
            "Sobrenome", "Saldo", "Limite");
    fprintf(txtPtr, "-------------------------------------------------\n");

    while (!feof(readPtr)) {
      // cria conta default
      struct clienteConta cliente = {0, "", "", 0.0, 0.0};
      int result = fread(&cliente, sizeof(struct clienteConta), 1, readPtr);
      // escreve um registro em um arquivo texto
      if (result != 0 && cliente.conta != 0) {
        fprintf(txtPtr, "%5d|%-15s|%-10s|%-9.2f|%-9.2f\n", cliente.conta,
                cliente.nome, cliente.sobrenome, cliente.saldo, cliente.limite);
        fprintf(txtPtr, "-------------------------------------------------\n");
      }
    }
    fclose(txtPtr);
  }
}

// Altera um registro
void alteraRegistro(FILE *fPtr, FILE *movimentacaoPtr) {
  unsigned int conta;
  puts("Conta para alterar (1 - 100): ");
  scanf("%d", &conta);
  fseek(fPtr, (conta - 1) * sizeof(struct clienteConta),
        SEEK_SET); // determina onde vai o ponteiro
  struct clienteConta cliente = {0, "", "", 0.0, 0.0};
  struct transacoes trans = {0, ' ', 0, 0};
  fread(&cliente, sizeof(struct clienteConta), 1, fPtr);
  fread(&trans, sizeof(struct transacoes), 1, movimentacaoPtr);
  trans.conta = cliente.conta;

  if (cliente.conta == 0) {
    printf("conta #%d sem informação.\n", conta);
  } else { // altera
    printf("%-6d%-16s%-11s%-10.2f%-10.2f\n", cliente.conta, cliente.nome,
           cliente.sobrenome, cliente.saldo, cliente.limite);

    // faz transação
    printf("%s", "Escolha a operação Débito(1) ou Saque(2): ");
    int operacao; // transacao amount
    double deposito;
    double saque;
    scanf("%d", &operacao);
    if (operacao == 1) {
      puts("Digite o valor que deseja adicionar");
      scanf("%lf", &deposito);
      cliente.saldo += deposito; // update record saldo
      trans.tipo = 'D';
      trans.valor = deposito;
      trans.saldo = cliente.saldo;

    } else if (operacao == 2) {
      puts("Digite o valor que deseja retirar");
      scanf("%lf", &saque);
      if (saque > cliente.limite + cliente.saldo) {
        puts("limite extrapolado\n");
        puts("Deseja extrapolar o limite?\n"); 
        puts(" Sim(1) Não(2)\n"); 
        int escolha;
        scanf("%d", &escolha); 
        if(escolha == 1){ 
          cliente.saldo -= saque;
        trans.tipo = 'S';
        trans.valor = saque;
        trans.saldo = cliente.saldo;
          }else{ 
          
          }
          
      } else {
        cliente.saldo -= saque;
        trans.tipo = 'S';
        trans.valor = saque;
        trans.saldo = cliente.saldo;
      }
    }
    

    printf("\n%-6d%-16s%-11s%-10.2f%-10.2f\n", cliente.conta, cliente.nome,
           cliente.sobrenome, cliente.saldo, cliente.limite);

    // move ponteiro para a posição

    fseek(fPtr, (conta - 1) * sizeof(struct clienteConta), SEEK_SET);

    // atualiza
    fwrite(&cliente, sizeof(struct clienteConta), 1, fPtr);
    fwrite(&trans, sizeof(struct transacoes), 1, movimentacaoPtr);
  }
}

// apaga um registro
void apagaRegistro(FILE *fPtr) {
  // obtendo conta para apagar
  printf("%s", "Conta para apagar (1 - 100): ");
  unsigned int contaNum;
  scanf("%d", &contaNum);
  // movimenta o ponteiro para o registro específico
  fseek(fPtr, (contaNum - 1) * sizeof(struct clienteConta), SEEK_SET);
  struct clienteConta cliente; // stores record read from file
  // lê registro do arquivo
  fread(&cliente, sizeof(struct clienteConta), 1, fPtr);
  if (cliente.conta == 0) {
    printf("\nA conta %d não existe.\n", contaNum);
  } else { // delete record
    fseek(fPtr, (contaNum - 1) * sizeof(struct clienteConta), SEEK_SET);
    struct clienteConta clienteVazio = {0, "", "", 0.0};
    // altera
    fwrite(&clienteVazio, sizeof(struct clienteConta), 1, fPtr);
  }
}

// cria e insere novo registro
void novoRegistro(FILE *fPtr) {
  printf("%s", "Entre com o número da conta (1 - 100): ");
  unsigned int contaNum;
  scanf("%d", &contaNum);
  fseek(fPtr, (contaNum - 1) * sizeof(struct clienteConta), SEEK_SET);
  struct clienteConta cliente = {0, "", "", 0.0};
  cliente.saldo = 0;

  fread(&cliente, sizeof(struct clienteConta), 1, fPtr);
  if (cliente.conta != 0) {
    printf("Conta #%d já existe.\n", cliente.conta);
  } else { // Cria registro
    printf("%s", "Entre com nome, sobrenome, limite?\n ");
    scanf("%14s%9s%lf", cliente.nome, cliente.sobrenome, &cliente.limite);
    cliente.conta = contaNum;
    fseek(fPtr, (cliente.conta - 1) * sizeof(struct clienteConta), SEEK_SET);
    // insere
    fwrite(&cliente, sizeof(struct clienteConta), 1, fPtr);
  }
}

// Menu de opções
unsigned int escolha(void) {
  unsigned int opcao;
  printf("1 - Criar conta\n"
         "2 - Depósito e Saque\n"
         "3 - Salvar dados no contas.txt\n"
         "4 - Apagar conta\n"
         "5 - Extrato\n"
         "6 - Fim do programa?\n ");
  scanf("%u", &opcao); // receive opcao from user
  system("clear");
  return opcao;
}
