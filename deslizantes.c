#include <stdio.h>
#include <stdlib.h>

int main() {
    int numeros[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int tam_janela = 3;
    int tam_numeros = 10;
    int erros;
    int *err;

    //matriz de duas dimensoes, 1 representa a mensagem e a 2 indica se a mensagem foi corrigida ou nao
    int ack[tam_numeros][2];
    for (int i = 0; i < tam_numeros; i++) {
        ack[i][0] = i;
        ack[i][1] = 0;
    }
    //le erros
    printf("Digite o numero de erros: ");
    scanf("%d", &erros);
    err = (int *) malloc(erros * sizeof(int));
    printf("Digite a numeracao dos erros das mensagens: ");
    for (int i = 0; i < erros; i++){
        scanf("%d", &err[i]);
    }
    //inicializa com zeros
    int qtderr[tam_numeros];
    for (int i = 0; i < tam_numeros; i++){
        qtderr[i] = 0;
    }
    //vetor qtderr recebe os erros e armazena no veto
    for (int i = 0; i < erros; i++){
        qtderr[err[i]]++;
    }

    int i = 0;
    //janela deslizante volta-n
    while (i < tam_numeros){
        int j = i;
        int janela_corrigida = 0;
        //anda na janela
        for (; j < i + tam_janela && j < tam_numeros; j++){
            //apos verificar que nao ha erro na msg mudar o valor na matriz
            if (qtderr[j] == 0)
                ack[j][1] = 1;
            //caso contrario ira ajustar a janela
            else{
                qtderr[j]--;
                i = j;
                janela_corrigida = 1;
                break;
            }
        }
        if (!janela_corrigida)
            i += tam_janela;
    }

    printf("Resultado:\n");
    for (int i = 0; i < tam_numeros; i++) {
        printf("%d ", ack[i][1]);
    }
    printf("\n");

    return 0;
}