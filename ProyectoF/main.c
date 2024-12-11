#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c1294ncpdt.h"
#include "BMP280_I2C.h"
#include "dht_11.h"
#include <stdio.h>

// Definiciones de pines para periféricos
#define FAN_PIN 0x01         // PE0 (bit 0)
#define LIGHT_PIN 0x02       // PE1 (bit 1)
#define SPRINKLER_PIN 0x04   // PE2 (bit 2)

// Definición de pines para LEDs
#define LED_PN0 0x01         // PN0 (bit 0)
#define LED_PN1 0x02         // PN1 (bit 1)
#define LED_PF0 0x01         // PF0 (bit 0)
#define LED_PF4 0x10         // PF4 (bit 4)

// Prototipos de funciones
void setupSystem(void);
void controlEnvironmentBMP280(float *temp_set, float *humidity_set, float *light_set);
void controlEnvironmentDHT11(float *temp_set, float *humidity_set, float *light_set);
void menu(void);
void resetSystem(void);
void I2C_Init(void);  // Declaración de I2C_Init()

// Variables globales
int sensor_selected = -1;  // -1: Sin seleccionar, 0: BMP280, 1: DHT11
//float last_temp = 0.0;     // Temperatura anterior
int motor_duty_cycle = 0;  // Ciclo de trabajo del ventilador
int light_duty_cycle = 0;  // Ciclo de trabajo del foco

int main(void) {
    // Inicializar I2C y periféricos del BMP280
    I2C_Init();
    setupSystem();

    // Inicializar sensores
    BMP280_I2C_Init();
    DHT_Init();

    // Inicializar PWM
    ConfigurarPWM();
    ConfigurarPWM1();

    // Inicializar teclado
    setupKeypad();

    while (1) {
        // Menú principal y control ambiental
        menu();
    }
}

void setupSystem(void) {
    SYSCTL_RCGCGPIO_R |= 0x00001200;  // Reloj para puertos N y K
    SYSCTL_RCGCGPIO_R |= 0x00000010;  // Reloj para puerto E
    SYSCTL_RCGCGPIO_R |= 0x00000020;  // Reloj para puerto F
    while ((SYSCTL_PRGPIO_R & 0x00001200) == 0) {}  // Esperar puertos N y K
    while ((SYSCTL_PRGPIO_R & 0x00000010) == 0) {}  // Esperar puerto E
    while ((SYSCTL_PRGPIO_R & 0x00000020) == 0) {}  // Esperar puerto F

    // Configurar GPIO para ventilador, foco, aspersor
    GPIO_PORTE_AHB_DIR_R |= (FAN_PIN | LIGHT_PIN | SPRINKLER_PIN);
    GPIO_PORTE_AHB_DEN_R |= (FAN_PIN | LIGHT_PIN | SPRINKLER_PIN);

    // Configurar LEDs en PN y PF
    GPIO_PORTN_DIR_R |= LED_PN0 | LED_PN1;
    GPIO_PORTN_DEN_R |= LED_PN0 | LED_PN1;
    GPIO_PORTF_AHB_DIR_R |= LED_PF0 | LED_PF4;
    GPIO_PORTF_AHB_DEN_R |= LED_PF0 | LED_PF4;
}

void menu(void) {
    static float temp_set = 0, humidity_set = 50.0, light_set = 70.0;
    char key;

    // Menú 1: Selección de sensor
    while (sensor_selected == -1) {
        key = getPressedKey();
        if (key != '\0') {
            switch (key) {
                case '1':  // Seleccionar BMP280
                    sensor_selected = 0;
                    break;
                case '2':  // Seleccionar DHT11
                    sensor_selected = 1;
                    break;
                case '*':  // Reset
                    resetSystem();
                    return;
                default:
                    break;
            }
            clearPressedKey();

            // Esperar a que se libere la tecla antes de continuar
            while (getPressedKey() != '\0');
        }
    }

    // Menú 2: Configuración de variables
    while (1) {
        key = getPressedKey();
        if (key != '\0') {
            switch (key) {
                case '1':  // Configuración preestablecida 1
                    temp_set = 22.0;
                    humidity_set = 50.0;
                    light_set = 70.0;
                    break;
                case '2':  // Configuración preestablecida 2
                    temp_set = 10.0;
                    humidity_set = 70.0;
                    light_set = 50.0;
                    break;
                case '3':  // Configuración personalizada
                    temp_set = 30.0;
                    humidity_set = 55.0;
                    light_set = 60.0;
                    break;
                case '*':  // Resetear al menú principal
                    resetSystem();
                    sensor_selected = -1;
                    return;  // Volver al menú 1
                default:
                    break;
            }
            clearPressedKey();

            // Esperar a que se libere la tecla antes de continuar
            while (getPressedKey() != '\0');
        }

        if (sensor_selected == 0) {
            controlEnvironmentBMP280(&temp_set, &humidity_set, &light_set);
        } else if (sensor_selected == 1) {
            controlEnvironmentDHT11(&temp_set, &humidity_set, &light_set);
        }
    }
}

void controlEnvironmentBMP280(float *temp_set, float *humidity_set, float *light_set) {
    float current_temp = 0, current_pressure = 0, current_humidity = 0;
    float dht_humidity = 0, dht_temp = 0, current_heig = 0;

    // Leer datos del BMP280
    ReadDataBMP280_I2C();
    current_temp = Read_Temp_BMP280_I2C();
    current_pressure = Read_Pres_BMP280_I2C();
    current_heig = Read_Heig_BMP280_I2C(current_pressure); // Esta es "altura",

    // Leer humedad del DHT11
    if (DHT_ReadData(&dht_temp, &dht_humidity) == 0) {
        // Control de humedad usando DHT11
        if (dht_humidity < *humidity_set) {
            GPIO_PORTE_AHB_DATA_R |= SPRINKLER_PIN; // Abrir válvula (aspersor)
            volatile int i;
            for (i = 0; i < 5; i++) {} // Delay aproximado
            GPIO_PORTE_AHB_DATA_R &= ~SPRINKLER_PIN; // Cerrar válvula
        }
    }

    // Control de temperatura
    if (current_temp < *temp_set) {
        light_duty_cycle = light_duty_cycle +20 ;
        if (light_duty_cycle > 100) {light_duty_cycle = 100;}
        ModificarCicloTrabajo1(light_duty_cycle);
        ModificarCicloTrabajo(2);
    } else {
        motor_duty_cycle = motor_duty_cycle +20;
        if (motor_duty_cycle > 100) motor_duty_cycle = 100;
        ModificarCicloTrabajo(motor_duty_cycle);
        ModificarCicloTrabajo1(2);
    }
}


void controlEnvironmentDHT11(float *temp_set, float *humidity_set, float *light_set) {
    float current_temp = 0, current_humidity = 0;

    // Leer datos del DHT11
    if (DHT_ReadData(&current_temp, &current_humidity) == 0) {
        // Control de humedad
        if (current_humidity < *humidity_set) {
             GPIO_PORTE_AHB_DATA_R |= SPRINKLER_PIN; // Abrir válvula (aspersor)
            volatile int i;
            for (i = 0; i < 5; i++) {} // Delay aproximado
            GPIO_PORTE_AHB_DATA_R &= ~SPRINKLER_PIN; // Cerrar válvula
        }

        // Control de temperatura
        if (current_temp < *temp_set) {
            light_duty_cycle = light_duty_cycle + 20;
            if (light_duty_cycle > 100) {light_duty_cycle = 100;}
            ModificarCicloTrabajo1(light_duty_cycle);
            ModificarCicloTrabajo(2);
        } else {
            motor_duty_cycle = motor_duty_cycle +20;
            if (motor_duty_cycle > 100) motor_duty_cycle = 100;
            ModificarCicloTrabajo(motor_duty_cycle);
            ModificarCicloTrabajo1(2);
        }
    }
}

void resetSystem(void) {

    // Apagar todos los dispositivos conectados
    GPIO_PORTN_DATA_R &= ~(LED_PN0 | LED_PN1);       // Apagar LEDs PN
    GPIO_PORTF_AHB_DATA_R &= ~(LED_PF0 | LED_PF4);  // Apagar LEDs PF
    GPIO_PORTE_AHB_DATA_R &= ~(FAN_PIN | LIGHT_PIN | SPRINKLER_PIN); // Apagar ventilador, foco y aspersor

    // Reiniciar variables globales
    sensor_selected = -1;     // Sin sensor seleccionado
    motor_duty_cycle = 0;     // Ciclo de trabajo del motor al 0%
    light_duty_cycle = 0;     // Ciclo de trabajo del foco al 0%
    //last_temp = 0.0;          // Resetear temperatura registrada

    // Reinicializar periféricos
    setupSystem();            // Configurar GPIO y LEDs
    I2C_Init();               // Reinicializar I2C
    BMP280_I2C_Init();        // Reconfigurar el sensor BMP280
    DHT_Init();               // Reconfigurar el sensor DHT11
    ConfigurarPWM();          // Reinicializar PWM para el ventilador
    ConfigurarPWM1();         // Reinicializar PWM para el foco
    setupKeypad();            // Configurar teclado matricial

}
