// multiplicacion de matrices cuadradas
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void InicializarMatricesCuadradas(int ***, int ***, int);
int ** MultiplicarMatricesCuadradas(int ***, int ***, int);
void MostrarMatriz(int***, int);


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
		printf("No se paso un tamaño de matriz. Se fijara uno por defecto\n\n");
		tamanioMatriz = 10;
	}
	
	if (argc == 2){
		printf("argumento en argv[1]:	%s\n", argv[1]);
		//tamanio_matriz = (int)&argv[1];
		tamanioMatriz = atoi(argv[1]);
	}
	
	InicializarMatricesCuadradas(&p_matrizA, &p_matrizB, tamanioMatriz);
	
	
	//MostrarMatriz(&p_matrizA, tamanioMatriz);
	//MostrarMatriz(&p_matrizB, tamanioMatriz);
	tiempo_inicio = clock();
	p_matrizResultado = MultiplicarMatricesCuadradas(&p_matrizA, &p_matrizB, tamanioMatriz);
	tiempo_final = clock();
	tiempo_transcurrido = (double)(tiempo_final - tiempo_inicio) / CLOCKS_PER_SEC;
	printf("\ntiempo transcurrido multiplicacion_matrices:	%f\n", tiempo_transcurrido);
	//MostrarMatriz(&p_matrizResultado, tamanioMatriz);
	
	system("pause");
	return 0;
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

// multiplicar las matrices cuadradas y generar una matriz resultado tambien cuadrada
int ** MultiplicarMatricesCuadradas(int *** p_matrizA, int *** p_matrizB, int tamanioMatriz){
	int i, j, k, acumulado;
	int ** p_matrizResultado = (int **)malloc(tamanioMatriz * sizeof(int *));
	
	for(i = 0; i < tamanioMatriz; i++){ // este bucle recorre las filas de la matriz multiplicando
		p_matrizResultado[i] = (int *)malloc(tamanioMatriz * sizeof(int));
		for(j = 0; j < tamanioMatriz; j++){ // este bucle recorre los elementos de la fila
			acumulado = 0;
			for(k = 0; k < tamanioMatriz; k++){ // este bucle recorre los elementos de la columna
				acumulado += (* p_matrizA)[i][k] * (* p_matrizB)[k][j];
			}
			p_matrizResultado[i][j] = acumulado;
		}
	}
	return p_matrizResultado;
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
