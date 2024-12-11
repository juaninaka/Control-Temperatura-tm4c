#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

// Prototipos de funciones
void ConfigurarPWM(void);                // Configura el PWM
void ModificarCicloTrabajo(uint32_t duty_cycle); // Modifica el ciclo de trabajo (0 a 100%)

#endif // MOTOR_H
