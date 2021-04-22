#ifndef _SNAKEPILIB_H_
#define _SNAKEPILIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ledDisplay.h"

enum t_direccion {
	ARRIBA,
	DERECHA,
	ABAJO,
	IZQUIERDA,
	NONE,
};

// La serpiente es una lista encadenada de segmentos
// Cada segmento cuenta con una ubicación (coordenadas x e y)
// y un puntero a la direccion del siguiente segmento
typedef struct s_segmento tipo_segmento;

struct s_segmento {
	tipo_segmento *p_next;
	int x;
	int y;
};

// La serpiente siempre tiene al menos un segmento que es la cabeza
// Para dicho segmento es el unico para el que tiene sentido hablar de direccion
// El resto de segmentos irán uno tras otro ocupando sucesivamente
// posiciones de sus predecesores en la cadena
typedef struct {
	tipo_segmento cabeza;
	tipo_segmento *p_cola;
	enum t_direccion direccion;
} tipo_serpiente;

typedef struct {
	int x;
	int y;
} tipo_manzana;

typedef struct {
	tipo_pantalla *p_pantalla; // Esta es nuestra pantalla de juego (matriz 7x8 de labo)
	tipo_serpiente serpiente;
	tipo_manzana manzana;
	tmr_t* tmr_refresco_snake;
} tipo_snakePi;

extern int flags; // Flags generales de sistema (necesario para comunicacion inter-FMs)

//------------------------------------------------------
// PROCEDIMIENTOS DE INICIALIZACION DE LOS OBJETOS ESPECIFICOS
//------------------------------------------------------

void InicializaManzana(tipo_manzana *p_manzana);
void InicializaSerpiente(tipo_serpiente *p_serpiente);
void InicializaSnakePi(tipo_snakePi *p_snakePi);
void ResetSnakePi(tipo_snakePi *p_snakePi);

//------------------------------------------------------
// PROCEDIMIENTOS PARA LA GESTION DEL JUEGO
//------------------------------------------------------

void ActualizaColaSerpiente(tipo_serpiente *p_serpiente);
int  ActualizaLongitudSerpiente(tipo_serpiente *p_serpiente);
int  ActualizaSnakePi(tipo_snakePi *p_snakePi);
void CambiarDireccionSerpiente(tipo_serpiente *serpiente, unsigned int direccion);
int  CompruebaColision(tipo_serpiente *serpiente, tipo_manzana *manzana, int chequea_manzana);
void LiberaMemoriaCola(tipo_serpiente *p_serpiente);

//------------------------------------------------------
// PROCEDIMIENTOS PARA LA VISUALIZACION DEL JUEGO
//------------------------------------------------------

void PintaManzana(tipo_snakePi *p_snakePi);
void PintaSerpiente(tipo_snakePi *p_snakePi);
void ActualizaPantallaSnakePi(tipo_snakePi *p_snakePi);
void ReseteaPantallaSnakePi(tipo_pantalla *p_pantalla);

//------------------------------------------------------
// FUNCIONES DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
int CompruebaBotonPulsado (fsm_t* this);
int CompruebaMovimientoArriba (fsm_t* this);
int CompruebaMovimientoAbajo (fsm_t* this);
int CompruebaMovimientoIzquierda (fsm_t* this);
int CompruebaMovimientoDerecha (fsm_t* this);
int CompruebaTimeoutActualizacionJuego (fsm_t* this);
int CompruebaFinalJuego (fsm_t* this);

//------------------------------------------------------
// FUNCIONES DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
void InicializaJuego (fsm_t* this);
void MueveSerpienteIzquierda (fsm_t* this);
void MueveSerpienteDerecha (fsm_t* this);
void MueveSerpienteArriba (fsm_t* this);
void MueveSerpienteAbajo (fsm_t* this);
void ActualizarJuego (fsm_t* this);
void FinalJuego (fsm_t* this);
void ReseteaJuego (fsm_t* this);

#endif /* _SNAKEPILIB_H_ */
