/* MEMORIA COMPARTIDA */
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>

/* PARA FORK */
#include <sys/types.h>
#include <unistd.h>

/* PARA TERMINAR EL PROCESO HIJO */
#include <sys/wait.h>

// tandom number generation
#include <sys/times.h>
#include <time.h>

#define SHM_2DARRAY "array_shrmem1"
#define NUM_PROCESSES 2

void InicializarMatricesCuadradas(int ** p_matrizA, int tamanioMatriz);
void MultiplicarMatricesCuadradas(int ** p_matrizA, int tamanioMatriz,int limiteInf, int limiteSup);
void MostrarMatriz(int ** p_matriz, int tamanioMatriz, int posInicio);
void MostrarContenidoBloqueMemoria(int ** p_matriz, int dimensiones);
//void DefinirIntervalos(int * lista_intervalos, int tamanioMatriz);
int * DefinirIntervalos(int tamanioMatriz);
int crearSHM();
void asignarMemoriaSHM(int fd_array, int tamanioBloqueMemoria);


int main(int argc, char *argv[]){
	srand(time(NULL));
	int tamanioMatriz;
	struct tms start_times, end_times;
	clock_t start_clock, end_clock;
	
	if (argc != 2){
		printf("No se paso un tamanio de matriz. Se fijara uno por defecto\n\n");
		tamanioMatriz = 4;
	}
	
	if (argc == 2){
		printf("argumento en argv[1]:	%s\n", argv[1]);
		tamanioMatriz = atoi(argv[1]);
	}
	
	int numMatrices = 3;
	int dimensiones = tamanioMatriz * tamanioMatriz;
	int bloqueTotalMem = (dimensiones * sizeof(int)) * numMatrices;
	int *matriz2d;
	int fd_2darray;
	clock_t tiempo_inicio, tiempo_final;
	double tiempo_transcurrido;

	
	int * intervalos = DefinirIntervalos(tamanioMatriz);
	int i;
	for(i = 0; i < NUM_PROCESSES; i++){
		printf("%d, ", intervalos[i]);
	}
	printf("\n\n");
	
	
	fd_2darray = crearSHM();
	asignarMemoriaSHM(fd_2darray, bloqueTotalMem);
	matriz2d = mmap(NULL, bloqueTotalMem,PROT_READ | PROT_WRITE, MAP_SHARED, fd_2darray, 0);

	InicializarMatricesCuadradas(&matriz2d, tamanioMatriz);

	//MostrarMatriz(&matriz2d, tamanioMatriz, 0);
	//MostrarMatriz(&matriz2d, tamanioMatriz, dimensiones);
	
	//MostrarContenidoBloqueMemoria(&matriz2d, dimensiones * numMatrices);
	start_clock = times(&start_times);
	if (start_clock == (clock_t)-1) {
	        perror("times error");
	        exit(EXIT_FAILURE);
	}
	
	
	pid_t p;
	p = fork();
	
	if(p < 0){
		perror("fork failed");
		exit(1);
	}
	
	// proceso hijo; p es igual a 0
	else if(p == 0){

		tiempo_inicio = clock();
		MultiplicarMatricesCuadradas(&matriz2d, tamanioMatriz, intervalos[0], intervalos[1] + intervalos[0]);
		tiempo_final = clock();
		tiempo_transcurrido = (double)(tiempo_final - tiempo_inicio) / CLOCKS_PER_SEC;
		printf("\ntiempo transcurrido proceso hijo:	%f\n", tiempo_transcurrido);
		//MostrarMatriz(&matriz2d, tamanioMatriz, dimensiones * 2);
		
		end_clock = times(&end_times);
		if (end_clock == (clock_t)-1) {
			perror("times error in child");
			exit(EXIT_FAILURE);
		}
		
		// Calcular e imprimir el tiempo de CPU del hijo
		long clk_tck = sysconf(_SC_CLK_TCK);
		printf("Child CPU time: user=%.2f s, system=%.2f s\n",
			(double)(end_times.tms_utime - start_times.tms_utime) / clk_tck,
			(double)(end_times.tms_stime - start_times.tms_stime) / clk_tck);	
		exit(EXIT_SUCCESS);
		
	}
	
	// proceso padre; p es mayor que 0
	else{	
		tiempo_inicio = clock();
		MultiplicarMatricesCuadradas(&matriz2d, tamanioMatriz, 0, intervalos[0]);
		tiempo_final = clock();
		
		tiempo_transcurrido = (double)(tiempo_final - tiempo_inicio) / CLOCKS_PER_SEC;
		printf("\ntiempo transcurrido proceso padre:	%f\n", tiempo_transcurrido);
	
		// tambien puede usarse waitpid()
		wait(NULL); // hay que usar algun tipo de pausa para esperar a que el proceso hijo termine antes que el padre
		
		if (end_clock == (clock_t)-1) {
			perror("times error in parent");
			exit(EXIT_FAILURE);
		}
		
		 // Calcular e imprimir el tiempo de CPU del padre
		long clk_tck = sysconf(_SC_CLK_TCK);
		printf("Parent CPU time: user=%.2f s, system=%.2f s\n",
			(double)(end_times.tms_utime - start_times.tms_utime) / clk_tck,
			(double)(end_times.tms_stime - start_times.tms_stime) / clk_tck);

		//MostrarMatriz(&matriz2d, tamanioMatriz, dimensiones * 2);

		close(fd_2darray);
		munmap(matriz2d, bloqueTotalMem);
	
		shm_unlink(SHM_2DARRAY);		

	}

	// sin estas instrucciones se puede presentar un error de segmentacion al volver a abrir el programa
	//close(fd_2darray);
	//munmap(matriz2d, bloqueTotalMem);
	
	//shm_unlink(SHM_2DARRAY);
	return 0;	
}

// Inicializar matrices cuadradas --> llena dos matrices con numeros aleatorios
void InicializarMatricesCuadradas(int ** p_matrizA, int tamanioMatriz){
	int i, numeroA, numeroB;
	int limit = tamanioMatriz * tamanioMatriz;
	for(i = 0; i < limit; i++){
		numeroA = rand() % 10 + 1;
		numeroB = rand() % 10 + 1;
		(* p_matrizA)[i] = numeroA;
		(* p_matrizA)[i + limit] = numeroB;
	}
}

// multiplicar las matrices cuadradas y generar una matriz resultado tambien cuadrada¨
void MultiplicarMatricesCuadradas(int ** p_matrizA, int tamanioMatriz,int limiteInf, int limiteSup){
	int i, j, k, dimensiones, acumulado;
	dimensiones = tamanioMatriz * tamanioMatriz;
	//struct timespec start_ts, end_ts;
	//clockid_t threadClockId;
	
	//pthread_getcpuclockid(pthread_self(), &threadClockId);
	//clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_ts);
	//clock_gettime(threadClockId, &start_ts);	
	
	for(i = limiteInf; i < limiteSup; i++){
	//for(i = 0; i < tamanioMatriz; i++){ // este bucle recorre las filas de la matriz multiplicando
		for(j = 0; j < tamanioMatriz; j++){ // este bucle recorre los elementos de la fila
			acumulado = 0;
			for(k = 0; k < tamanioMatriz; k++){ // este bucle recorre los elementos de la columna
				//(* p_matrizA)[i][k] * (* p_matrizB)[k][j];
				acumulado += (* p_matrizA)[i * tamanioMatriz + k] * (* p_matrizA)[(k * tamanioMatriz + j) + dimensiones];
			}
			(* p_matrizA)[(i * tamanioMatriz + j) + (dimensiones * 2)] = acumulado;
		}
	}
	//clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_ts);
	//clock_gettime(threadClockId, &end_ts);

	//double elapsed_time = (end_ts.tv_sec - start_ts.tv_sec) + 
        //                  (double)(end_ts.tv_nsec - start_ts.tv_nsec) / 1e9;
	//printf("Thread CPU time: %f seconds\n", elapsed_time);
}

int crearSHM(){
	int fd_2darray = shm_open(SHM_2DARRAY, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if(fd_2darray == -1){
		perror("Error: No se pudo crear el espacio de memoria compartida");
		shm_unlink(SHM_2DARRAY);
	   	exit(1);
	}
	
	return fd_2darray;
}

void asignarMemoriaSHM(int fd_array, int tamanioBloqueMemoria){
		
	if(-1 == ftruncate(fd_array, tamanioBloqueMemoria) ){
		//printf("Error shared memory cannot be resized \n");
		perror("Error: No se pudo crear el espacio de memoria compartida");
		close(fd_array);
		shm_unlink(SHM_2DARRAY);
		exit(1);
	}
}

// definir intervalos de procesos --> fijo a 4 intervalos, por ahora
int * DefinirIntervalos(int tamanioMatriz){
//void DefinirIntervalos(int * lista_intervalos, int tamanioMatriz){
	int * lista_intervalos = (int *)malloc(sizeof(int) * NUM_PROCESSES);
	int modulo = tamanioMatriz % NUM_PROCESSES;
	int cociente = tamanioMatriz / NUM_PROCESSES;
	
	// falta el caso en el que el tamaño de la matriz es menor que el numero de hilos
	// pero por ahora no importa
	if(cociente != 0){
		for(int i = 0; i < NUM_PROCESSES; i++){
			if(i < NUM_PROCESSES - 1){
				lista_intervalos[i] = cociente;
			}
			
			if(i == NUM_PROCESSES - 1){
				lista_intervalos[i] = cociente + modulo;
			}
		}
	}
	
	if(cociente == 0){
		for(int i = 0; i < NUM_PROCESSES; i++){
			lista_intervalos[i] = cociente;
		}
	}
	return lista_intervalos;
}



void MostrarMatriz(int ** p_matriz, int tamanioMatriz, int posInicio){
	int i, j;
	for(i = 0; i < tamanioMatriz; i++){ // rows
		for(j = 0; j < tamanioMatriz; j++){ // columns
			printf("%d ", (* p_matriz)[(i * tamanioMatriz + j) + posInicio]);
		}
		printf("\n");
	}
	printf("\n\n");
}

void MostrarContenidoBloqueMemoria(int ** p_matriz, int dimensiones){
	int i;
	for(i = 0; i < dimensiones; i++){
		printf("%d ", (* p_matriz)[i]);
	}
	
	printf("\n\n");
}
