#include <stdbool.h>
#include <stdint.h>
#include "inc/tm4c1294ncpdt.h"

// Función para inicializar PWM en el Timer 2B (T2CCP1 en PM1)
void ConfigurarPWM1(void) {
    // Habilitar reloj para GPIO M y TIMER 2
    SYSCTL_RCGCGPIO_R |= 0x00000800;  // Habilitar reloj para PORT M
    SYSCTL_RCGCTIMER_R |= 0x04;       // Habilitar reloj para TIMER 2

    // Esperar a que el reloj esté listo
    while ((SYSCTL_PRGPIO_R & 0x00000800) == 0);

    // Configurar GPIO PM1 para función alterna (T2CCP1)
    GPIO_PORTM_AFSEL_R |= 0x02;       // Habilitar función alterna en PM1
    GPIO_PORTM_PCTL_R = 0x00000330;  // Configurar PM1 como T2CCP1
    GPIO_PORTM_DEN_R |= 0x02;         // Habilitar función digital en PM1
    GPIO_PORTM_DIR_R |= 0x02;         // Configurar PM1 como salida
    //GPIO_PORTM_DATA_R = 0x01; // SALIDA A 0

    // Configurar Timer 2B para PWM
    TIMER2_CTL_R &= ~0x100;           // Deshabilitar Timer 2B durante configuración
    TIMER2_CFG_R = 0x04;              // Configurar para 16 bits
    TIMER2_TBMR_R = 0x0A;             // Modo PWM, modo periódico, cuenta hacia abajo

    // Configurar recarga para frecuencia de 10 kHz (suponiendo reloj a 16 MHz)
    TIMER2_TBILR_R = 1600 - 1;        // Valor de recarga (frecuencia de reloj = 16 MHz)
    TIMER2_TBMATCHR_R = 800 - 1;      // Ciclo de trabajo inicial del 50%

    TIMER2_TBPR_R = 0x00;             // Sin preescalador
    TIMER2_CTL_R |= 0x100;            // Habilitar Timer B
}

// Función para modificar el ciclo de trabajo en Timer 2B
void ModificarCicloTrabajo1(uint8_t dutyCycle) {
    //if (dutyCycle > 100) dutyCycle = 100; // Limitar a 100%

    // Calcular el nuevo valor de coincidencia

    uint32_t matchValue2 = (TIMER2_TAILR_R + 1) * (100 - dutyCycle) / 100;
    TIMER2_TBMATCHR_R = matchValue2 - 1;

    //uint32_t matchValue2 = (dutyCycle / 100)*(TIMER2_TBILR_R);

    // Configurar el valor de coincidencia
    //TIMER2_TBMATCHR_R = matchValue2;
}
