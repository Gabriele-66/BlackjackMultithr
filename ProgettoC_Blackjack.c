#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>					

#define LUNGHEZZA_BUFFER 10
#define N_CARTE 52						//Numero di carte che compone ogni mazzo
#define perc '%'						//Carattere percentuale

void *producer();
void *consumer1();
void *consumer2();

pthread_mutex_t r3_mutex=PTHREAD_MUTEX_INITIALIZER;

int buffer[LUNGHEZZA_BUFFER];
int in = 0;								//indice di buffer che punta alla prossima posizione libera per produrre
int out = 0;							//indice di buffer che punta alla prossima posizione per il consumo dei valori
int num_elementi_presenti_buffer = 0;

int elemento_consumato = 0;
int elemento_appena_prodotto = 0;

int mazzi = 0;

int num_elementi_consumati = 0;  					
int num_elementi_prodotti = 0;						

int truecont1 = 0;						//calcolo aritmetico del runningCount secondo il metodo del Hi-Low
int truecont2 = 0;	

float prob1 = 0;						//probabilità di vincita associata al giocatore 1
float prob2 = 0;						//probabilità di vincita associata al giocatore 2 (mazziere)

int differenza = 0;						//numero di carte rimaste nel mazzo espresse in mazzi


int conteggio(int carta,int truecont){		

	if (carta < 7 ) truecont ++;
	if (carta > 9 ) truecont --;

	return truecont;
}

float calcolaprob(int truecont){
	
	float prob = 0;	
	
	differenza = (N_CARTE * mazzi - num_elementi_consumati)/N_CARTE;
	
	if (truecont == 0 || differenza == 0)	return 0;		//se il calcolo aritmetico risulta nullo o non ci sono carte nel mazzo la probabilità è nulla
	
	prob = (float)truecont/differenza;
	return prob;	
}

void produci_elementi(){
	
	elemento_appena_prodotto = 1+rand()%13;						//genera num.casuale tramite la funzione rand() da 1 a 13
	printf("\nPRODUCER: %d", elemento_appena_prodotto);
    buffer[in] = elemento_appena_prodotto;						//si inserisce nel buffer l'elemento (valore) appena prodotto dal producer
    in = (in + 1) % LUNGHEZZA_BUFFER;							//contatore circolare basato sul resto.
    num_elementi_presenti_buffer++;
}

void consuma_elementi(int num){		
	
	elemento_consumato = buffer[out];
 	out = (out + 1) % LUNGHEZZA_BUFFER;
   	num_elementi_presenti_buffer--;
	printf("\nGIOCATORE %d, RICEVE: %d",num ,elemento_consumato);
    truecont1 = conteggio(elemento_consumato,truecont1);
    num_elementi_consumati ++;
    
}

int main() {

	printf("Inserire il numero di mazzi:");
	scanf("%d",&mazzi);
	
    pthread_t thread1, thread2, thread3;
    if (pthread_create(&thread1, NULL,
                        producer,
                        NULL) != 0)
                        perror("pthread_create"), exit(1);
    if (pthread_create(&thread2, NULL,
                        consumer1,
                        NULL)!= 0)
                        perror("pthread_create"), exit(1);
    if (pthread_create(&thread3, NULL,
                        consumer2,
                        NULL)!= 0)
                        perror("pthread_create"), exit(1);
                        
    if (pthread_join(thread1, NULL) != 0)
        perror("pthread_join"), exit(1);

    if (pthread_join(thread2, NULL) != 0)
        perror("pthread_join"), exit(1);
        
    if (pthread_join(thread3, NULL) != 0)
        perror("pthread_join"), exit(1);
        
    printf("\n\n GIOCO TERMINATO ! \n\n");
    return 0;
}

void *producer() {
	
    while(true) {
    	
        while (num_elementi_presenti_buffer == LUNGHEZZA_BUFFER)  printf("\nproducer: buffer pieno");

        pthread_mutex_lock(&r3_mutex);
        produci_elementi();
        produci_elementi();
        pthread_mutex_unlock(&r3_mutex); 
        
        if (prob1 > 1 || prob2 > 1) 	break;				//se uno dei due giocatori ha raggiunto una buona probabilità vince quindi non bisogna più produrre carte
        
        if (num_elementi_prodotti == N_CARTE * mazzi) {
			printf("\nCARTE TERMINATE");
			break; 	
		}
	
    }
}

void *consumer1() {
	int num = 1;				//nome del primo giocatore
	
	while(true) {
    		
    	if(num_elementi_presenti_buffer == 0 && num_elementi_prodotti == N_CARTE * mazzi) break;
    	
    	while (num_elementi_presenti_buffer == 0)  printf("\n\nconsumer 1: buffer vuoto");
        
   		pthread_mutex_lock(&r3_mutex); 			
   		consuma_elementi(num);
 		consuma_elementi(num);
      	pthread_mutex_unlock(&r3_mutex);
			
		if(prob2 > 1.00){
			printf("\nIL GIOCATORE SI E' ALZATO DAL TAVOLO");	
			break;
		}
		
       	prob1 = calcolaprob(truecont1);
       	if(prob1 > 1.00){
			printf("\nVINCE IL GIOCATORE CON PROBABILITA' PARI A %f %c",prob1*100 ,perc);
			break;
		}
		
		if (num_elementi_prodotti == N_CARTE * mazzi) {
			printf("\nCARTE TERMINATE");
			break; 	
		}
	}
}

void *consumer2() {
	int num = 2;		//nome del secondo giocatore

     while(true) {
    		
    	if(num_elementi_presenti_buffer == 0 && num_elementi_prodotti == N_CARTE * mazzi) break;
    	
    	while (num_elementi_presenti_buffer == 0 )  printf("\n\nconsumer 2: buffer vuoto");
    					
		pthread_mutex_lock(&r3_mutex); 			
   		consuma_elementi(num);
 		consuma_elementi(num);
      	pthread_mutex_unlock(&r3_mutex);

		if(prob1 > 1.00){
			printf("\nIL DEALER HA IMPRECATO CONTRO IL GIOCATORE ED E' STATO PORTATO VIA DALLA SICUREZZA")	;	
			break;
  		}	
	
		prob2 = calcolaprob(truecont2);
		if(prob2 > 1.00){
			printf("\nVINCE IL BANCO CON PROBABILITA' PARI A %f %c",prob2*100 ,perc);
			break;
		}
		
		if (num_elementi_prodotti == N_CARTE * mazzi) {
			printf("\nCARTE TERMINATE");
			break; 	
		}
	}
}
