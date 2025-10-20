/* 
Multiplicacion de matrices cuadradas con optimizacion, implementada con openmp

La optimizacion que se realizo consistio en definir la transpuesta de la matriz B
antes de realizar la multiplicacion, para reducir el tiempo de acceso a los elementos
de la columna en esta matriz

Esto redujo el tiempo del proceso de multiplicacion considerablemente
sin el uso de banderas de compilacion
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

// para compilar, incluir bandera -fopenmp

// firmas de las funciones usadas
void inicializarMatricesCuadradas(int **, int **, int);
void multiplicarMatricesCuadradas(int **, int **, int **, int);
void multiplicarMatricesCuadradas2(int **, int **, int **, int);
void mostrarMatriz(int**, int);
void liberarMemoriaMatriz(int **, int);
int ** crearMatriz(int);
int ** transponerMatriz(int **, int);

// funcion main
int main(int argc, char *argv[]){

	int tamanioMatriz;
	int numeroHilos;
	
	// definiciones para tiempos cpu
	clock_t tiempo_inicio, tiempo_final;
	double tiempo_transcurrido;
	struct timespec begin, end; 
	
	// definiciones para tiempos wall
	long seconds;
	long nanoseconds;
	double elapsed;
	
	// verificacion del numero de variables y asignacion de variables 
	// para tamaño de la matriz y el numero de hilos
	// solo se verifica el numero de variables, no si estan en el rango aceptado
	if (argc != 2){
		printf("No se paso un tamaño de matriz. Se fijara uno por defecto\n\n");
		tamanioMatriz = 10;
		numeroHilos = 4;
	}
	
	if (argc == 2){
		printf("argumento en argv[1]:	%s\n", argv[1]);
		tamanioMatriz = atoi(argv[1]);
		numeroHilos = 4;
	}
	//inicializar generador de numeros aleatorios
	srand(getpid());
	
	// Configurar número de hilos de OpenMP
	omp_set_num_threads(numeroHilos);
	
	// declaracion e inicializacion de variables
	int ** p_matrizA, ** p_matrizB, ** p_matrizResultado;
	
	// creacion de matrices
	p_matrizA = crearMatriz(tamanioMatriz);
	p_matrizB = crearMatriz(tamanioMatriz);
	p_matrizResultado = crearMatriz(tamanioMatriz);
	

	//llenar de 0s la matriz resultado
	for (int i = 0; i < tamanioMatriz; i++) {
		memset(p_matrizResultado[i], 0, tamanioMatriz * sizeof(int));
	}

	
	// inicializacion de matrices operandos
	inicializarMatricesCuadradas(p_matrizA, p_matrizB, tamanioMatriz);
	
	// trasposicion de la matriz B
	p_matrizB = transponerMatriz(p_matrizB, tamanioMatriz);
	
	// inicio de los relojes para los tiempos de cpu y wall
	clock_gettime(CLOCK_REALTIME, &begin);
	tiempo_inicio = clock();

	// multiplicacion de las matrices cuadradas
	multiplicarMatricesCuadradas(p_matrizA, p_matrizB, p_matrizResultado, tamanioMatriz);

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

	// liberar memoria reservada para las matrices
	liberarMemoriaMatriz(p_matrizA, tamanioMatriz);
	liberarMemoriaMatriz(p_matrizB, tamanioMatriz);
	liberarMemoriaMatriz(p_matrizResultado, tamanioMatriz);
	
	return 0;
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

// multiplicar las matrices cuadradas y generar una matriz resultado tambien cuadrada
void multiplicarMatricesCuadradas(int ** p_matrizA, int ** p_matrizB, int ** p_matrizResultado, int tamanioMatriz){
	int i, j, k, acumulado;
		
	#pragma omp parallel for schedule(static) shared(acumulado, p_matrizA, p_matrizB, p_matrizResultado, tamanioMatriz) private(i, j, k)
	for(i = 0; i < tamanioMatriz; i++){ // este bucle recorre las filas de la matriz multiplicando

		//#pragma omp parallel for schedule(static) shared(i, acumulado, p_matrizA, p_matrizB, p_matrizResultado, tamanioMatriz) private( j, k)
		for(j = 0; j < tamanioMatriz; j++){ // este bucle recorre los elementos de la fila
			acumulado = 0;
			//#pragma omp parallel for schedule(static) shared(i, j, acumulado, p_matrizA, p_matrizB, p_matrizResultado, tamanioMatriz) private( k)
			for(k = 0; k < tamanioMatriz; k++){ // este bucle recorre los elementos de la columna

				acumulado = acumulado + (p_matrizA[i][k] * p_matrizB[j][k]);
			}
			p_matrizResultado[i][j] = acumulado;
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


// liberar memoria matriz
void liberarMemoriaMatriz(int ** matriz, int tamanioMatriz){
	int i;
	for(i = 0; i < tamanioMatriz; i++){
		free(matriz[i]);
	}
	free(matriz);
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
