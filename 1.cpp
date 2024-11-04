#include <iostream>
#include <vector>
#include <iomanip>
#include <pthread.h>
#include <cstdlib>

// Estructura hilos
struct ThreadArgs
{
    int thread_id;
    const std::vector<double> *data;
    int num_bins;
    double min_value;
    double bin_width;
    int start;
    int end;
    std::vector<int> *local_histogram;
};

void *calculateHistogram(void *args)
{
    ThreadArgs *threadArgs = static_cast<ThreadArgs *>(args);
    int thread_id = threadArgs->thread_id;
    const std::vector<double> &data = *threadArgs->data;
    int num_bins = threadArgs->num_bins;
    double min_value = threadArgs->min_value;
    double bin_width = threadArgs->bin_width;
    int start = threadArgs->start;
    int end = threadArgs->end;
    std::vector<int> &local_histogram = *threadArgs->local_histogram;

    std::cout << "Hilo " << thread_id << " procesando datos desde índice "
              << start << " hasta " << end - 1 << std::endl;

    for (int i = start; i < end; ++i)
    {
        double value = data[i];
        if (value >= min_value && value < (num_bins * bin_width) + min_value)
        {
            int bin_index = static_cast<int>((value - min_value) / bin_width);
            local_histogram[bin_index]++;
        }
    }
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
    // Configuración de datos y parámetros de los bins
    const int num_bins = 5;
    double min_value = 0.0;
    double max_value = 5.0;
    double bin_width = (max_value - min_value) / num_bins;

    // Datos a procesar
    std::vector<double> data = {1.3, 2.9, 0.4, 0.3, 1.3, 4.4, 1.7, 0.4, 3.2, 0.3,
                                4.9, 2.4, 3.1, 4.4, 3.9, 0.4, 4.2, 4.5, 4.9, 0.9};

    int data_size = data.size();
    int local_size = data_size / num_threads;

    // Inicializa el histograma global
    std::vector<int> global_histogram(num_bins, 0);

    std::vector<pthread_t> threads(num_threads);
    std::vector<ThreadArgs> thread_args(num_threads);
    std::vector<std::vector<int>> local_histograms(num_threads, std::vector<int>(num_bins, 0));

    for (int t = 0; t < num_threads; ++t)
    {
        thread_args[t] = {t, &data, num_bins, min_value, bin_width, t * local_size,
                          (t == num_threads - 1) ? data_size : (t + 1) * local_size,
                          &local_histograms[t]};
        pthread_create(&threads[t], nullptr, calculateHistogram, &thread_args[t]);
    }

    for (int t = 0; t < num_threads; ++t)
        pthread_join(threads[t], nullptr);

    for (int t = 0; t < num_threads; ++t)
        for (int b = 0; b < num_bins; ++b)
            global_histogram[b] += local_histograms[t][b];

    std::cout << "Histograma:" << std::endl;
    for (int i = 0; i < num_bins; i++)
        std::cout << std::fixed << std::setprecision(2) << (min_value + i * bin_width) << " - "
                  << std::fixed << std::setprecision(2) << (min_value + (i + 1) * bin_width) << ": "
                  << global_histogram[i] << std::endl;

    return 0;
}
