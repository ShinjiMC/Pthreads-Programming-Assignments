#include <iostream>
#include <pthread.h>
#include <cmath>
#include <cstdlib>
#include <semaphore.h> // Biblioteca para usar semáforos

double a = 0.0, b = 10.0;   // Límites de integración
int n = 1000000;            // Número total de trapezoides
double global_result = 0.0; // Resultado compartido
sem_t result_semaphore;     // Semáforo para proteger la variable compartida

// Función f(x) = x^3
double f(double x)
{
    return x * x * x;
}

// Estructura  hilos
struct ThreadData
{
    int thread_id;
    double a, b;
    int n, num_threads;
};

void *trapezoidalRule(void *args)
{
    ThreadData *data = (ThreadData *)args;
    int thread_id = data->thread_id;
    double local_a = data->a + thread_id * (data->b - data->a) / data->num_threads;
    double local_b = local_a + (data->b - data->a) / data->num_threads;
    int local_n = data->n / data->num_threads;
    double h = (data->b - data->a) / data->n;
    double local_result = 0.0;

    for (int i = 0; i < local_n; ++i)
    {
        double x1 = local_a + i * h;
        double x2 = local_a + (i + 1) * h;
        local_result += (f(x1) + f(x2)) * h / 2.0;
    }

    // Bloquea la sección crítica usando semáforo
    sem_wait(&result_semaphore);
    global_result += local_result;
    sem_post(&result_semaphore);

    return nullptr;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Use: " << argv[0] << " <num_threads>" << std::endl;
        return 1;
    }

    int num_threads = std::atoi(argv[1]);
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    // Inicialización del semáforo con valor 1
    sem_init(&result_semaphore, 0, 1);

    for (int i = 0; i < num_threads; ++i)
    {
        thread_data[i] = {i, a, b, n, num_threads};
        pthread_create(&threads[i], nullptr, trapezoidalRule, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; ++i)
        pthread_join(threads[i], nullptr);

    // Destruir el semáforo
    sem_destroy(&result_semaphore);

    std::cout << "Resultado aproximado de la integral: " << global_result << std::endl;

    return 0;
}
