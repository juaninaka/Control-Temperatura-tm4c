#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
#include "dht_11.h"

#define DATA_PIN 0x01  // PK0 (pin para comunicación con DHT11)

// Variables globales
volatile uint8_t timer_flag = 0;

// Función para inicializar el GPIO
void GPIO_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x200;  // Habilitar reloj para GPIO K
    while ((SYSCTL_PRGPIO_R & 0x200) == 0);  // Esperar a que el puerto K esté listo

    GPIO_PORTK_DEN_R |= DATA_PIN;  // Habilitar función digital en PK0
}

// Configuración del temporizador
void Timer_Init(void) {
    SYSCTL_RCGCTIMER_R |= 0x01;  // Habilitar reloj para Timer 0
    while ((SYSCTL_PRTIMER_R & 0x01) == 0);  // Esperar a que el temporizador esté listo

    TIMER0_CTL_R &= ~0x01;        // Deshabilitar Timer A durante la configuración
    TIMER0_CFG_R = 0x00;          // Configurar para modo de 32 bits
    TIMER0_TAMR_R = 0x01;         // Modo One-Shot
    TIMER0_IMR_R = 0x01;          // Habilitar interrupción del temporizador
    NVIC_EN0_R |= (1 << 19);      // Habilitar interrupción del Timer 0A
}

// Configurar el temporizador para un retardo en microsegundos
void Timer_DelayUs(uint32_t microseconds) {
    TIMER0_TAILR_R = (16 * microseconds) - 1;  // Configurar el valor de cuenta (16 MHz)
    TIMER0_CTL_R |= 0x01;                     // Habilitar el temporizador
    timer_flag = 0;                           // Reiniciar bandera
    while (!timer_flag);                      // Esperar a que se complete el tiempo
}

// Rutina de interrupción del temporizador
void Timer0A_Handler(void) {
    TIMER0_ICR_R = 0x01;  // Limpiar la bandera de interrupción
    timer_flag = 1;       // Establecer la bandera
}

// Inicialización del DHT11
void DHT_Init(void) {
    GPIO_Init();
    Timer_Init();
}

// Rutina para iniciar la comunicación con el DHT11
void DHT_StartSignal(void) {
    GPIO_PORTK_DIR_R |= DATA_PIN;   // Configurar como salida
    GPIO_PORTK_DATA_R &= ~DATA_PIN; // Poner el pin en bajo
    Timer_DelayUs(18000);           // Mantenerlo en bajo durante 18 ms
    GPIO_PORTK_DATA_R |= DATA_PIN;  // Poner el pin en alto
    Timer_DelayUs(30);              // Mantenerlo en alto durante 20-40 µs
    GPIO_PORTK_DATA_R &= ~DATA_PIN; // Poner el pin en bajo para limpiar
    GPIO_PORTK_DIR_R &= ~DATA_PIN;  // Configurar como entrada para leer
}

// Función para leer los datos del DHT11
int DHT_ReadData(float *temperature, float *humidity) {
    uint8_t data[5] = {0};
    int i, j;

    // Generar señal de inicio
    DHT_StartSignal();

    // Esperar respuesta del DHT11
    if ((GPIO_PORTK_DATA_R & DATA_PIN) == 0) {
        Timer_DelayUs(80);  // Pulso bajo de 80 µs
    } else {
        return -1;  // No hay respuesta
    }

    if ((GPIO_PORTK_DATA_R & DATA_PIN) != 0) {
        Timer_DelayUs(80);  // Pulso alto de 80 µs
    } else {
        return -1;  // No hay respuesta
    }

    // Leer 40 bits de datos
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            while ((GPIO_PORTK_DATA_R & DATA_PIN) == 0);  // Esperar inicio del bit
            Timer_DelayUs(30);  // Verificar duración del pulso
            if ((GPIO_PORTK_DATA_R & DATA_PIN) != 0) {
                data[i] |= (1 << (7 - j));  // Registrar el bit como 1
            }
            while ((GPIO_PORTK_DATA_R & DATA_PIN) != 0);  // Esperar fin del bit
        }
    }

    // Verificar checksum
    if (data[4] != (data[0] + data[1] + data[2] + data[3])) {
        return -2;  // Error en la transmisión
    }

    *humidity = data[0];
    *temperature = data[2];
    return 0;  // Lectura exitosa
}
