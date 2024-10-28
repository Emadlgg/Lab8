    #include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

// Definir número de nodos y archivos compartidos iniciales
const int NUM_NODOS = 5;
const int NUM_ARCHIVOS_INICIALES = 3;

// Estructura que representa un archivo
struct Archivo {
    std::string nombre;
    bool disponible; // True si el archivo está disponible
};

// Lista de archivos compartidos (protegida por semáforo)
std::vector<Archivo> archivos_compartidos;
sem_t semaforo_archivos;  // Semáforo para proteger el acceso a la lista de archivos

// Función para inicializar archivos compartidos
void inicializarArchivos() {
    for (int i = 0; i < NUM_ARCHIVOS_INICIALES; ++i) {
        archivos_compartidos.push_back({"Archivo_" + std::to_string(i + 1), true});
    }
}

// Función que simula la descarga de un archivo
void descargarArchivo(int nodo_id) {
    sem_wait(&semaforo_archivos); // Bloquear el acceso a la lista de archivos

    // Buscar un archivo disponible
    bool encontrado = false;
    for (auto& archivo : archivos_compartidos) {
        if (archivo.disponible) {
            archivo.disponible = false;
            encontrado = true;
            std::cout << "Nodo " << nodo_id << " descargando " << archivo.nombre << std::endl;
            sleep(1);  // Simula el tiempo de descarga
            std::cout << "Nodo " << nodo_id << " ha descargado " << archivo.nombre << std::endl;
            archivo.disponible = true;  // El archivo vuelve a estar disponible
            break;
        }
    }

    if (!encontrado) {
        std::cout << "Nodo " << nodo_id << " no encontró archivos disponibles para descargar." << std::endl;
    }

    sem_post(&semaforo_archivos); // Liberar el semáforo
}

// Función que simula la subida de un archivo
void subirArchivo(int nodo_id) {
    sem_wait(&semaforo_archivos); // Bloquear el acceso a la lista de archivos

    // Crear un nuevo archivo compartido
    std::string nuevo_archivo = "Archivo_" + std::to_string(rand() % 100 + 10);
    archivos_compartidos.push_back({nuevo_archivo, true});
    std::cout << "Nodo " << nodo_id << " ha subido " << nuevo_archivo << std::endl;

    sem_post(&semaforo_archivos); // Liberar el semáforo
}

// Función que representa las operaciones de cada nodo en la red
void* nodo(void* arg) {
    int nodo_id = *(int*)arg;
    delete (int*)arg; // Liberar memoria

    while (true) {
        int operacion = rand() % 2; // Elegir aleatoriamente entre descargar (0) o subir (1)

        if (operacion == 0) {
            descargarArchivo(nodo_id);
        } else {
            subirArchivo(nodo_id);
        }

        sleep(2);  // Espera antes de realizar la siguiente operación
    }
    pthread_exit(nullptr);
}

int main() {
    srand(time(0)); // Inicializar semilla de números aleatorios

    // Inicializar archivos compartidos
    inicializarArchivos();

    // Inicializar el semáforo
    sem_init(&semaforo_archivos, 0, 1);

    // Crear hilos para cada nodo
    pthread_t nodos[NUM_NODOS];
    for (int i = 0; i < NUM_NODOS; ++i) {
        int* nodo_id = new int(i + 1);
        pthread_create(&nodos[i], nullptr, nodo, nodo_id);
    }

    // Esperar a que los hilos terminen (en este caso, el programa corre indefinidamente)
    for (int i = 0; i < NUM_NODOS; ++i) {
        pthread_join(nodos[i], nullptr);
    }

    // Destruir el semáforo
    sem_destroy(&semaforo_archivos);

    return 0;
}
