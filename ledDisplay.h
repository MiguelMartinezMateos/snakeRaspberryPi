#ifndef _LEDDISPLAY_H_
#define _LEDDISPLAY_H_

#include "systemLib.h"
#include "pseudoWiringPi.h" //////


// REFRESCO DISPLAY
#define TIMEOUT_COLUMNA_DISPLAY 15

#define NUM_COLUMNAS_DISPLAY	8
#define NUM_FILAS_DISPLAY		7
#define NUM_COL_FOR				2

// FLAGS FSM CONTROL DE EXCITACION DISPLAY
// ATENCION: Valores a modificar por el alumno
#define FLAG_TIMEOUT_COLUMNA_DISPLAY 0x200 //

enum estados_excitacion_display_fsm {
	DISPLAY_ESPERA_COLUMNA
};

typedef struct {
	int matriz[NUM_COLUMNAS_DISPLAY][NUM_FILAS_DISPLAY];
} tipo_pantalla;

typedef struct {
	int columnas[NUM_COLUMNAS_DISPLAY]; // Array con los valores BCM de los pines GPIO empleados para cada columna
	int filas[NUM_FILAS_DISPLAY]; // Array con los valores BCM de los pines GPIO empleados para cada fila
	int columna_actual; // Variable que almacena el valor de la columna que esta activa
	tipo_pantalla pantalla; // Objeto que almacena el estado encendido o apagado de cada uno de los leds de la matriz
	tmr_t* tmr_refresco_display; // Temporizador responsable de medir el tiempo de activacion de cada columna
	int flags; // Variable para gestion de flags especificamente ligados a la gestion del display
} TipoLedDisplay;

extern TipoLedDisplay led_display;
extern tipo_pantalla pantalla_inicial;
extern tipo_pantalla pantalla_final;
extern fsm_trans_t fsm_trans_excitacion_display[];

//------------------------------------------------------
// PROCEDIMIENTOS DE INICIALIZACION DE LOS OBJETOS ESPECIFICOS
//------------------------------------------------------

void InicializaLedDisplay (TipoLedDisplay *led_display);

//------------------------------------------------------
// OTROS PROCEDIMIENTOS PROPIOS DE LA LIBRERIA
//------------------------------------------------------

void ApagaFilas (TipoLedDisplay *led_display);
void ExcitaColumnas (int columna);
void ActualizaLedDisplay (TipoLedDisplay *led_display);
void PintaPantallaPorTerminal (tipo_pantalla *p_pantalla);

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaTimeoutColumnaDisplay (fsm_t* this);

//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

void ActualizaExcitacionDisplay (fsm_t* this);

//------------------------------------------------------
// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------

void timer_refresco_display_isr (union sigval value);

#endif /* _LEDDISPLAY_H_ */
