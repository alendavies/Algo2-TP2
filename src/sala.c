#include "estructuras.h"
#include "sala.h"
#include "objeto.h"
#include "interaccion.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lista.h"

#define MAX_LINEA 1024

typedef struct sala {
	lista_t *objetos;
	lista_t *interacciones;
} sala_t;

sala_t *sala_crear_desde_archivos(const char *objetos, const char *interacciones)
{
	if(!objetos || !interacciones){
		return NULL;
	}

	FILE *archivo_objetos = fopen(objetos, "r");
	if(!archivo_objetos){
		return NULL;
	}

	FILE *archivo_interacciones = fopen(interacciones, "r");
	if (!archivo_interacciones){
		fclose(archivo_objetos);
		return NULL;
	}

	char linea[MAX_LINEA];

	sala_t *sala = malloc(sizeof(sala_t));

	if(!sala){
		fclose(archivo_objetos);
		fclose(archivo_interacciones);
		free(sala);
		return NULL;
	}

	lista_t *lista_objetos = malloc(sizeof(lista_t));

	while(fgets(linea, MAX_LINEA, archivo_objetos) != NULL){

		struct objeto *objeto = objeto_crear_desde_string(linea);
		if(!objeto){
			fclose(archivo_objetos);
			fclose(archivo_interacciones);
			sala_destruir(sala);
			return NULL;
		}
		lista_objetos = lista_insertar(lista_objetos, objeto);

		if (!lista_objetos){
			fclose(archivo_objetos);
			fclose(archivo_interacciones);
			sala_destruir(sala);
			return NULL;
		}
	}

	sala->objetos = lista_objetos;
	
	lista_t *lista_interacciones = malloc(sizeof(lista_t));

	while( fgets( linea, MAX_LINEA, archivo_interacciones ) != NULL ) {

		struct interaccion *interaccion = interaccion_crear_desde_string(linea);
		if(!interaccion){
			fclose(archivo_objetos);
			fclose(archivo_interacciones);
			sala_destruir(sala);
			return NULL;
		}
		lista_interacciones = lista_insertar(lista_interacciones, interaccion);

		if (!lista_interacciones){
			fclose(archivo_objetos);
			fclose(archivo_interacciones);
			sala_destruir(sala);
			return NULL;
		}
	}

	sala->interacciones = lista_interacciones;

	fclose(archivo_objetos);
	fclose(archivo_interacciones);

	if(sala->interacciones->cantidad == 0){
		sala_destruir(sala);
		return NULL;
	}

	return sala;
}

char **sala_obtener_nombre_objetos(sala_t *sala, int *cantidad)
{
	if (sala == NULL) {
		*cantidad = -1;
		return NULL;
	}

	if (cantidad == NULL) {
		int i = 0;
		cantidad = &i;
	}

	*cantidad = (int)sala->objetos->cantidad;
	char **nombres_objetos = malloc( (unsigned) (*cantidad) * ( sizeof(char *) ) );

	if (nombres_objetos == NULL) {
		free(nombres_objetos);
		*cantidad = -1;
		return NULL;
	}

	for( int i = 0; i < *cantidad; i++ ) {
		struct objeto *objeto = (struct objeto*)lista_elemento_en_posicion(sala->objetos, (size_t)i);
		nombres_objetos[i] = objeto->nombre;
	}

	return nombres_objetos;
}

bool sala_es_interaccion_valida(sala_t *sala, const char *verbo, const char *objeto1, const char *objeto2)
{
	if(!sala || !verbo || !objeto1 || !objeto2){
		return NULL;
	}

	for(int i = 0; i < sala->interacciones->cantidad; i++){

		struct interaccion *interaccion = (struct interaccion *)lista_elemento_en_posicion(sala->interacciones, (size_t)i);

		if(!strcmp(interaccion->verbo, verbo) &&
		!strcmp(interaccion->objeto, objeto1) &&
		!strcmp(interaccion->objeto_parametro, objeto2)){
		 	return true;
		}
			
	}
	return false;
}

void sala_destruir(sala_t *sala)
{
	if(!sala){
		return;
	}
	lista_destruir(sala->objetos);
	lista_destruir(sala->interacciones);
	free(sala);
}