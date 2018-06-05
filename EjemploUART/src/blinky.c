/* Copyright 2016, Eric Pernia.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Date: 2016-04-26
 */

/*==================[inclusions]=============================================*/

//#include "blinky.h"   // <= own header (optional)
#include "sapi.h"       // <= sAPI header

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/* Funcion para controlar el led RGB de la placa */
void controlRGB( bool red , bool green , bool blue , int time );
/* Devuelve el valor entero de la tecla que se presiona. Cero si no se presiona ninguna*/
uint8_t leerTeclado( void );
/* Funcion para calcular la potencia de un numero */
uint32_t potencia( int base, int indice );
/*Funcion para recibir un valor entero de hasta 4 cifras por UART. Parametro: puntero al valor del retardo*/
bool recibirIntporUART( uint32_t *ptr );

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/* Funcion para controlar el led RGB de la placa */
void controlRGB(bool red , bool green , bool blue , int time){
  gpioWrite( LEDR, red);
  gpioWrite( LEDG, green);
  gpioWrite( LEDB, blue);

  delay(time);
}

/* Devuelve el valor entero de la tecla que se presiona. Cero si no se presiona ninguna*/
uint8_t leerTeclado(void){
  if(!gpioRead(TEC1)) return 1;
  else  if(!gpioRead(TEC2)) return 2;
        else  if(!gpioRead(TEC3)) return 3;
              else  if(!gpioRead(TEC4)) return 4;
  return 0;
}

/* Funcion para recibir un valor entero de hasta 4 cifras por UART */
bool recibirIntporUART( uint32_t *ptr ){
    /* dato para recibir por uart */
    uint8_t dato;
    /* vector para almacenar los caracteres*/
    uint8_t numero[4];

    /* Se lee la UART*/
    if( uartReadByte( UART_USB, &dato ) ){

      uartWriteString( UART_USB, "\r\n" );
      uartWriteByte( UART_USB, dato );

      /* Control caracter numerico o enter*/
      if((dato < 48 || dato > 57) && dato != 13) uartWriteString( UART_USB, ": Mal ingreso \r\n" );
      /* Si el caracter recibido es valido*/
      else{
        /* Declaracion de i para cargar el vector y j para construir el entero */
        uint8_t i=0, j=0;
        do{
          /* Se almacena el dato recibido convertido a entero en el lugar del vector que le corresponde */
          numero[i] = dato-48;
          
          delay(500);
          /* Se leen los siguientes caracteres numericos */
          if(uartReadByte( UART_USB, &dato ) && ((dato > 47 && dato < 58) || dato == 13) ){
            i++;
            uartWriteByte( UART_USB, dato );
          }

        /* Llega el 'enter' o supera las 4 cifras y termina la recepcion */
       }while( dato != 13 && i < 4 );

        uartWriteByte( UART_USB, 13 );
        uartWriteString( UART_USB, " --> Recibido \r\n" );
        uint32_t recibido=0;

        /* Contruye el entero */
        do{

          recibido = recibido + (numero[j] * potencia(10,--i));
          j++;

         }while(i != 0);

        uartWriteString( UART_USB, "\r\n" );

        /* Se guarda el valor del entero en la direccion de memoria recibida como parametro */
        *ptr = recibido;

      return 1;
    }
  }
  return 0;
}

/* Funcion para calcular la potencia de un numero */
uint32_t potencia(int base, int indice){
  uint32_t acumulador=1;

  for( uint8_t i=0 ; i < indice ; i++) acumulador = acumulador * base;

  return acumulador;
}
/*==================[external functions definition]==========================*/

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void){

   /* ------------- INICIALIZACIONES ------------- */
    uint32_t time_lapse = 50;
    uint8_t startFlag;
   /* Inicializar la placa */
   boardConfig();

   /* Inicializar UART_USB a 115200 baudios */
   uartConfig( UART_USB, 115200 );
   /* Mensaje de bienvenida. Notar que la placa no blinkea hasta que se ingresa un numero */
   uartWriteString( UART_USB, "Inicio \r\n" );
   uartWriteString( UART_USB, "Se espera un numero de 0 a 9999 para setear el tiempo de la secuencia RGB \r\n" );
   uartWriteString( UART_USB, "La secuencia empieza en 50 ms \r\n" );
   uartWriteString( UART_USB, "Presionar una tecla para empezar \r\n" );
   while(!uartReadByte( UART_USB, &startFlag ));

   /* ------------- REPETIR POR SIEMPRE ------------- */
   while(1) {

   	for( int i = 0 ; i < 2 ; i++ ){

   		for( int j = 0 ; j < 2 ; j++ ){

   			for( int k = 0 ; k < 2 ; k++ ){
          /* Si se recibe un valor se imprime el valor recibido */
          if(recibirIntporUART(&time_lapse)) stdioPrintf(UART_USB, "time_lapse = %d ", time_lapse);
          /* No se acepta el valor cero */
   				if(time_lapse != 0) controlRGB(i , j , k, time_lapse);
   			}
   		}
   	}
   }

   /* NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa no es llamado
      por ningun S.O. */
   return 0 ;
}

/*==================[end of file]============================================*/
