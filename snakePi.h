#ifndef _SNAKEPI_H_
#define _SNAKEPI_H_

#include "systemLib.h"
#include "teclado_TL04.h"
#include "ledDisplay.h"
#include "snakePiLib.h"

typedef struct {
	tipo_snakePi snakePi;
	int debug;
	TipoLedDisplay led_display;
	TipoTeclado teclado;
} TipoSistema;

//------------------------------------------------------
// FUNCIONES DE CONFIGURACION/INICIALIZACION
//------------------------------------------------------

int ConfiguraInicializaSistema (TipoSistema *p_sistema);

//------------------------------------------------------
// FUNCIONES LIGADAS A THREADS ADICIONALES
//------------------------------------------------------

PI_THREAD(thread_explora_teclado_PC);

#endif /* SNAKEPI_H_ */
