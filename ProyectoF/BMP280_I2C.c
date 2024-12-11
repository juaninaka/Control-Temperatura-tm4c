/* Programa que configura con interfaz I2C al sensor BMP280 de
 * temperatura y presión para medir altura.
 * PB2 -> I2C0-SCL
 * PB3 -> I2C0-SDA
 */
//******************Declaración de Librerias ***********************//
#include<stdint.h>
#include<stdbool.h>
#include <math.h>
#include "inc/tm4c1294ncpdt.h"
#include "BMP280_I2C.h"
//******************************************************************//
//********************Declaración de variables**********************//
//32 bits//
int32_t t_fine,adc_P, adc_T;
uint32_t ID, Config, Meas;
int TPR=7;

//16 bits//
int16_t dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
uint16_t dig_T1, dig_P1;
int16_t Dato[24];
int16_t D[6];
int16_t D_Aux[88];

//8 bits
uint8_t error;
//******************************************************************//
//*****************************Funcion Esperar**********************//
int esperar(){
    uint32_t i;
    for(i=0;i<300;i++){} //Delay
    while(I2C0_MCS_R&0x00000001){}; //Espero a que la transmisión acabe
      if(I2C0_MCS_R&0x00000002==1){ //¿Hubo error?
          error=1;
          return error;
      };
    return 0;
}
//******************************************************************//
//*****************Configuración I2C en la TIVA*********************//
void I2C_Init(void){
     SYSCTL_RCGCI2C_R |= 0x0001;                                    //Reloj de I2C0
     SYSCTL_RCGCGPIO_R |= 0x0002;                                   //Reloj GPIO-B
     while((SYSCTL_PRGPIO_R&0x0002) == 0){};                        //Espero a que se active el Reloj
     GPIO_PORTB_AHB_AFSEL_R |= 0x0C;                                //Función alterna de PB2 y PB3
     GPIO_PORTB_AHB_ODR_R |= 0x08;                                  //OPEN DRAIN para PB3
     GPIO_PORTB_AHB_DIR_R |= 0x0C;                                  //PB2 y PB3 como OUTPUT
     GPIO_PORTB_AHB_DEN_R |= 0x0C;                                  //Función digital de PB3 y PB2
     GPIO_PORTB_AHB_PCTL_R|=0x00002200;                             //PB2 y PB3 como I2C0
     I2C0_MCR_R = 0x00000010;                                       //Función MASTER para el I2C0
     I2C0_MTPR_R = TPR;                                             //Velocidad estándar de 100kbps
     I2C0_MICR_R= 0X00000040;                                        //Limpia Bandera de interrupcion
     I2C0_MIMR_R=0X00000040;                                         //Interrupcion por stop
     //Interrupcion #8
    // NVIC_EN0_R |= 1<<8;
     NVIC_PRI2_R |=0X00000020;

}
//******************************************************************//
//*****************Configuración del BMP280 como SLAVE**************//
void BMP280_I2C_Init(){
    while(I2C0_MCS_R&0x00000001){};                                 //Espera al I2C
    //Para transmitir
    I2C0_MSA_R=(Ad_BMP280<<1)&0xFE;                                 //Carga direccion BMP280 & escritura Slave
    I2C0_MDR_R=Ad_ctrl_meas&0x0FF;                                  //Carga subdirección del Slave
    I2C0_MCS_R=(I2C_MCS_RUN|I2C_MCS_START);                         //Condición de Start & Run
    esperar();

    I2C0_MDR_R=0x57;                                                // 010 101 11 Envia configuración
    I2C0_MCS_R=(I2C_MCS_RUN);
    esperar();

    I2C0_MDR_R= 0x30;                                               // 001 100 00  Envia configuración
    I2C0_MCS_R=(I2C_MCS_STOP|I2C_MCS_RUN);                          //Inicia la ultima transmisión & Stop
    esperar();
}
//******************************************************************//
//***************Funcion que lee todos los registros ***************//
void ReadDataBMP280_I2C(){
    int i;

    while(I2C0_MCS_R&0x00000001){};                                 //Espera al I2C

    //Para actualizar registro para iniciar la lectura
    I2C0_MSA_R=(Ad_BMP280<<1)&0xFE;                                 //Carga la dirección del esclavo
    I2C0_MDR_R=Ad_T1_LSB &0x0FF;                                    //Carga subdirección del Slave
    I2C0_MCS_R=(I2C_MCS_START|I2C_MCS_RUN);                         //Condición de START y RUN
    esperar();

    //Para recibir información
    I2C0_MSA_R=(Ad_BMP280<<1)&0xFE;                                 //Carga direccion BMP280
    I2C0_MSA_R|=0x01;                                               //Esclavo como transmisor
    I2C0_MCS_R=(I2C_MCS_START|I2C_MCS_RUN|I2C_MCS_ACK);             //Condición de RESTART
    esperar();
    Dato[23]=(int16_t)(I2C0_MDR_R&0xFF);                            //Master lee lo que envia el esclavo

    for (i=22; i>=0 ;i--){
        I2C0_MCS_R=(I2C_MCS_RUN|I2C_MCS_ACK);                       //RUN
        esperar();
        Dato[i]=(I2C0_MDR_R&0xFF);                                  //Master lee lo que envia el esclavo
    }
    for (i=86; i>=0;i--){
         I2C0_MCS_R=(I2C_MCS_RUN|I2C_MCS_ACK);                      //RUN
         esperar();
         D_Aux[i]=(I2C0_MDR_R&0xFF);                                //Master lee lo que envia el esclavo
        }
    for (i=5; i>=0;i--){
         I2C0_MCS_R=(I2C_MCS_RUN|I2C_MCS_ACK);                      //RUN
         esperar();
         D[i]=(I2C0_MDR_R&0xFF);                                    //Master lee lo que envia el esclavo
        }

    I2C0_MCS_R=(I2C_MCS_RUN|I2C_MCS_STOP);                          //Stop

    //Construye variables de 16 bits
        dig_T1=(uint32_t)Dato[23];
        dig_T1|=(uint32_t)Dato[22]<<8;

        dig_T2=Dato[21];
        dig_T2|=Dato[20]<<8;

        dig_T3=Dato[19];
        dig_T3|=Dato[18]<<8;

        dig_P1=(uint32_t)Dato[17];
        dig_P1|=(uint32_t)Dato[16]<<8;

        dig_P2=Dato[15];
        dig_P2|=Dato[14]<<8;

        dig_P3=Dato[13];
        dig_P3|=Dato[12]<<8;

        dig_P4=Dato[11];
        dig_P4|=Dato[10]<<8;

        dig_P5=Dato[9];
        dig_P5|=Dato[8]<<8;

        dig_P6=Dato[7];
        dig_P6|=Dato[6]<<8;

        dig_P7=Dato[5];
        dig_P7|=Dato[4]<<8;

        dig_P8=Dato[3];
        dig_P8|=Dato[2]<<8;

        dig_P9=Dato[1];
        dig_P9|=Dato[0]<<8;
    //Construye variables de 20 bits
        adc_T= D[2];
        adc_T= adc_T<<12;
        adc_T|=D[1]<<4;
        adc_T|=D[0]>>4;

        adc_P=D[5];
        adc_P=adc_P<<12;
        adc_P|=D[4]<<4;
        adc_P|=D[3]>>4;
}

//******************************************************************//
//*******Funcion que devuelve la Temperatura en [°C]****************//
float Read_Temp_BMP280_I2C(){
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) *
              ((int32_t)dig_T2)) >> 11;

      var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) *
                ((adc_T >> 4) - ((int32_t)dig_T1))) >>
               12) * ((int32_t)dig_T3)) >> 14;

      t_fine = var1 + var2;

      float T = (t_fine * 5 + 128) >> 8;
      return T / 100;
}
//******************************************************************//
//***********Función que devuelve la presión en [Pa]****************//
float Read_Pres_BMP280_I2C(){
    int64_t var1, var2, p;
      var1 = ((int64_t)t_fine) - 128000;
      var2 = var1 * var1 * (int64_t)dig_P6;
      var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
      var2 = var2 + (((int64_t)dig_P4) << 35);
      var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) +
             ((var1 * (int64_t)dig_P2) << 12);
      var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

      if (var1 == 0) {
        return 0; // avoid exception caused by division by zero
      }
      p = 1048576 - adc_P;
      p = (((p << 31) - var2) * 3125) / var1;
      var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
      var2 = (((int64_t)dig_P8) * p) >> 19;

      p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
      return (float)p / 256;
}
//******************************************************************//
//****************Función que devuelve la altura en [m]*************//
float Read_Heig_BMP280_I2C(float P_Ref){
float altitude;
float pres = Read_Pres_BMP280_I2C(); // in Si units for Pascal
pres /= 100;

altitude = 44330 * (1.0 - pow(pres / P_Ref, 0.1903));

return altitude;
}
//******************************************************************//

