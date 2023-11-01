#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

#define QUANTIDADE_ALUNOS 15 // Quantidade de alunos
#define QUANTIDADE_ALUNOS_TIPO_1 (2*QUANTIDADE_ALUNOS)/3// Quantidade de alunos do tipo 1
#define QUANTIDADE_ALUNOS_TIPO_2 QUANTIDADE_ALUNOS/3// Quantidade de alunos do tipo 2

pthread_t alunos_1[QUANTIDADE_ALUNOS_TIPO_1];
pthread_t alunos_2[QUANTIDADE_ALUNOS_TIPO_2];
pthread_t campiolo;

sem_t entrada_alunos_tipo_1;//
sem_t entrada_alunos_tipo_2;//

pthread_mutex_t mutex_cadeira;
pthread_mutex_t mutex_entrada_alunos_tipo_1;
pthread_mutex_t mutex_decremento;

pthread_cond_t cadeira;
int finalizar_atividades=0;
int entrega_para_o_professor=0;
typedef enum TipoAtividade{
    ATIVIDADE_1=1,
    ATIVIDADE_2=2,
}TipoAtividade;

void inicializar_monitor(){
    pthread_mutex_init(&mutex_cadeira,NULL);
    pthread_mutex_init(&mutex_entrada_alunos_tipo_1,NULL);
    pthread_mutex_init(&mutex_decremento,NULL);
    pthread_cond_init(&cadeira,NULL);
    sem_init(&entrada_alunos_tipo_1,0,0);
    sem_init(&entrada_alunos_tipo_2,0,0);
}
void entrar_grupo(){
    printf("\n\nEntrar grupo da sala\n\n");
}
void sair_grupo(){
    printf("\n\nO grupo saiu da sala\n\n");
}
void destroy_monitor(){
    pthread_mutex_destroy(&mutex_cadeira);
    pthread_mutex_destroy(&mutex_decremento);
    pthread_mutex_destroy(&mutex_entrada_alunos_tipo_1);
    pthread_cond_destroy(&cadeira);
    sem_destroy(&entrada_alunos_tipo_1);
    sem_destroy(&entrada_alunos_tipo_2);
}
void receberAtividade(){
    printf("O Professor acabou de receber as atividades\n");
}

void filanizarEntrega(){
    printf("Finalizar entrega das atividade\n");
    exit(EXIT_SUCCESS);
}

void fazerAtividade(int id_aluno,TipoAtividade atividade){
    printf("O Aluno com id %d está fazendo a atividade do tipo %d\n",id_aluno,atividade);
    printf("O Aluno %d finalizou a atividade\n",id_aluno);
}

void aguardarEntrega(int id_aluno){
    printf("Aluno %d aguardando...\n",id_aluno);
}

void entrarSala(int id_aluno,TipoAtividade atividade){
    printf("O Aluno de id: %d com a atividade do tipo %d entrou na sala\n",id_aluno,atividade);
}

void entregarAtividade(int id_aluno,TipoAtividade atividade){
    printf("O Aluno de id %d entregou atividade de %d\n",id_aluno,atividade);
}

void sairSala(int id_aluno){
    printf("O Aluno %d saiu da sala\n",id_aluno);
}

void*aluno_tipo_1(void*args){
    int id=(int)args;
    TipoAtividade atividade_do_aluno=ATIVIDADE_1;
    fazerAtividade(id,atividade_do_aluno);
    aguardarEntrega(id);
    pthread_mutex_lock(&mutex_decremento);
    finalizar_atividades++;
    pthread_mutex_unlock(&mutex_decremento);
    if (finalizar_atividades==QUANTIDADE_ALUNOS){
        pthread_cond_signal(&cadeira);
    }
    sem_wait(&entrada_alunos_tipo_1);
    entrarSala(id,atividade_do_aluno);
    entregarAtividade(id,atividade_do_aluno);
    receberAtividade();
    sairSala(id);
    pthread_exit(NULL);
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
        pthread_cond_signal(&cadeira);
    }
    sem_wait(&entrada_alunos_tipo_2);
    entrarSala(id,atividade_do_aluno);
    entregarAtividade(id,atividade_do_aluno);
    receberAtividade();
    sairSala(id);
    pthread_exit(NULL);
}

void* professor(void* args){
    while (1){
        pthread_mutex_lock(&mutex_cadeira);
        while (finalizar_atividades<QUANTIDADE_ALUNOS){
            pthread_cond_wait(&cadeira,&mutex_cadeira);     
        }
        pthread_mutex_unlock(&mutex_cadeira);
        
        entrar_grupo();

        entrega_para_o_professor++; 
        sem_post(&entrada_alunos_tipo_1);         
        getchar();

        entrega_para_o_professor++;
        sem_post(&entrada_alunos_tipo_2);         
        getchar();
        
        entrega_para_o_professor++;
        sem_post(&entrada_alunos_tipo_1);
        getchar();

        sair_grupo();

        if (entrega_para_o_professor==QUANTIDADE_ALUNOS){
            filanizarEntrega();
        }
    }
}

int main(int argc, char const *argv[]){
    init_monitor();
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
    pthread_exit(&campiolo);
    pthread_exit(&alunos_1);
    pthread_exit(&alunos_2);
    destroy_monitor();
    return 0;
}
