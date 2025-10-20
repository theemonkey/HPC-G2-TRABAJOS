/*
 multiplicacion de matrices cuadradas optimizad
 
 Se aplicaron un par de optimizaciones como:
 
 	- Tiling/blocking
 	- Matriz transpuesta
 	- utilizar una constante para almacenar el valor de las multiplicaciones
 	  en lugar de acceder constantemente al indice correspondiente de la
 	  matriz resultado, a almacenar operaciones
 	  
 	  Por separado, la optmizacion mas efectiva es tiling/blocking, que reduce
 	  la mayor cantidad de tiempo.
 	  
 	  Luego, sigue la transposicion de la matriz B, que en la version secuencial,
 	  logra reducir unos pocos segundos, lo que se nota mas con matrices de 1000 en
 	  adelante, y mientras mas grandes mas se nota.
 	  
 	  la otra optimizacion ayuda a reducir las operaciones de acceso a la matriz resultado
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

// para compilar, incluir bandera -fopenmp

// optimizacion -O
// firmas de las funciones usadas

void inicializarMatricesCuadradas(int **, int **, int);
void multiplicarMatricesCuadradasOptimizada(int **, int **, int **, int);
void mostrarMatriz(int**, int);
void liberarMemoriaMatriz(int **, int);
int ** crearMatriz(int);
int ** transponerMatriz(int **, int);

// funcion main
int main(int argc, char *argv[]){

	int tamanioMatriz;
	int numeroHilos;
	
	// variables para calcular tiempo cpu
	clock_t tiempo_inicio, tiempo_final;
	double tiempo_transcurrido;
	struct timespec begin, end; 
	
	// variables para calcular tiempo wall
	long seconds;
	long nanoseconds;
	double elapsed;
	
	if (argc != 2){
		printf("No se paso un tamaño de matriz. Se fijara uno por defecto\n\n");
		tamanioMatriz = 10;
		numeroHilos = 2;
	}
	
	if (argc == 2){
		printf("argumento en argv[1]:	%s\n", argv[1]);
		tamanioMatriz = atoi(argv[1]);
		numeroHilos = 2;
	}
	
	//inicializar generador de numeros aleatorios
	srand(getpid());
	
	// declaracion e inicializacion de variables
	int ** p_matrizA, ** p_matrizB, ** p_matrizResultado;

	// crear matrices en memoria
	p_matrizA = crearMatriz(tamanioMatriz);
	p_matrizB = crearMatriz(tamanioMatriz);
	p_matrizResultado = crearMatriz(tamanioMatriz);
	
	// inicializar matriz resultado en 0
	for (int i = 0; i < tamanioMatriz; i++) {
		memset(p_matrizResultado[i], 0, tamanioMatriz * sizeof(int));
	}

	// inicializar matrices A y B con valores aleatorios
	inicializarMatricesCuadradas(p_matrizA, p_matrizB, tamanioMatriz);
	
	// transponer matriz B
	p_matrizB = transponerMatriz(p_matrizB, tamanioMatriz);
	
	// iniciar relojes para calcular tiempo cpu y wall
	clock_gettime(CLOCK_REALTIME, &begin);
	tiempo_inicio = clock();
	
	// calcular multiplicacion de matrices A y B
	multiplicarMatricesCuadradasOptimizada(p_matrizA, p_matrizB, p_matrizResultado, tamanioMatriz);
	
	// detener relojes para obtener tiempos finales de cpu y wall
	tiempo_final = clock();
	clock_gettime(CLOCK_REALTIME, &end);
	
	// calcular tiempo wall final
	seconds = end.tv_sec - begin.tv_sec;
	nanoseconds = end.tv_nsec - begin.tv_nsec;
	elapsed = seconds + nanoseconds*1e-9;
	
	// calcular tiempo cpu final
	tiempo_transcurrido = (double)(tiempo_final - tiempo_inicio) / CLOCKS_PER_SEC;
	
	// mostrar resultados en pantalla
	printf("\ntiempo transcurrido multiplicacion_matrices (CPU/WALL):	%f	%f\n", tiempo_transcurrido, elapsed);
	
	// liberar memoria de la matriz
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

// multiplicar las matrices cuadradas con blocking/tiling y transposicion de matriz B
void multiplicarMatricesCuadradasOptimizada(int ** p_matrizA, int ** p_matrizB, int ** p_matrizResultado, int tamanioMatriz){
	int i, j, k, acumulado;
	int lim_i, lim_j, lim_k;
	const int BLOCK_SIZE = 64; // Tamaño de bloque para optimización de cache
    // Multiplicación por bloques para mejor uso de cache
    for (lim_i = 0; lim_i < tamanioMatriz; lim_i += BLOCK_SIZE) {
        for (lim_j = 0; lim_j < tamanioMatriz; lim_j += BLOCK_SIZE) {
            for (lim_k = 0; lim_k < tamanioMatriz; lim_k += BLOCK_SIZE) {
                // Calcular límites del bloque
                int i_end = (lim_i + BLOCK_SIZE < tamanioMatriz) ? lim_i + BLOCK_SIZE : tamanioMatriz;
                int j_end = (lim_j + BLOCK_SIZE < tamanioMatriz) ? lim_j + BLOCK_SIZE : tamanioMatriz;
                int k_end = (lim_k + BLOCK_SIZE < tamanioMatriz) ? lim_k + BLOCK_SIZE : tamanioMatriz;
                	
		for(i = lim_i; i < i_end; i++){ // este bucle recorre las filas de la matriz multiplicando
		    for(j = lim_j; j < j_end; j++){ // este bucle recorre los elementos de la fila
			acumulado = p_matrizResultado[i][j];
			    for(k = lim_k; k < k_end; k++){ // este bucle recorre los elementos de la columna
				acumulado += p_matrizA[i][k] * p_matrizB[j][k];
			    }
			    p_matrizResultado[i][j] = acumulado;
		    }
	        }
            }
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


// liberar memoria reservada para una matriz
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
