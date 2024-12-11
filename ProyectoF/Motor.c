#include <stdbool.h>
#include <stdint.h>
#include "inc/tm4c1294ncpdt.h"

// Función para inicializar PWM en el Timer 3
void ConfigurarPWM(void) {
    // Habilitar reloj para GPIO M2 y TIMER 3
    SYSCTL_RCGCGPIO_R |= 0x00000800;  // Habilita PORT M2
    SYSCTL_RCGCTIMER_R |= 0x08;       // Habilita TIMER 3

    // Esperar a que el reloj esté listo
    while ((SYSCTL_PRGPIO_R & 0x00000800) == 0);

    // Configurar GPIO PM2 para función alterna (T3CCP0)
    GPIO_PORTM_AFSEL_R |= 0x04;       // Función alterna en PM2 bit 2
    GPIO_PORTM_PCTL_R = 0x00000330; //0x00000300-- DIRIGIDO A T3CCP0
    GPIO_PORTM_DEN_R |= 0x04;        // Habilitar función digital en PM2 bit2
    GPIO_PORTM_DIR_R |= 0x04; //bit 2 SALID
    GPIO_PORTM_DATA_R = 0x00; // SALIDA A 0

    // Configurar el temporizador
    TIMER3_CTL_R &= ~0x01;           // Deshabilitar Timer A
    TIMER3_CFG_R = 0x04;             // Configurar para 16 bits
    TIMER3_TAMR_R = 0x0A;            // Modo PWM, modo periódico, cuenta hacia abajo

    // Configurar recarga para frecuencia de 10 kHz (suponiendo reloj a 16 MHz)
    TIMER3_TAILR_R = 1600 - 1;      // Valor de recarga (frecuencia de reloj = 16 MHz)
    TIMER3_TAMATCHR_R = 800 - 1;    // Ciclo de trabajo inicial del 50%

    TIMER3_TAPR_R = 0x00;            // Sin preescalador
    TIMER3_CTL_R |= 0x01;            // Habilitar Timer A
}

// Función para modificar el ciclo de trabajo
void ModificarCicloTrabajo(uint8_t dutyCycle) {
    //if (dutyCycle > 100) dutyCycle = 100; // Limitar a 100%

    // Calcular el nuevo valor de coincidencia
    //uint32_t matchValue = (dutyCycle / 100)*(TIMER3_TAILR_R);
    uint32_t matchValue = (TIMER3_TAILR_R + 1) * (100 - dutyCycle) / 100;
    TIMER3_TAMATCHR_R = matchValue - 1;

    // Configurar el valor de coincidencia
    //TIMER3_TAMATCHR_R = matchValue;
}

