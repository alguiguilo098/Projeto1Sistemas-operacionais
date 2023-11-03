#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
//Autores: Ingrid Reupke Sbeguen Moran RA:2349388,Guilherme Almeida Lopes RA:2458802,Caio rangel ferreira rodrigues RA:2252716
// Data de criação: 30.10.2023
// Data de atualização: 2.11.2023

/*
    O exercício proposto envolve a criação de um sistema concorrente.
    O sistema inclui o Professor, Alunos de SO com atividade 1 e Alunos de SO com atividade 2.
    A proporção de alunos é tal que 2/3 são do tipo 1 e 1/3 são do tipo 2.
    Somente um grupo formado por 2 alunos do tipo 1 e 1 aluno do tipo 2 pode entrar na sala do professor por vez.
    As regras incluem restrições para entrada na sala do professor, formação de grupos e entrega de atividades. 
*/

#define QUANTIDADE_ALUNOS 12// Quantidade de alunos -> altere esse valor para mudar a quantidade de alunos e realizar o testes 
#define QUANTIDADE_ALUNOS_TIPO_1 (2*QUANTIDADE_ALUNOS)/3// Quantidade de alunos do tipo 1
#define QUANTIDADE_ALUNOS_TIPO_2 QUANTIDADE_ALUNOS/3// Quantidade de alunos do tipo 2

pthread_t alunos_1[QUANTIDADE_ALUNOS_TIPO_1];//thread do aluno 1
pthread_t alunos_2[QUANTIDADE_ALUNOS_TIPO_2];//thread dos alunos 2
pthread_t campiolo;//thread do professo 

sem_t entrada_alunos_tipo_1;// semaforos do alunos do tipo 1
sem_t entrada_alunos_tipo_2;// semaforo do alunos do tipo 2

pthread_mutex_t mutex_cadeira;// mutex para controlar a variável de condição
pthread_mutex_t mutex_entrada_alunos_tipo_1;// mutex para garantir a exclusão mutua do semáforo 1
pthread_mutex_t mutex_entrada_alunos_tipo_2;// mutex para garantir a exclusão mutua do semáforo 2
pthread_mutex_t mutex_decremento;// mutex para garantir a exclusão mutua da variaveis de decremento

pthread_cond_t cadeira;// variavel de condição, faz o professor thread professor dormir
int finalizar_atividades=0;// Quantidade de atividades entregas
int entrega_para_o_professor=0;// Quantidade entregas pelo professor
typedef enum TipoAtividade{
    ATIVIDADE_1=1,
    ATIVIDADE_2=2,
}TipoAtividade;

//Impriem no terminal que um grupo entrou na sala
void entrar_grupo(){
    printf("\n\nEntrar grupo da sala\n\n");
}

//imprime no terminal quando um grupo saiu da sala
void sair_grupo(){
    printf("\n\nO grupo saiu da sala\n\n");
}

// imprime a mesagem que uo professor recebeu a atividade
void receberAtividade(){
    printf("O Professor acabou de receber as atividades\n");
}
//Finaliza  a entrega das atividades
void filanizarEntrega(){
    printf("Finalizar entrega das atividade\n");
    exit(EXIT_SUCCESS);// finaliza o processo, terminado a execução
}

//impprime o id do aluno e a atividade que o aluno está fazendo
void fazerAtividade(int id_aluno,TipoAtividade atividade){
    // id_aluno-> valor inteiro positivo que representa o aluno
    // Tipo da atividade-> valor inteiro pode ser 1 ou 2
    printf("O Aluno com id %d está fazendo a atividade do tipo %d\n",id_aluno,atividade);
    printf("O Aluno %d finalizou a atividade\n",id_aluno);
}
//imprime a mensagem que o aluno está aguardando
void aguardarEntrega(int id_aluno){
    // id_aluno-> valor inteiro positivo que representa o aluno
    printf("Aluno %d aguardando...\n",id_aluno);
}
//imprime a mensagem que o aluno entrou na srecurso comparala
void entrarSala(int id_aluno,TipoAtividade atividade){
    // id_aluno-> valor inteiro positivo que representa o aluno
    // Tipo da atividade-> valor inteiro pode ser 1 ou 2
    printf("O Aluno de id: %d com a atividade do tipo %d entrou na sala\n",id_aluno,atividade);
}
//imprime a mensagem que o aluno entregou a atividade para o professor
void entregarAtividade(int id_aluno,TipoAtividade atividade){
    // id_aluno-> valor inteiro positivo que representa o aluno
    // Tipo da atividade-> valor inteiro pode ser 1 ou 2
    printf("O Aluno de id %d entregou atividade de %d\n",id_aluno,atividade);
}
// imprime a mesagem que o aluno saiu da sala
void sairSala(int id_aluno){
    // id_aluno-> valor inteiro positivo que representa o aluno
    printf("O Aluno %d saiu da sala\n",id_aluno);
}

void*aluno_tipo_1(void*args){
    int id=(int)args;//ID do aluno
    TipoAtividade atividade_do_aluno=ATIVIDADE_1;// atividade
    fazerAtividade(id,atividade_do_aluno);
    aguardarEntrega(id);
    pthread_mutex_lock(&mutex_decremento);// entra na região crítica
    finalizar_atividades++;
    pthread_mutex_unlock(&mutex_decremento);// sai da região crítica
    if (finalizar_atividades==QUANTIDADE_ALUNOS){
        // Quando a ultima aluno termina a atividade 
        //libera a thread do professor
        pthread_cond_signal(&cadeira);
    }
    pthread_mutex_lock(&mutex_entrada_alunos_tipo_1); // entrada na região crítica
    sem_wait(&entrada_alunos_tipo_1);// faz com que todas as threads fiquem suspensas
    pthread_mutex_unlock(&mutex_entrada_alunos_tipo_1);// saída daa região crítica
    entrarSala(id,atividade_do_aluno);
    entregarAtividade(id,atividade_do_aluno);
    receberAtividade();
    sairSala(id);
    pthread_exit(NULL);//finaliza a thread
}

void*aluno_tipo_2(void*args){
    int id=(int)args;
    TipoAtividade atividade_do_aluno=ATIVIDADE_2;
    fazerAtividade(id,atividade_do_aluno);
    aguardarEntrega(id);
    pthread_mutex_lock(&mutex_decremento);
    finalizar_atividades++;
    pthread_mutex_unlock(&mutex_decremento);
    if (finalizar_atividades==QUANTIDADE_ALUNOS){
        //libera a thread professor quando 
        pthread_cond_signal(&cadeira);
    }
    pthread_mutex_lock(&mutex_entrada_alunos_tipo_2);// entra na região crítica
    sem_wait(&entrada_alunos_tipo_2);// suspende a thread alunos do tipo 2
    pthread_mutex_unlock(&mutex_entrada_alunos_tipo_2);//sai da região crítica
    entrarSala(id,atividade_do_aluno);
    entregarAtividade(id,atividade_do_aluno);
    receberAtividade();
    sairSala(id);
    pthread_exit(NULL);//finaliza a thread 
}

void* professor(void* args){
    while (1){
        pthread_mutex_lock(&mutex_cadeira);
        while (finalizar_atividades<QUANTIDADE_ALUNOS){
            // suspende o professor até todos os alunos
            //finalizarem as atividades 
            pthread_cond_wait(&cadeira,&mutex_cadeira);     
        }
        pthread_mutex_unlock(&mutex_cadeira);
        
        entrar_grupo();

        entrega_para_o_professor++; 
        sem_post(&entrada_alunos_tipo_1);         
        getchar();// pressione enter para continuar a execução

        entrega_para_o_professor++;
        sem_post(&entrada_alunos_tipo_2);         
        getchar();// pressione enter para continuar a execução
        
        entrega_para_o_professor++;
        sem_post(&entrada_alunos_tipo_1);
        getchar();// pressione enter par continuar a execução

        sair_grupo();

        if (entrega_para_o_professor==QUANTIDADE_ALUNOS){
            //finaliza o programa quando todos os alunos entregarem
            // as atividades
            filanizarEntrega();
        }
    }
}

int main(int argc, char const *argv[]){
    // inicializa os mutex
    pthread_mutex_init(&mutex_cadeira,NULL);
    pthread_mutex_init(&mutex_entrada_alunos_tipo_1,NULL);
    pthread_mutex_init(&mutex_entrada_alunos_tipo_2,NULL);
    pthread_mutex_init(&mutex_decremento,NULL);
    // inicializa a variaveis de condição
    pthread_cond_init(&cadeira,NULL);

    // inicializa o semáforo
    sem_init(&entrada_alunos_tipo_1,0,0);// inicializa o semaforo comm count=0
    sem_init(&entrada_alunos_tipo_2,0,0);// inicializa o semaforo com cont=0

    pthread_create(&campiolo,NULL,professor,NULL);
    int j=0;
    for (int i = 0; i <QUANTIDADE_ALUNOS/3; i++){
        pthread_create(&alunos_1[i],NULL,aluno_tipo_1,(void*)j);
        j++;
        pthread_create(&alunos_1[i+1],NULL,aluno_tipo_1,(void*)(j));
        j++;
        pthread_create(&alunos_2[i],NULL,aluno_tipo_2,(void*)(j));
        j++;
    }
    // espera todas as threads finalizarem
    pthread_exit(&campiolo);
    pthread_exit(&alunos_1);
    pthread_exit(&alunos_2);

    //destroi todos os mutex
    pthread_mutex_destroy(&mutex_cadeira);
    pthread_mutex_destroy(&mutex_decremento);
    pthread_mutex_destroy(&mutex_entrada_alunos_tipo_1);
    pthread_mutex_destroy(&mutex_entrada_alunos_tipo_2);

    // destroi a váriavel de condição
    pthread_cond_destroy(&cadeira);

    // destroi os semáforos
    sem_destroy(&entrada_alunos_tipo_1);
    sem_destroy(&entrada_alunos_tipo_2);

    return 0;
}
