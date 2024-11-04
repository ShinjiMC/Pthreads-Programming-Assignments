# Pthreads Programming Assignments

Based on Peter Pacheco-An Introduction to Parallel Programming-Morgan Kaufmann (2011)

By Braulio Nayap Maldonado Casilla

## Ejercicio 4.1

### Contexto

Según la sección 2.7.1 mencionada:

1. **Generación de Datos**: Se tiene un conjunto de datos que son números de punto flotante. En el ejemplo dado, los datos son:

   ```
   1.3, 2.9, 0.4, 0.3, 1.3, 4.4, 1.7, 0.4, 3.2, 0.3,
   4.9, 2.4, 3.1, 4.4, 3.9, 0.4, 4.2, 4.5, 4.9, 0.9
   ```

2. **Rango de Datos**: Los datos se encuentran en un rango de 0 a 5. Esto significa que se desea visualizar la distribución de estos datos en este rango específico.

3. **División en Bins**: Para crear un histograma, el rango de los datos se divide en intervalos iguales, llamados "bins". En el ejemplo, se elige dividir el rango en 5 bins.

4. **Conteo de Mediciones**: Para cada bin, se cuenta cuántos datos caen dentro de ese intervalo. Esto permite observar la distribución de los datos a través de los diferentes bins.

5. **Representación del Histograma**: Finalmente, el histograma se puede representar como un gráfico de barras donde cada barra indica el número de mediciones en cada bin.

![Ejemplo](.docs/text_3_1.png)

### Implementación con Pthreads

- **Importaciones de Bibliotecas:** Se incluyen bibliotecas necesarias para entrada/salida, vectores, manipulación de salida, hilos y funciones de utilidad.

- **Definición de la Estructura `ThreadArgs`:** Se define una estructura para almacenar los argumentos necesarios para cada hilo, incluyendo identificador, datos, número de bins, valores de rango, y un puntero al histograma local.

  ```cpp
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
  ```

- **Definición de la Función `calculateHistogram`:** Se define la función que ejecutarán los hilos. Los argumentos se convierten a `ThreadArgs` para su uso.

- **Extracción de Parámetros:** Se extraen los parámetros necesarios desde la estructura.

- **Cálculo del Histograma Local:** Se itera sobre los datos asignados al hilo y se calcula el índice del bin correspondiente para incrementar el histograma local.

  ```cpp
  for (int i = start; i < end; ++i)
  {
      double value = data[i];
      if (value >= min_value && value < (num_bins * bin_width) + min_value)
      {
          int bin_index = static_cast<int>((value - min_value) / bin_width);
          local_histogram[bin_index]++;
      }
  }
  ```

- **Finalización de la Función:** La función finaliza devolviendo `nullptr`.

- **Inicio de `main` y Comprobación de Argumentos:** Se inicia el programa y se verifica que se reciba el número de hilos como argumento.

- **Configuración de Parámetros del Histograma:** Se definen el número de bins y se calculan los valores mínimos, máximos y el ancho de cada bin.

- **Inicialización de Datos:** Se inicializa un vector con los datos a procesar.

- **Cálculo de Tamaño de Datos y Localización:** Se calcula el tamaño total de los datos y el tamaño local que cada hilo procesará.

- **Inicialización del Histograma Global:** Se crea un vector para almacenar el histograma global, inicializado en cero.

- **Creación y Lanzamiento de Hilos:** Se crean y lanzan hilos, configurando sus argumentos antes de llamarlos.
  ```cpp
  for (int t = 0; t < num_threads; ++t)
  {
      thread_args[t] = {t, &data, num_bins, min_value, bin_width, t * local_size,
                        (t == num_threads - 1) ? data_size : (t + 1) * local_size,
                        &local_histograms[t]};
      pthread_create(&threads[t], nullptr, calculateHistogram, &thread_args[t]);
  }
  ```
- **Sincronización de Hilos:** Se espera a que todos los hilos terminen antes de continuar.
  ```cpp
  for (int t = 0; t < num_threads; ++t)
        pthread_join(threads[t], nullptr);
  ```
- **Combinar Histogramas Locales:** Se suman los histogramas locales a un histograma global.

  ```cpp
  for (int t = 0; t < num_threads; ++t)
        for (int b = 0; b < num_bins; ++b)
            global_histogram[b] += local_histograms[t][b];
  ```

- **Impresión del Histograma Global:** Se imprime el histograma global con el rango y conteo de cada bin.

### Ejecución

```bash
g++ -o histogram 1.cpp -pthread
./histogram <num_threads>
```

### Visualización

![Ejecución](.docs/4_1.png)

## Ejercicio 4.3

### Contexto

La regla trapezoidal es un método numérico utilizado para aproximar la integral definida de una función. Se basa en dividir el área bajo la curva en trapezoides y sumar sus áreas. Para implementar esto en un programa de Pthreads:

1. **Dividir el intervalo de integración** en subintervalos (por ejemplo, dividirlo en `n` subintervalos).
2. **Cada hilo calculará el área** de uno o más trapezoides en su intervalo asignado.
3. **Utilizar una variable compartida** para almacenar la suma total de las áreas calculadas por todos los hilos.

![Ejemplo](.docs/text_4_2.png)

#### Enfoques para Exclusión Mutua

1. **Busy-Waiting (Espera Activa):**

   - **Descripción:** Un hilo que necesita acceder a la sección crítica se queda en un bucle comprobando repetidamente si puede entrar. No libera la CPU mientras espera.
   - **Implementación:**

     - Un hilo utiliza un bucle que comprueba si puede adquirir el control del recurso compartido.

   - **Ventajas:**

     - Simple de implementar y entender.
     - Puede ser adecuado para sistemas de tiempo real donde los tiempos de espera son cortos.

   - **Desventajas:**
     - Desperdicia CPU: consume recursos de CPU mientras espera, lo que puede llevar a una reducción del rendimiento en sistemas con múltiples hilos.
     - No es eficiente para tareas largas, ya que los hilos ocupan ciclos de CPU innecesariamente.

2. **Mutexes (Mutual Exclusion Locks):**

   - **Descripción:** Un mutex es una primitiva de sincronización que permite que un solo hilo acceda a un recurso compartido en un momento dado.
   - **Implementación:**

     - Antes de entrar en la sección crítica, un hilo bloquea el mutex. Después de completar la operación, libera el mutex.

   - **Ventajas:**

     - Eficiente: los hilos no desperdician CPU mientras esperan.
     - Proporciona una solución robusta para la sincronización de acceso a recursos compartidos.

   - **Desventajas:**
     - Puede haber un aumento en la complejidad del código, especialmente en la gestión de errores.
     - Pueden ocurrir condiciones de bloqueo si no se gestionan adecuadamente.

3. **Semaphores:**

   - **Descripción:** Un semáforo es un contador que se utiliza para controlar el acceso a un recurso compartido. Puede permitir que varios hilos accedan simultáneamente hasta un límite especificado.
   - **Implementación:**

     - Un hilo decrementa el semáforo antes de entrar en la sección crítica y lo incrementa al salir.

   - **Ventajas:**

     - Permiten mayor flexibilidad en el control de acceso a recursos compartidos.
     - Pueden permitir la sincronización entre hilos sin necesariamente usar exclusión mutua total.

   - **Desventajas:**
     - Más difícil de implementar correctamente que los mutexes.
     - Pueden ser propensos a problemas de condición de carrera si no se usan adecuadamente.
     - La lógica puede volverse más compleja, especialmente cuando se utilizan múltiples semáforos.

### Implementación con Pthreads

### Ejecución

```bash
g++ -o histogram 3.cpp -pthread
./histogram <num_threads>
```

### Visualización

![Ejecución](.docs/4_2.png)

## Ejercicio 4.5

### Contexto

### Implementación con Pthreads

### Ejecución

```bash
g++ -o histogram 5.cpp -pthread
./histogram <num_threads>
```

### Visualización

![Ejecución](.docs/4_2.png)
