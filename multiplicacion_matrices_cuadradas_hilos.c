/*
Regla para dividir el tamanio de la matriz por el numero de hilos

Ejemplo: si tengo tamanio 100 de matriz y 4 hilos, cada hilo hara 25 filas

Si tengo tamanio 100 de matriz y 12 hilos, entonces

100 / 12 = 8, residuo = 4

entonces, se puede tener 11 hilos que hagan 8 filas y un hilo que haga 8 + 4 = 12
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#define NUM_THREADS 2

struct datos_hilo{
	int limite_inferior;
	int limite_superior;
	int tamanioMatriz;
	int *** matrizResultado;
	int *** matrizA;
	int *** matrizB;
};

struct datos_hilo arreglo_datos_hilo[NUM_THREADS];

void InicializarMatricesCuadradas(int ***, int ***, int);
void MostrarMatriz(int***, int);
void *MultiplicarMatricesCuadradas(void *);
void DefinirIntervalos(int **, int);

int main(int argc, char *argv[]){
	srand(time(NULL));
	
	int tamanioMatriz;
	int ** p_matrizA;
	int ** p_matrizB;
	int ** p_matrizResultado;
	clock_t tiempo_inicio, tiempo_final;
	double tiempo_transcurrido;
	// NOTA: no se puede hacer esto
	// int ** p_matrizA, p_matrizB, p_matrizResultado;
	// no los reconoce a todos como punteros
	// supuestamente, se deberia hacer
	// int ** p_matrizA, ** p_matrizB, ** p_matrizResultado;
	
	if (argc != 2){
		printf("No se paso un tamaÃ±o de matriz. Se fijara uno por defecto\n\n");
		tamanioMatriz = 56;
	}
	
	if (argc == 2){
		printf("argumento en argv[1]:	%s\n", argv[1]);
		//tamanio_matriz = (int)&argv[1];
		tamanioMatriz = atoi(argv[1]);
	}
	
	InicializarMatricesCuadradas(&p_matrizA, &p_matrizB, tamanioMatriz);
	
	p_matrizResultado = (int **)malloc(tamanioMatriz * sizeof(int *));
	pthread_t hilos[NUM_THREADS];
	int *lista_intervalos;
	
	DefinirIntervalos(&lista_intervalos, tamanioMatriz);
	
	int limite_superior = lista_intervalos[0];
	int limite_inferior = 0;
	int rc;
	int i;
	long t;
	tiempo_inicio = clock();
	for(i = 0; i < NUM_THREADS; i++){
		printf("\nLimite inferior: %d		Limite superior: %d\n", limite_inferior, limite_superior);
		arreglo_datos_hilo[i].limite_inferior = limite_inferior;
		arreglo_datos_hilo[i].limite_superior = limite_superior;
		arreglo_datos_hilo[i].tamanioMatriz = tamanioMatriz;
		arreglo_datos_hilo[i].matrizA = &p_matrizA;
		arreglo_datos_hilo[i].matrizB = &p_matrizB;
		arreglo_datos_hilo[i].matrizResultado = &p_matrizResultado;
		//tiempo_inicio = clock();
		rc = pthread_create(&hilos[i], NULL, MultiplicarMatricesCuadradas, (void *) &arreglo_datos_hilo[i]);
		if(rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		
		if(i + 1 < NUM_THREADS){
			limite_inferior = limite_superior;
			limite_superior = limite_superior + lista_intervalos[i + 1];
		}
		pthread_join(hilos[i], NULL);
	}
	tiempo_final = clock();
	tiempo_transcurrido = (double)(tiempo_final - tiempo_inicio) / CLOCKS_PER_SEC;
	printf("\ntiempo transcurrido multiplicacion_matrices:	%f\n", tiempo_transcurrido);
	//MostrarMatriz(&p_matrizA, tamanioMatriz);
	//MostrarMatriz(&p_matrizB, tamanioMatriz);
	
	//MostrarMatriz(&p_matrizResultado, tamanioMatriz);
	
	system("pause");
	pthread_exit(NULL);
}


// inicializar matrices cuadradas con numeros int random 
void InicializarMatricesCuadradas(int *** p_matrizA, int *** p_matrizB, int tamanioMatriz){
	int i, j, numeroA, numeroB;
	* p_matrizA = (int **)malloc(tamanioMatriz * sizeof(int *));
	* p_matrizB = (int **)malloc(tamanioMatriz * sizeof(int *));
	
	for(i = 0; i < tamanioMatriz; i++){
		(* p_matrizA)[i] = (int *)malloc(tamanioMatriz * sizeof(int));
		(* p_matrizB)[i] = (int *)malloc(tamanioMatriz * sizeof(int));
		for(j = 0; j < tamanioMatriz; j++){
			numeroA = rand() % 10 + 1;
			numeroB = rand() % 10 + 1;
			(* p_matrizA)[i][j] = numeroA;
			(* p_matrizB)[i][j] = numeroB;
		}
	}
}

void *MultiplicarMatricesCuadradas(void *parametros_hilo){
	int i, j, k, acumulado, limite_inferior, limite_superior, tamanioMatriz;
	int *** p_matrizResultado, *** p_matrizA, *** p_matrizB;
	struct datos_hilo * datos_operacion;
	struct timespec start_ts, end_ts;
	clockid_t threadClockId;
	
	datos_operacion = (struct datos_hilo *) parametros_hilo;
	tamanioMatriz = datos_operacion->tamanioMatriz;
	limite_inferior = datos_operacion->limite_inferior;
	limite_superior = datos_operacion->limite_superior;
	p_matrizA = datos_operacion->matrizA;
	p_matrizB = datos_operacion->matrizB;
	p_matrizResultado = datos_operacion->matrizResultado;
	
	pthread_getcpuclockid(pthread_self(), &threadClockId);
	//clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_ts);
	clock_gettime(threadClockId, &start_ts);
	for(i = limite_inferior; i < limite_superior; i++){ // este bucle recorre las filas de la matriz multiplicando
		(* p_matrizResultado)[i] = (int *)malloc(tamanioMatriz * sizeof(int));
		for(j = 0; j < tamanioMatriz; j++){ // este bucle recorre los elementos de la fila
			acumulado = 0;
			for(k = 0; k < tamanioMatriz; k++){ // este bucle recorre los elementos de la columna
				acumulado += (* p_matrizA)[i][k] * (* p_matrizB)[k][j];
			}
			(* p_matrizResultado)[i][j] = acumulado;
		}
	}
	//clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_ts);
	clock_gettime(threadClockId, &end_ts);

	double elapsed_time = (end_ts.tv_sec - start_ts.tv_sec) + 
                          (double)(end_ts.tv_nsec - start_ts.tv_nsec) / 1e9;
	printf("Thread CPU time: %f seconds\n", elapsed_time);
}

// mostrar el contenido de una matriz cuadrada
void MostrarMatriz(int *** p_matriz, int tamanioMatriz){
	int i, j;
	for(i = 0; i < tamanioMatriz; i++){
		for(j = 0; j < tamanioMatriz; j++){
			printf("%d ", (* p_matriz)[i][j]);
		}
		printf("\n");
	}
	printf("\n\n");
}

void DefinirIntervalos(int ** lista_intervalos, int tamanioMatriz){
	(* lista_intervalos) = (int *)malloc(sizeof(int) * NUM_THREADS);
	int modulo = tamanioMatriz % NUM_THREADS;
	int cociente = tamanioMatriz / NUM_THREADS;
	
	// falta el caso en el que el tamaño de la matriz es menor que el numero de hilos
	// pero por ahora no importa
	if(cociente != 0){
		for(int i = 0; i < NUM_THREADS; i++){
			if(i < NUM_THREADS - 1){
				(* lista_intervalos)[i] = cociente;
			}
			
			if(i == NUM_THREADS - 1){
				(* lista_intervalos)[i] = cociente + modulo;
			}
		}
	}
	
	if(cociente == 0){
		for(int i = 0; i < NUM_THREADS; i++){
			(* lista_intervalos)[i] = cociente;
		}
	}
	//return &lista_intervalos;
}
