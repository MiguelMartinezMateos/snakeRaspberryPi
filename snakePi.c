#include "snakePi.h"

int flags = 0;

// Declaracion del objeto teclado
TipoTeclado teclado = {
	.columnas = {

			GPIO_KEYBOARD_COL_1,
			GPIO_KEYBOARD_COL_2,
			GPIO_KEYBOARD_COL_3,
			GPIO_KEYBOARD_COL_4
	},
	.filas = {

			GPIO_KEYBOARD_ROW_1,
			GPIO_KEYBOARD_ROW_2,
			GPIO_KEYBOARD_ROW_3,
			GPIO_KEYBOARD_ROW_4
	},
	.rutinas_ISR = {
			teclado_fila_1_isr,
			teclado_fila_2_isr,
			teclado_fila_3_isr,
			teclado_fila_4_isr
	},
};

// Declaracion del objeto display
TipoLedDisplay led_display = {
	.columnas = {
			GPIO_LED_DISPLAY_COL_1,
			GPIO_LED_DISPLAY_COL_2,
			GPIO_LED_DISPLAY_COL_3,
			//GPIO_LED_DISPLAY_COL_4
	},
	.filas = {

			GPIO_LED_DISPLAY_ROW_1,
			GPIO_LED_DISPLAY_ROW_2,
			GPIO_LED_DISPLAY_ROW_3,
			GPIO_LED_DISPLAY_ROW_4,
			GPIO_LED_DISPLAY_ROW_5,
			GPIO_LED_DISPLAY_ROW_6,
			GPIO_LED_DISPLAY_ROW_7
	},
};



//------------------------------------------------------
// FUNCIONES DE CONFIGURACION/INICIALIZACION
//------------------------------------------------------

// int ConfiguracionSistema (TipoSistema *p_sistema): procedimiento de configuracion
// e inicializacion del sistema.
// Realizará, entra otras, todas las operaciones necesarias para:
// configurar el uso de posibles librerías (e.g. Wiring Pi),
// configurar las interrupciones externas asociadas a los pines GPIO,
// configurar las interrupciones periódicas y sus correspondientes temporizadores,
// la inicializacion de los diferentes elementos de los que consta nuestro sistema,
// crear, si fuese necesario, los threads adicionales que pueda requerir el sistema
// como el thread de exploración del teclado del PC

int ConfiguraInicializaSistema (TipoSistema *p_sistema) {
	int result = 0;

	if(wiringPiSetupGpio()<0){
		piLock(STD_IO_BUFFER_KEY);
		printf("unable to setup wiringPi");
		piUnlock(STD_IO_BUFFER_KEY);
		return -1;
	}

	p_sistema->snakePi.p_pantalla = &(led_display.pantalla);
	InicializaTeclado(&(teclado));
	InicializaLedDisplay(&(led_display));

	tmr_startms((tmr_t*)(p_sistema->snakePi.tmr_refresco_snake), TIMEOUT_ACTUALIZA_JUEGO);

	// Lanzamos thread para exploracion del teclado convencional del PC
	//result = piThreadCreate (thread_explora_teclado_PC);

	/*if (result != 0) {
		printf ("Thread didn't start!!!\n");
		return -1;
	}*/

	return result;
}

//------------------------------------------------------
// FUNCIONES LIGADAS A THREADS ADICIONALES
//------------------------------------------------------

PI_THREAD (thread_explora_teclado_PC) {
	int teclaPulsada;

	while(1) {
		delay(10); // Wiring Pi function: pauses program execution for at least 10 ms

		piLock (STD_IO_BUFFER_KEY);

		if(kbhit()) {
			teclaPulsada = kbread();

			switch(teclaPulsada) {

				case 's':
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_BOTON;
					piUnlock (SYSTEM_FLAGS_KEY);
					printf("Tecla S pulsada!\n");
					fflush(stdout);
					break;

				case 'x':
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_ABAJO;
					piUnlock (SYSTEM_FLAGS_KEY);
					printf("Tecla X pulsada!\n");
					fflush(stdout);
					break;

				case 'd':
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_DERECHA;
					piUnlock (SYSTEM_FLAGS_KEY);
					printf("Tecla D pulsada!\n");
					fflush(stdout);
					break;

				case 'a':
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_IZQUIERDA;
					piUnlock (SYSTEM_FLAGS_KEY);
					printf("Tecla A pulsada!\n");
					fflush(stdout);
					break;

				case 'w':
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_ARRIBA ;
					piUnlock (SYSTEM_FLAGS_KEY);
					printf("Tecla W pulsada!\n");
					fflush(stdout);
					break;

				case 'q':
					exit(0);
					break;

				default:
					printf("INVALID KEY!!!\n");
					break;
			}
		}

		piUnlock (STD_IO_BUFFER_KEY);
	}
}

// wait until next_activation (absolute time)
void delay_until (unsigned int next) {
	unsigned int now = millis();
	if (next > now) {
		delay (next - now);
	}
}

void timer_isr (union sigval value) {
	piLock(SYSTEM_FLAGS_KEY);
	flags |= FLAG_TIMER_JUEGO;
	piUnlock(SYSTEM_FLAGS_KEY);
}

int main () {
	TipoSistema sistema;
	unsigned int next;
	sistema.snakePi.tmr_refresco_snake = tmr_new (timer_isr);

	fsm_trans_t snakePi[] = {
		{ WAIT_START, CompruebaBotonPulsado, WAIT_PUSH, InicializaJuego },
		{ WAIT_PUSH, CompruebaTimeoutActualizacionJuego, WAIT_PUSH, ActualizarJuego },
		{ WAIT_PUSH, CompruebaMovimientoArriba, WAIT_PUSH, MueveSerpienteArriba },
		{ WAIT_PUSH, CompruebaMovimientoAbajo, WAIT_PUSH, MueveSerpienteAbajo },
		{ WAIT_PUSH, CompruebaMovimientoIzquierda, WAIT_PUSH, MueveSerpienteIzquierda },
		{ WAIT_PUSH, CompruebaMovimientoDerecha, WAIT_PUSH, MueveSerpienteDerecha },
		{ WAIT_PUSH, CompruebaFinalJuego, WAIT_END, FinalJuego },
		{ WAIT_END, CompruebaBotonPulsado, WAIT_START, ReseteaJuego },
		{-1, NULL, -1, NULL },
	};

	// Configuracion e inicializacion del sistema
	ConfiguraInicializaSistema (&sistema);

	fsm_t* snakePi_fsm = fsm_new (WAIT_START, snakePi, &(sistema.snakePi));
	fsm_t* teclado_fsm = fsm_new(TECLADO_ESPERA_COLUMNA, fsm_trans_excitacion_columnas, &(teclado));
	fsm_t* tecla_fsm = fsm_new(TECLADO_ESPERA_TECLA, fsm_trans_deteccion_pulsaciones, &(teclado));
	fsm_t* display_fsm = fsm_new(DISPLAY_ESPERA_COLUMNA, fsm_trans_excitacion_display, &(led_display));
	tmr_t* tmr = tmr_new (timer_isr);

	next = millis();
	while (1) {
		fsm_fire (snakePi_fsm);
		fsm_fire (teclado_fsm);
		fsm_fire (tecla_fsm);
		fsm_fire (display_fsm);
		next += CLK_MS;
		delay_until (next);
	}

	fsm_destroy (snakePi_fsm);
	tmr_destroy ((tmr_t*)(snakePi_fsm->user_data));
	fsm_destroy (teclado_fsm);
	fsm_destroy (tecla_fsm);
	fsm_destroy (display_fsm);
	return 0;
}
