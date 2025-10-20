/*
Multiplicacion matrices hilos optimizada

Se aplicarion algunas optimizaaciones como 

	- transposicion de la matriz b
	- utilizar una constante para almacenar el valor de las multiplicaciones
 	  en lugar de acceder constantemente al indice correspondiente de la
 	  matriz resultado, a almacenar operaciones
 	  
 	  La matriz transpuesta reduce enormemente el tiempo que demora el algoritmo en
 	  realizar el calculo e la multipicadio
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
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

void inicializarMatricesCuadradas(int **, int **, int);
void mostrarMatriz(int**, int);
void *multiplicarMatricesCuadradasOptimizada(void *);
void definirIntervalos(int **, int);
int ** transponerMatriz(int **, int);
int ** crearMatriz(int);
void liberarMemoriaMatriz(int **, int);

int main(int argc, char *argv[]){
	srand(time(NULL));
	
	int tamanioMatriz;
	
	// verificacion del numero de variables y asignacion de variables 
	// para tamaño de la matriz y el numero de hilos
	// solo se verifica el numero de variables, no si estan en el rango aceptado
	if (argc != 2){
		printf("No se paso un tamaÃ±o de matriz. Se fijara uno por defecto\n\n");
		tamanioMatriz = 56;
	}
	
	if (argc == 2){
		printf("argumento en argv[1]:	%s\n", argv[1]);
		//tamanio_matriz = (int)&argv[1];
		tamanioMatriz = atoi(argv[1]);
	}
	

	// variables para medir tiempo cpu
	clock_t tiempo_inicio, tiempo_final;
	double tiempo_transcurrido;
	struct timespec begin, end;
		
	// definiciones para tiempos wall
	long seconds;
	long nanoseconds;
	double elapsed;
	
	// declarar matrices	
	int ** p_matrizA;
	int ** p_matrizB;
	int ** p_matrizResultado;
	
	// crear matrices
	p_matrizA = crearMatriz(tamanioMatriz);
	p_matrizB = crearMatriz(tamanioMatriz);
	p_matrizResultado = crearMatriz(tamanioMatriz);
	
	// inicializar matrices
	inicializarMatricesCuadradas(p_matrizA, p_matrizB, tamanioMatriz);
	
	//llenar de 0s la matriz resultado
	for (int i = 0; i < tamanioMatriz; i++) {
		memset(p_matrizResultado[i], 0, tamanioMatriz * sizeof(int));
	}
	
	// transponer matriz B
	p_matrizB = transponerMatriz(p_matrizB, tamanioMatriz);
	
	// arreglo de contexto de hilos
	pthread_t hilos[NUM_THREADS];
	
	// intervalos de trabajo de cada proceso
	int *lista_intervalos;
	
	// definir intervalos de iteracion
	definirIntervalos(&lista_intervalos, tamanioMatriz);
	
	// variables de bucle for de hilos
	int limite_superior = lista_intervalos[0];
	int limite_inferior = 0;
	int rc;
	int i;
	long t;
	
	// inicio de los relojes para los tiempos de cpu y wall
	clock_gettime(CLOCK_REALTIME, &begin);
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
		rc = pthread_create(&hilos[i], NULL, multiplicarMatricesCuadradasOptimizada, (void *) &arreglo_datos_hilo[i]);
		if(rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		
		if(i + 1 < NUM_THREADS){
			limite_inferior = limite_superior;
			limite_superior = limite_superior + lista_intervalos[i + 1];
		}		

	}
	
	// sincronizar hilos
	for(i = 0; i < NUM_THREADS; i++){
		pthread_join(hilos[i], NULL);
	}
	
	// medicion de los tiempos finales de los relojes para cpu y wall
	tiempo_final = clock();
	clock_gettime(CLOCK_REALTIME, &end);
	
	// calculo segundos y nanosegundos tiempo wall
	seconds = end.tv_sec - begin.tv_sec;
	nanoseconds = end.tv_nsec - begin.tv_nsec;
	
	// calculo de tiempo final wall
	elapsed = seconds + nanoseconds*1e-9;
	
	// calculo de tiempo final cpu
	tiempo_transcurrido = (double)(tiempo_final - tiempo_inicio) / CLOCKS_PER_SEC;
	
	// mostrar tiempos en pantalla
	printf("\ntiempo transcurrido multiplicacion_matrices (CPU/WALL):	%f	%f\n", tiempo_transcurrido, elapsed);

	
	// liberar memoria matriz
	liberarMemoriaMatriz(p_matrizA, tamanioMatriz);
	liberarMemoriaMatriz(p_matrizB, tamanioMatriz);
	liberarMemoriaMatriz(p_matrizResultado, tamanioMatriz);
	
	pthread_exit(NULL);
}


// inicializar matrices cuadradas con numeros int random 
void inicializarMatricesCuadradas(int ** p_matrizA, int ** p_matrizB, int tamanioMatriz){
	int i, j, numeroA, numeroB;
	
	for(i = 0; i < tamanioMatriz; i++){
		for(j = 0; j < tamanioMatriz; j++){
			numeroA = rand() % 10 + 1;
			numeroB = rand() % 10 + 1;
			p_matrizA[i][j] = numeroA;
			p_matrizB[i][j] = numeroB;
		}
	}
}

void *multiplicarMatricesCuadradasOptimizada(void *parametros_hilo){
	int i, j, k, acumulado, limite_inferior, limite_superior, tamanioMatriz;
	int *** p_matrizResultado, *** p_matrizA, *** p_matrizB;
	struct datos_hilo * datos_operacion;
	
	datos_operacion = (struct datos_hilo *) parametros_hilo;
	tamanioMatriz = datos_operacion->tamanioMatriz;
	limite_inferior = datos_operacion->limite_inferior;
	limite_superior = datos_operacion->limite_superior;
	p_matrizA = datos_operacion->matrizA;
	p_matrizB = datos_operacion->matrizB;
	p_matrizResultado = datos_operacion->matrizResultado;
	
	for(i = limite_inferior; i < limite_superior; i++){ // este bucle recorre las filas de la matriz multiplicando
		(* p_matrizResultado)[i] = (int *)malloc(tamanioMatriz * sizeof(int));
		for(j = 0; j < tamanioMatriz; j++){ // este bucle recorre los elementos de la fila
			acumulado = 0;
			for(k = 0; k < tamanioMatriz; k++){ // este bucle recorre los elementos de la columna
				acumulado += (* p_matrizA)[i][k] * (* p_matrizB)[j][k];
			}
			(* p_matrizResultado)[i][j] = acumulado;
		}
	}

}

// mostrar el contenido de una matriz cuadrada
void mostrarMatriz(int ** p_matriz, int tamanioMatriz){
	int i, j;
	for(i = 0; i < tamanioMatriz; i++){
		for(j = 0; j < tamanioMatriz; j++){
			printf("%d ", p_matriz[i][j]);
		}
		printf("\n");
	}
	printf("\n\n");
}

void definirIntervalos(int ** lista_intervalos, int tamanioMatriz){
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

// crear matrices cuadradas
int ** crearMatriz(int tamanioMatriz){
	int ** matriz = (int **)malloc(tamanioMatriz * sizeof(int *));
	int i, j;
	
	if(matriz == NULL){
		perror("No se pudo reservar memoria para los punteros de las columnas");
		exit(1);
	}
	
	for(i = 0; i < tamanioMatriz; i++){
		matriz[i] = (int *)malloc(tamanioMatriz * sizeof(int));
	
		if(matriz[i] == NULL){
			perror("No se pudo reservar memoria para los punteros de las filas");
			
			for(j = 0; j < i; j++){
				free(matriz[j]);
				}
				free(matriz);
				exit(1);
			}
		}
	
	return matriz;
}

// transponer matriz
int ** transponerMatriz(int ** matriz, int tamanioMatriz){
	int i, j;
	int ** matrizResultado = crearMatriz(tamanioMatriz);
	
	for(i = 0; i < tamanioMatriz; i++){
		for(j = 0; j < tamanioMatriz; j++){
			matrizResultado[i][j] = matriz[j][i];
		}
	}
	liberarMemoriaMatriz(matriz, tamanioMatriz);
	return matrizResultado;
}

// liberar memoria matriz
void liberarMemoriaMatriz(int ** matriz, int tamanioMatriz){
	int i;
	for(i = 0; i < tamanioMatriz; i++){
		free(matriz[i]);
	}
	free(matriz);
}
