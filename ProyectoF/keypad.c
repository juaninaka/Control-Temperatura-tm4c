#include <stdint.h>
#include "inc/tm4c1294ncpdt.h"
#include "keypad.h"

#define ROW_PINS 0x3C  // PN2-PN5 para las filas (0x3C = 0011 1100 en binario)
#define COL_PINS 0xF0  // PK4-PK7 para las columnas (0xF0 = 1111 0000 en binario)
// Mapeo de las teclas (puedes ajustar esto según tu configuración de teclado)
const char keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void setupKeypad(void) {
    // Habilitar el reloj para los puertos N y K
    SYSCTL_RCGCGPIO_R |= 0x00001200;  // Activar reloj para PN (0x1000) y PK (0x200)
    while ((SYSCTL_PRGPIO_R & 0x00001200) == 0);  // Esperar hasta que estén listos

    // Configurar filas (PN2-PN5) como entradas con resistencias pull-up
    GPIO_PORTN_DIR_R &= ~ROW_PINS;  // PN2-PN5 como entradas
    GPIO_PORTN_DEN_R |= ROW_PINS;   // Habilitar pines de PN2-PN5
    GPIO_PORTN_PUR_R |= ROW_PINS;   // Activar resistencias pull-up en PN2-PN5

    // Configurar columnas (PK4-PK7) como salidas
    GPIO_PORTK_DIR_R |= 0xF0;   // PK4-PK7 como salidas
    GPIO_PORTK_DEN_R |= 0xF0;   // Habilitar pines de PK4-PK7
    GPIO_PORTK_DATA_R &= ~0xF0; // Inicialmente todas las columnas desactivadas
}

char getPressedKey(void) {
    int col, row;

    // Escaneo de cada columna
    for (col = 0; col < 4; col++) {
        // Activar únicamente la columna actual
        GPIO_PORTK_DATA_R = ~(1 << (col + 4));  // Establecer la columna activa en 0

        // Leer las filas
        for (row = 2; row < 6; row++) {
            if (!(GPIO_PORTN_DATA_R & (1 << row))) {  // Verificar si la fila está activa
                delay_ms(10);  // Retraso para evitar rebotes

                // Confirmar que la fila sigue activa
                if (!(GPIO_PORTN_DATA_R & (1 << row))) {
                    GPIO_PORTK_DATA_R = 0xFF;  // Desactivar todas las columnas
                    return keymap[row - 2][col];  // Retornar el valor mapeado de la tecla
                }
            }
        }

        // Desactivar todas las columnas después de la iteración
        GPIO_PORTK_DATA_R = 0xFF;
    }

    return '\0';  // Ninguna tecla presionada
}

void delay_ms(int n) {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < 3180; j++) {
            // Ciclo de espera
        }
    }
}

void clearPressedKey(void) {
    // Esperar a que se libere la tecla antes de continuar
    while (getPressedKey() != '\0');

}
