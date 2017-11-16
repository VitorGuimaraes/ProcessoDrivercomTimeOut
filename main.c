//Implementação de um processo driver com timeout

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
 
//Vetor que armazena os votos e status das threads
int canal[6] = {-1, -1, -1, -1, -1, -1};
 
//Funcao de envio sincronizado e temporizado
int send_mes_sync_timeout(int *buff, int c, int t){
	time_t start_t, end_t;

	start_t = time(NULL);
	canal[c] = *buff;
	do{
		end_t = time(NULL);      
	}while(difftime(end_t, start_t) <= t && canal[c] != -1);
	if(canal[c] != -1)
		return 1;
	else
		return 0;
}

//Funcao de recepção temporizada
int receive_timeout(int *buff, int c, int t){
	time_t start_t, end_t;

	start_t = time(NULL);
	do{
		end_t = time(NULL);      
	}while(difftime(end_t, start_t) <= t && canal[c] == -1);
	if(canal[c] == -1)
		return 1;
	else{
		*buff = canal[c];
		canal[c] = -1;
		return 0;
	}
} 

void receive_mes(int *buff, int c){
   while(canal[c] == -1){}
   *buff = canal[c];
   canal[c] = -1;
   return;
} 

//Funcao de espera seletiva
int alt_wait(int q, int vetor[]){
	
	while(1){
		for(int i = 0; i < q; i++){
			if(vetor[i] != -1)
				return i;		
		}
	}
	return 0;
}

void *run_A(void *thread_id){
	char *name = thread_id;  
	int status, vote = 1, i, x = 1;
 
	while((i <= 2) && (x != 0)){
		i++;
		x = send_mes_sync_timeout(&vote, 0, 5);
	}

	if(x != 0){
		printf("Processo destino falhou\n");
	}

	x = receive_timeout(&status, 3, 5);
	
	if(x == 1){
		printf("Processo driver falhou\n");
	}
	else{
		if(status == 0){
			printf("Thread A: Voto = %d, incorreto!\n", vote);
			while(1);
		}
		else{
			printf("Thread A: Voto = %d, correto!\n", vote);
		}
	}
}
 
void *run_B(void *thread_id){
	char *name = thread_id;
	int status, vote = 1, i, x = 1;
 
	while((i <= 2) && (x != 0)){
		i++;
		x = send_mes_sync_timeout(&vote, 1, 5);
	}

	if(x != 0){
		printf("Processo destino falhou\n");
	}

	x = receive_timeout(&status, 4, 5);
	
	if(x == 1){
		printf("Processo driver falhou\n");
	}
	else{
		if(status == 0){
			printf("Thread B: Voto = %d, incorreto!\n", vote);
			while(1);
		}
		else{
			printf("Thread B: Voto = %d, correto!\n", vote);
		}
	}
}
 
void *run_C(void *thread_id){
	char *name = thread_id;
		int status, vote = 2, i, x = 1;
 
	while((i <= 2) && (x != 0)){
		i++;
		x = send_mes_sync_timeout(&vote, 2, 5);
	}

	if(x != 0){
		printf("Processo destino falhou\n");
	}

	x = receive_timeout(&status, 5, 5);
	
	if(x == 1){
		printf("Processo driver falhou\n");
	}
	else{
		if(status == 0){
			printf("Thread C: Voto = %d, incorreto!\n", vote);
			while(1);
		}
		else{
			printf("Thread C: Voto = %d, correto!\n", vote);
		}
	}
}
 
void compara(){ 
	//Variável para receber os valores dos votos que estão armazenados no canal
	int voto[3];
	
	int correto;
	int voto_correto = 1;
	int voto_errado = 0;
	int x;
 
 	//Armazena os votos que estao no canal[c] no vetor local voto[3] usando espera seletiva
	for(int i = 0; i < 3; i++){
		x = alt_wait(3, canal);
		receive_mes(&voto[x], x);
	}

	if((voto[0] == voto[1]) && (voto[0] == voto[2])){
		correto = voto[0];
		send_mes_sync_timeout(&voto_correto, 3, 5); //envia o status para a thread
		send_mes_sync_timeout(&voto_correto, 4, 5); //envia o status para a thread
		send_mes_sync_timeout(&voto_correto, 5, 5); //envia o status para a thread
	}
 
	else if(voto[0] == voto[1]){
		correto = voto[0];
		send_mes_sync_timeout(&voto_correto, 3, 5); //envia o status para a thread
		send_mes_sync_timeout(&voto_correto, 4, 5); //envia o status para a thread
		send_mes_sync_timeout(&voto_errado, 5, 5); //envia o status para a thread
	}
 
	else if(voto[0] == voto[2]){
		correto = voto[0];
		send_mes_sync_timeout(&voto_correto, 3, 5); //envia o status para a thread
		send_mes_sync_timeout(&voto_errado, 4, 5); //envia o status para a thread
		send_mes_sync_timeout(&voto_correto, 5, 5); //envia o status para a thread
	}
 
	else if(voto[1] == voto[2]){
		correto = voto[1];
		send_mes_sync_timeout(&voto_errado, 3, 5); //envia o status para a thread
		send_mes_sync_timeout(&voto_correto, 4, 5); //envia o status para a thread
		send_mes_sync_timeout(&voto_correto, 5, 5); //envia o status para a thread
	}
 
	printf("Voto majoritario = %d\n", correto);
}
 
void *run_driver(void *thread_id){
	char *name = thread_id;
 
	pthread_t t1, t2, t3;
 
	pthread_create(&t1, NULL, run_A, (void *)"A");
	pthread_create(&t2, NULL, run_B, (void *)"B");
	pthread_create(&t3, NULL, run_C, (void *)"C");
 
	sleep(1);
	
	compara();
}
 
int main(){
	pthread_t driver;
 
	pthread_create(&driver, NULL, run_driver, (void *)"Z");
 
	pthread_exit(NULL);
	return 0; 
}
 
