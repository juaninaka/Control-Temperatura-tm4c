#ifndef DHT_11_H_
#define DHT_11_H_

#include <stdint.h>
#include "inc/tm4c1294ncpdt.h"



// Funciones principales
void DHT_Init(void); // Inicializa el puerto GPIO para el DHT11
int DHT_ReadData(float *temperature, float *humidity); // Lee datos del DHT11

#endif /* DHT_11_H_ */
