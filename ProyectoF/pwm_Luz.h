#ifndef FOCO_H
#define FOCO_H

#include <stdint.h>

// Prototipos de funciones
void ConfigurarPWM1(void);                // Configura el PWM en T2CCP1 para el foco
void ModificarCicloTrabajo1(uint32_t duty_cycle); // Modifica el ciclo de trabajo (0 a 100%)

#endif // FOCO_H
