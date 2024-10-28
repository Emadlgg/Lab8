#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;

const int MAX_BUFFER = 5;    // Capacidad máxima del almacén (buffer).
const int MAX_SILLAS = 3;    // Límite de sillas a ensamblar.

enum Piezas { PATAS, RESPALDO, ASIENTO };
const char* productos[] = { "Patas", "Respaldo", "Asiento" };
const int numProductos = 3;

int buffer[MAX_BUFFER];      // Almacén de piezas
int in = 0, out = 0;         // Índices para productores y consumidores

sem_t vacios, llenos;        // Semáforos de control de buffer
pthread_mutex_t mutex;       // Mutex para protección del buffer

int sillasProducidas = 0;      // Contador de sillas ensambladas
int piezasRestantes[3] = {0};  // Arreglo que mantiene el conteo de piezas sobrantes: patas, respaldos, asientos.
bool produccionTerminada = false; // Controla el estado de la producción

void* productor(void* arg) {
    int id = *(int*)arg;
    int piezaId;
    
    while (true) {
        // Revisar límite de sillas
        pthread_mutex_lock(&mutex);
        if (sillasProducidas >= MAX_SILLAS) {
            produccionTerminada = true;
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        piezaId = rand() % numProductos;  // Seleccionar una pieza al azar

        sem_wait(&vacios);                // Espera hasta que hay espacio en el buffer
        pthread_mutex_lock(&mutex);       // Protege el acceso al buffer

        buffer[in] = piezaId;             // Añade la pieza al buffer
        in = (in + 1) % MAX_BUFFER;
        cout << "Productor " << id << " ha fabricado la pieza " << productos[piezaId] << endl;

        pthread_mutex_unlock(&mutex);
        sem_post(&llenos);               // Indica que hay un item más en el buffer

        sleep(1);                        // Simula el tiempo de producción
    }
    pthread_exit(nullptr); // Termina el hilo
    return nullptr;        // Retorna nullptr para evitar warnings
}

void* consumidor(void* arg) {
    int id = *(int*)arg;
    int piezasNecesarias[3] = {4, 1, 1}; // Requiere 4 patas, 1 respaldo, 1 asiento

    while (true) {
        pthread_mutex_lock(&mutex);
        if (produccionTerminada && sillasProducidas >= MAX_SILLAS) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        int pieza;
        sem_wait(&llenos);               // Espera hasta que haya algo en el buffer
        pthread_mutex_lock(&mutex);       // Protege el acceso al buffer

        pieza = buffer[out];
        piezasRestantes[pieza]++;         // Actualiza piezas restantes
        out = (out + 1) % MAX_BUFFER;
        cout << "Consumidor " << id << " ha tomado la pieza " << productos[pieza] << endl;

        piezasNecesarias[pieza]--;
        if (piezasNecesarias[0] <= 0 && piezasNecesarias[1] <= 0 && piezasNecesarias[2] <= 0) {
            sillasProducidas++;
            cout << "Consumidor " << id << " ha ensamblado una silla. Total sillas: " << sillasProducidas << endl;
            piezasNecesarias[0] = 4;
            piezasNecesarias[1] = 1;
            piezasNecesarias[2] = 1;
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&vacios);               // Indica que hay un espacio libre en el buffer

        sleep(1);                        // Simula el tiempo de ensamblado
    }
    pthread_exit(nullptr); // Termina el hilo
    return nullptr;        // Retorna nullptr para evitar warnings
}

int main() {
    pthread_t productores[2], consumidores[2];
    int ids[2] = {1, 2};

    // Inicialización de semáforos y mutex
    sem_init(&vacios, 0, MAX_BUFFER);  // Semáforo de vacíos inicia en capacidad total del buffer
    sem_init(&llenos, 0, 0);           // Semáforo de llenos inicia en 0
    pthread_mutex_init(&mutex, nullptr);

    // Creación de hilos productores y consumidores
    for (int i = 0; i < 2; i++) {
        pthread_create(&productores[i], nullptr, productor, &ids[i]);
        pthread_create(&consumidores[i], nullptr, consumidor, &ids[i]);
    }

    // Espera la finalización de los hilos
    for (int i = 0; i < 2; i++) {
        pthread_join(productores[i], nullptr);
        pthread_join(consumidores[i], nullptr);
    }

    // Reporte final
    cout << "\n--- Reporte Final ---" << endl;
    cout << "Sillas ensambladas: " << sillasProducidas << endl;
    cout << "Piezas sobrantes en el almacén:" << endl;
    cout << "Patas: " << piezasRestantes[0] << endl;
    cout << "Respaldos: " << piezasRestantes[1] << endl;
    cout << "Asientos: " << piezasRestantes[2] << endl;

    // Destrucción de semáforos y mutex
    sem_destroy(&vacios);
    sem_destroy(&llenos);
    pthread_mutex_destroy(&mutex);

    return 0;
}
