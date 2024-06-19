#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define R 1.0   // Значение сопротивления
#define C 1.0   // Значение ёмкости

void simulate(int num_nodes, double time_interval) {
    // Реализация моделирования распространения сигнала
    // num_nodes: количество узлов
    // time_interval: временной интервал
    
    // Расчет количества узлов для каждого процесса
    int my_nodes = num_nodes / size;
    
    // Инициализация временных массивов
    double *prev_values = (double *)malloc(my_nodes * sizeof(double));
    double *curr_values = (double *)malloc(my_nodes * sizeof(double));
    
    // Основной цикл моделирования
    for (int t = 0; t < time_interval; t++) {
        // Расчет напряжения для текущего временного слоя
        for (int i = 0; i < my_nodes; i++) {
            if (i == 0) {
                // Задание начального напряжения для первого узла
                curr_values[i] = 1.0; // Примерное начальное напряжение
            } else if (i == my_nodes - 1) {
                // Расчет напряжения для последнего узла
                curr_values[i] = (prev_values[i-1] - 2 * prev_values[i]) * (time_interval / (R * C)) + prev_values[i];
            } else {
                // Расчет напряжения для остальных узлов
                curr_values[i] = (prev_values[i-1] - 2 * prev_values[i] + prev_values[i+1]) * (time_interval / (R * C)) + prev_values[i];
            }
        }
        
        // Обмен данными между процессами
        MPI_Allgather(curr_values, my_nodes, MPI_DOUBLE, prev_values, my_nodes, MPI_DOUBLE, MPI_COMM_WORLD);
    }
    
    // Освобождение выделенной памяти
    free(prev_values);
    free(curr_values);
}

int main(int argc, char *argv[]) {
    int num_nodes;            // Количество узлов
    double time_interval;     // Временной интервал
    
    MPI_Init(&argc, &argv);  // Инициализация MPI
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Получение ранга процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // Получение общего количества процессов
    
    if (argc != 3) {
        if (rank == 0) {
            printf("Usage: %s <num_nodes> <time_interval>\n", argv[0]);
        }
        MPI_Finalize();
        exit(1);
    }
    
    num_nodes = atoi(argv[1]);
    time_interval = atof(argv[2]);
    
    if (num_nodes % size != 0) {
        if (rank == 0) {
            printf("Number of nodes must be divisible by number of processes.\n");
        }
        MPI_Finalize();
        exit(1);
    }
    
    // Вызов функции моделирования
    simulate(num_nodes, time_interval);
    
    MPI_Finalize();  // Завершение MPI
    
    return 0;
}