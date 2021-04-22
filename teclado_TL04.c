#include "teclado_TL04.h"

char tecladoTL04[4][4] = {
	{'1', '2', '3', 'C'},
	{'4', '5', '6', 'D'},
	{'7', '8', '9', 'E'},
	{'A', '0', 'B', 'F'}
};

// Maquina de estados: lista de transiciones
// {EstadoOrigen, CondicionDeDisparo, EstadoFinal, AccionesSiTransicion }
fsm_trans_t fsm_trans_excitacion_columnas[] = {
	{ TECLADO_ESPERA_COLUMNA, CompruebaTimeoutColumna, TECLADO_ESPERA_COLUMNA, TecladoExcitaColumna },
	{-1, NULL, -1, NULL },
};

fsm_trans_t fsm_trans_deteccion_pulsaciones[] = {
	{ TECLADO_ESPERA_TECLA, CompruebaTeclaPulsada, TECLADO_ESPERA_TECLA, ProcesaTeclaPulsada},
	{-1, NULL, -1, NULL },
};

//------------------------------------------------------
// PROCEDIMIENTOS DE INICIALIZACION DE LOS OBJETOS ESPECIFICOS
//------------------------------------------------------

void InicializaTeclado(TipoTeclado *p_teclado) {
	//Primera columna
		if (wiringPiSetupGpio() <0){
			fprintf(stderr, "Unable to setup wiringPi: %s\n", strerror(errno));
			return;
		}

		p_teclado->columna_actual = COLUMNA_1;

		p_teclado->teclaPulsada.col = -1;
		p_teclado->teclaPulsada.row = -1;

		p_teclado->flags = 0;

		p_teclado->debounceTime[FILA_1] = 0;
		p_teclado->debounceTime[FILA_2] = 0;
		p_teclado->debounceTime[FILA_3] = 0;
		p_teclado->debounceTime[FILA_4] = 0;

		p_teclado->tmr_duracion_columna = tmr_new(timer_duracion_columna_isr);

		tmr_startms((tmr_t*)(teclado.tmr_duracion_columna), TIMEOUT_COLUMNA_TECLADO);

		pinMode(teclado.filas[0], INPUT);
		pullUpDnControl(teclado.filas[0], PUD_DOWN);
		wiringPiISR(teclado.filas[0], INT_EDGE_RISING, teclado.rutinas_ISR[0]);

		pinMode(teclado.filas[1], INPUT);
		pullUpDnControl(teclado.filas[1], PUD_DOWN);
		wiringPiISR(teclado.filas[1], INT_EDGE_RISING, teclado.rutinas_ISR[1]);

		pinMode(teclado.filas[2], INPUT);
		pullUpDnControl(teclado.filas[2], PUD_DOWN);
		wiringPiISR(teclado.filas[2], INT_EDGE_RISING, teclado.rutinas_ISR[2]);

		pinMode(teclado.filas[3], INPUT);
		pullUpDnControl(teclado.filas[3], PUD_DOWN);
		wiringPiISR(teclado.filas[3], INT_EDGE_RISING, teclado.rutinas_ISR[3]);

		pinMode(teclado.columnas[0], OUTPUT);
		digitalWrite(teclado.columnas[0], HIGH);

		pinMode(teclado.columnas[1], OUTPUT);
		digitalWrite(teclado.columnas[1], LOW);

		pinMode(teclado.columnas[2], OUTPUT);
		digitalWrite(teclado.columnas[2], LOW);

		pinMode(teclado.columnas[3], OUTPUT);
		digitalWrite(teclado.columnas[3], LOW);

		//printf("\nInicialización completada, Teclado listo\n");
		//fflush(stdout);
}

//------------------------------------------------------
// OTROS PROCEDIMIENTOS PROPIOS DE LA LIBRERIA
//------------------------------------------------------

void ActualizaExcitacionTecladoGPIO (int columna) {
	piLock(KEYBOARD_KEY);
		switch(columna){
		case COLUMNA_1:
			digitalWrite(GPIO_KEYBOARD_COL_4,LOW);
			digitalWrite(GPIO_KEYBOARD_COL_1,HIGH);
			break;
		case COLUMNA_2:
			digitalWrite(GPIO_KEYBOARD_COL_1,LOW);
			digitalWrite(GPIO_KEYBOARD_COL_2,HIGH);
			break;
		case COLUMNA_3:
			digitalWrite(GPIO_KEYBOARD_COL_2,LOW);
			digitalWrite(GPIO_KEYBOARD_COL_3,HIGH);
			break;
		case COLUMNA_4:
			digitalWrite(GPIO_KEYBOARD_COL_3,LOW);
			digitalWrite(GPIO_KEYBOARD_COL_4,HIGH);
		}
		piUnlock(KEYBOARD_KEY);
}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaTimeoutColumna (fsm_t* this) {
	int result = 0;
	TipoTeclado *p_teclado;
	p_teclado = (TipoTeclado*)(this->user_data);

	piLock(KEYBOARD_KEY);
	result = (p_teclado->flags & FLAG_TIMEOUT_COLUMNA_TECLADO);
	piUnlock(KEYBOARD_KEY);

	return result;
}

int CompruebaTeclaPulsada (fsm_t* this) {
	int result = 0;
	TipoTeclado *p_teclado;
	p_teclado = (TipoTeclado*)(this->user_data);

	piLock (KEYBOARD_KEY);
	result = (p_teclado->flags & FLAG_TECLA_PULSADA);
	piUnlock (KEYBOARD_KEY);
	return result;
}

//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LAS MAQUINAS DE ESTADOS
//------------------------------------------------------

void TecladoExcitaColumna (fsm_t* this) {
	TipoTeclado *p_teclado;
	p_teclado = (TipoTeclado*)(this->user_data);

	p_teclado->columna_actual=p_teclado->columna_actual+1;
	if (p_teclado->columna_actual==4) {
		p_teclado->columna_actual=0;
	}
	ActualizaExcitacionTecladoGPIO(p_teclado->columna_actual);
	tmr_startms((tmr_t*)(teclado.tmr_duracion_columna),TIMEOUT_COLUMNA_TECLADO);
}

void ProcesaTeclaPulsada (fsm_t* this) {
	TipoTeclado *p_teclado;
	p_teclado = (TipoTeclado*)(this->user_data);

	piLock (KEYBOARD_KEY);
	p_teclado->flags &= (~FLAG_TECLA_PULSADA);
	switch(p_teclado->teclaPulsada.col){
	case COLUMNA_1:
		if (p_teclado->teclaPulsada.row == FILA_2) {
			// Caso tecla '4'
			piLock(STD_IO_BUFFER_KEY);
			//printf("\nKeypress 4!!!\n");
			//fflush(stdout);
			piUnlock(STD_IO_BUFFER_KEY);
			piLock(SYSTEM_FLAGS_KEY);
			flags |= FLAG_MOV_IZQUIERDA;
			piUnlock(SYSTEM_FLAGS_KEY);
		}
		if (p_teclado->teclaPulsada.row == FILA_4) {
			// Caso tecla 'A'
			piLock(STD_IO_BUFFER_KEY);
			//printf("\nKeypress A!!!\n");
			printf("\nFIN DE JUEGO\n");
			fflush(stdout);
			piUnlock(STD_IO_BUFFER_KEY);
			piLock(SYSTEM_FLAGS_KEY);
			flags |= FLAG_FIN_JUEGO;
			piUnlock(SYSTEM_FLAGS_KEY);
		}
	break;
	case COLUMNA_2:
		if (p_teclado->teclaPulsada.row == FILA_1) {
			// Caso tecla '2'
			piLock(STD_IO_BUFFER_KEY);
			//printf("\nKeypress 2!!!\n");
			//fflush(stdout);
			piUnlock(STD_IO_BUFFER_KEY);
			piLock(SYSTEM_FLAGS_KEY);
			flags |= FLAG_MOV_ARRIBA;
			piUnlock(SYSTEM_FLAGS_KEY);
		}
		if (p_teclado->teclaPulsada.row == FILA_3) {
			// Caso tecla '8'
			piLock(STD_IO_BUFFER_KEY);
			//printf("\nKeypress 8!!!\n");
			//fflush(stdout);
			piUnlock(STD_IO_BUFFER_KEY);
			piLock(SYSTEM_FLAGS_KEY);
			flags |= FLAG_MOV_ABAJO;
			piUnlock(SYSTEM_FLAGS_KEY);
		}
	break;
	case COLUMNA_3:
		if (p_teclado->teclaPulsada.row == FILA_2) {
			// Caso tecla '6'
			piLock(STD_IO_BUFFER_KEY);
			//printf("\nKeypress 6!!!\n");
			//fflush(stdout);
			piUnlock(STD_IO_BUFFER_KEY);
			piLock(SYSTEM_FLAGS_KEY);
			flags |= FLAG_MOV_DERECHA;
			piUnlock(SYSTEM_FLAGS_KEY);
		}
	break;
	case COLUMNA_4:
		if (p_teclado->teclaPulsada.row == FILA_4) {
			// Caso tecla 'F'
			piLock(STD_IO_BUFFER_KEY);
			//printf("\nKeypress F!!!\n");
			//printf("\nEmpieza el juego\n");
			//fflush(stdout);
			piUnlock(STD_IO_BUFFER_KEY);
			piLock(SYSTEM_FLAGS_KEY);
			flags |= FLAG_BOTON;
			piUnlock(SYSTEM_FLAGS_KEY);

		}
	break;
	default:
		piLock(STD_IO_BUFFER_KEY);
		//printf("\nInvalid key\n");
		//fflush(stdout);
		piUnlock(STD_IO_BUFFER_KEY);
		p_teclado->teclaPulsada.row=-1;
		p_teclado->teclaPulsada.col=-1;
	break;
	}
	piUnlock (KEYBOARD_KEY);
}


//------------------------------------------------------
// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------
unsigned long debounceTime;
void teclado_fila_1_isr (void) {

	if (millis () < teclado.debounceTime[FILA_1]) {
	 teclado.debounceTime[FILA_1] = millis () + DEBOUNCE_TIME ;
	 return;
	}
	piLock (KEYBOARD_KEY);
	teclado.teclaPulsada.row = FILA_1;
	teclado.teclaPulsada.col = teclado.columna_actual;
	teclado.flags |= FLAG_TECLA_PULSADA;
	piUnlock (KEYBOARD_KEY);
	teclado.debounceTime[FILA_1] = millis () + DEBOUNCE_TIME ;
}

void teclado_fila_2_isr (void) {

	if (millis () < teclado.debounceTime[FILA_2]) {
		 teclado.debounceTime[FILA_2] = millis () + DEBOUNCE_TIME ;
		 return;
		}
		piLock (KEYBOARD_KEY);
		teclado.teclaPulsada.row = FILA_2;
		teclado.teclaPulsada.col = teclado.columna_actual;
		teclado.flags |= FLAG_TECLA_PULSADA;
		piUnlock (KEYBOARD_KEY);
		teclado.debounceTime[FILA_2] = millis () + DEBOUNCE_TIME ;
}

void teclado_fila_3_isr (void) {

	if (millis () < teclado.debounceTime[FILA_3]) {
		 teclado.debounceTime[FILA_3] = millis () + DEBOUNCE_TIME ;
		 return;
		}
		piLock (KEYBOARD_KEY);
		teclado.teclaPulsada.row = FILA_3;
		teclado.teclaPulsada.col = teclado.columna_actual;
		teclado.flags |= FLAG_TECLA_PULSADA;
		piUnlock (KEYBOARD_KEY);
		teclado.debounceTime[FILA_3] = millis () + DEBOUNCE_TIME ;
}

void teclado_fila_4_isr (void) {

	if (millis () < teclado.debounceTime[FILA_4]) {
		 teclado.debounceTime[FILA_4] = millis () + DEBOUNCE_TIME ;
		 return;
		}
		piLock (KEYBOARD_KEY);
		teclado.teclaPulsada.row = FILA_4;
		teclado.teclaPulsada.col = teclado.columna_actual;
		teclado.flags |= FLAG_TECLA_PULSADA;
		piUnlock (KEYBOARD_KEY);
		teclado.debounceTime[FILA_4] = millis () + DEBOUNCE_TIME ;
}

void timer_duracion_columna_isr (union sigval value) {

	piLock(KEYBOARD_KEY);
	teclado.flags |= FLAG_TIMEOUT_COLUMNA_TECLADO;
	piUnlock(KEYBOARD_KEY);
}
