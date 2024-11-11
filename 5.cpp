#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

// Lista enlazada
struct Task
{
    int data;   // Datos de la tarea
    Task *next; // Apuntador a la siguiente tarea
};

// Estructura hilos
struct ThreadData
{
    pthread_t thread_id;
    int thread_num;
};

// Globales
Task *task_list = nullptr;      // Lista enlazada de tareas
Task *task_list_last = nullptr; // Última tarea de la lista
pthread_mutex_t task_mutex;     // Acceso a la lista de tareas
pthread_cond_t task_condition;  // Condición para esperar tareas
bool done = false;              // No hay más tareas

// Función Trabajo realizado por los hilos sobre las tareas
void *worker_thread(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    Task *task;
    while (true)
    {
        pthread_mutex_lock(&task_mutex); // Bloquea el acceso a la lista de tareas
        while (task_list == nullptr && !done)
        {
            pthread_cond_wait(&task_condition, &task_mutex); // El hilo se duerme
            std::cout << "Hilo " << data->thread_num << " esperando por tareas..." << std::endl;
        }

        if (done && task_list == nullptr)
        {
            pthread_mutex_unlock(&task_mutex);
            std::cout << "Hilo " << data->thread_num << " ha terminado, no hay más tareas." << std::endl;
            break;
        }
        task = task_list;            // Toma la primera tarea de la lista
        task_list = task_list->next; // Avanza a la siguiente tarea
        if (task_list == nullptr)
            task_list_last = nullptr;      // Si la lista está vacía
        pthread_mutex_unlock(&task_mutex); // Libera el acceso a la lista de tareas
        std::cout << "Hilo " << data->thread_num << " ejecutando tarea con datos: " << task->data << std::endl;
        delete task; // Elimina la tarea ejecutada
    }
    return nullptr;
}

// Función Genera tareas y las agrega a la lista
void generate_tasks(int num_tasks)
{
    for (int i = 0; i < num_tasks; ++i)
    {
        Task *new_task = new Task;
        new_task->data = rand() % 100;
        new_task->next = nullptr;
        pthread_mutex_lock(&task_mutex); // Bloquea el acceso a la lista de tareas
        if (task_list == nullptr)        // Si no hay tareas
        {
            task_list = new_task;      // La nueva tarea es la primera
            task_list_last = new_task; // Apunta también al último nodo
        }
        else
        {
            task_list_last->next = new_task; // Añade al final de la lista
            task_list_last = new_task;       // Actualiza el último nodo
        }
        pthread_mutex_unlock(&task_mutex); // Libera el acceso a la lista de tareas
        std::cout << "Generada tarea con datos: " << new_task->data << std::endl;
        pthread_cond_signal(&task_condition); // Despierta un hilo para procesar la tarea
    }
}

// Función para finalizar todas las tareas
void finalize_tasks()
{
    pthread_mutex_lock(&task_mutex);
    done = true;                             // Marca que no habrá más tareas
    pthread_cond_broadcast(&task_condition); // Despierta a todos los hilos para que salgan
    pthread_mutex_unlock(&task_mutex);
    std::cout << "No hay más tareas, finalizando..." << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Uso: " << argv[0] << " <num_threads> <num_tasks>" << std::endl;
        return 1;
    }

    int num_threads = atoi(argv[1]);
    int num_tasks = atoi(argv[2]);

    pthread_mutex_init(&task_mutex, nullptr);    // Inicializa el mutex
    pthread_cond_init(&task_condition, nullptr); // Inicializa la condición de tarea

    // Crea los hilos trabajadores
    ThreadData thread_data[num_threads];
    pthread_t threads[num_threads];

    for (int i = 0; i < num_threads; ++i)
    {
        thread_data[i].thread_num = i + 1;
        pthread_create(&threads[i], nullptr, worker_thread, &thread_data[i]);
    }
    generate_tasks(num_tasks);            // Genera las tareas
    sleep(1);                             // Espera para simular el trabajo
    finalize_tasks();                     // Finaliza el procesamiento de tareas
    for (int i = 0; i < num_threads; ++i) // Espera que todos los hilos terminen
        pthread_join(threads[i], nullptr);

    // Limpia los recursos
    pthread_mutex_destroy(&task_mutex);
    pthread_cond_destroy(&task_condition);
    std::cout << "Todas las tareas han sido completadas." << std::endl;

    return 0;
}
