#include <iostream>
#include <pthread.h>
#include <cmath>
#include <cstdlib>

double a = 0.0, b = 10.0;   // Límites de integración
int n = 1000000;            // Número total de trapezoides
double global_result = 0.0; // Resultado compartido
bool busy_flag = false;     // Bandera para busy-waiting

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

    while (busy_flag)
        ;
    busy_flag = true;
    global_result += local_result;
    busy_flag = false;

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

    for (int i = 0; i < num_threads; ++i)
    {
        thread_data[i] = {i, a, b, n, num_threads};
        pthread_create(&threads[i], nullptr, trapezoidalRule, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; ++i)
        pthread_join(threads[i], nullptr);

    std::cout << "Resultado aproximado de la integral: " << global_result << std::endl;

    return 0;
}
