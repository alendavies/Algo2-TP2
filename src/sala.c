#include "estructuras.h"
#include "sala.h"
#include "objeto.h"
#include "interaccion.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lista.h"

typedef struct sala {
	lista_t *objetos;
	lista_t *interacciones;
	bool escape_exitoso;
} sala_t;

#define MAX_CARACTERES 1024
#define CANT_OBJETOS 20
#define ERROR -1

lista_t *crear_lista_objetos(FILE *archivo)
{
	char linea[MAX_CARACTERES];
	lista_t *lista_objetos = lista_crear();
	if(!lista_objetos){
		return NULL;
	}
	
	while(fgets(linea, MAX_CARACTERES, archivo) != NULL){
		
		struct objeto *objeto = objeto_crear_desde_string(linea);
		if(!objeto){
			lista_destruir(lista_objetos);
			return NULL;
		}
		lista_objetos = lista_insertar(lista_objetos, objeto);
		if(!lista_objetos){
			free(objeto);
			lista_destruir(lista_objetos);
			return NULL;
		}	
	}
	return lista_objetos;
}

lista_t *crear_lista_interacciones(FILE *archivo)
{
	char linea[MAX_CARACTERES];
	lista_t *lista_interacciones = lista_crear();
	if(!lista_interacciones){
		return NULL;
	}
	
	while(fgets(linea, MAX_CARACTERES, archivo) != NULL){
		
		struct interaccion *interaccion = interaccion_crear_desde_string(linea);
		if(!interaccion){
			lista_destruir(lista_interacciones);
			return NULL;
		}
		lista_interacciones = lista_insertar(lista_interacciones, interaccion);
		if(!lista_interacciones){
			free(interaccion);
			lista_destruir(lista_interacciones);
			return NULL;
		}	
	}
	return lista_interacciones;
}

sala_t *sala_crear_desde_archivos(const char *objetos, const char *interacciones)
{	
	if(!objetos || !interacciones){
		return NULL;
	}

	FILE *arch_objetos = fopen(objetos, "r");
	if(!arch_objetos){
        	return NULL;
	}

	FILE *arch_interacciones = fopen(interacciones, "r");
	if(!arch_interacciones){
		fclose(arch_objetos);
        	return NULL;
	}

	sala_t *sala = calloc(1, sizeof(struct sala));
	if(!sala){
		fclose(arch_objetos);
		fclose(arch_interacciones);
		return NULL;
	}
	
	lista_t *lista_objetos = crear_lista_objetos(arch_objetos);
	if(!lista_objetos){
		fclose(arch_objetos);
		fclose(arch_interacciones);
		sala_destruir(sala);
		return NULL;
	}
	sala->objetos = lista_objetos;
	fclose(arch_objetos);

	lista_t *lista_interaccion = crear_lista_interacciones(arch_interacciones);
	if(!lista_interaccion){
		fclose(arch_interacciones);
		sala_destruir(sala);
		return NULL;
	}
	sala->interacciones = lista_interaccion;
	fclose(arch_interacciones);
	
	if(sala->interacciones->cantidad == 0 || sala->objetos->cantidad == 0){
		return NULL;
	}
	
	return sala;
}

char **sala_obtener_nombre_objetos(sala_t *sala, int *cantidad)
{
	if(!sala){
		*cantidad = ERROR;
		return NULL;
	}

	if(!cantidad){
		int i = 0;
		cantidad = &i;
	}

	*cantidad = (int)sala->objetos->cantidad;
	char **nombres_objetos = malloc((unsigned)(*cantidad) * (sizeof(char *)));

	if(!nombres_objetos) {
		*cantidad = ERROR;
		return NULL;
	}

	for(int i = 0; i < *cantidad; i++){
		struct objeto *objeto = (struct objeto*)lista_elemento_en_posicion(sala->objetos, (size_t)i);
		if(!objeto){
			free(nombres_objetos);
			return NULL;
		}
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

		if(strcmp(interaccion->verbo, verbo) == 0 &&
		strcmp(interaccion->objeto, objeto1) == 0 &&
		strcmp(interaccion->objeto_parametro, objeto2) == 0){
		 	return true;
		}
			
	}
	return false;
}

bool sala_agarrar_objeto(sala_t *sala, const char *nombre_objeto)
{
	if(!sala || !nombre_objeto){
		return false;
	}
	for(int i = 0; i < sala->objetos->cantidad; i++){
		struct objeto *objeto = lista_elemento_en_posicion(sala->objetos, (size_t)i);
		if(!objeto){
			return false;
		}
		if(strcmp(objeto->nombre, nombre_objeto) == 0){
			return objeto->es_asible;
		}
	}
	return false;
}

char* sala_describir_objeto(sala_t* sala, const char *nombre_objeto)
{
	if(!sala || !nombre_objeto){
		return NULL;
	}
	for(int i = 0; i < sala->objetos->cantidad; i++){
		struct objeto *objeto = lista_elemento_en_posicion(sala->objetos, (size_t)i);
		if(!objeto){
			return NULL;
		}
		if(strcmp(objeto->nombre, nombre_objeto) == 0){
			return objeto->descripcion;
		}
	}
	return NULL;
}

int ejecutar_accion(sala_t *sala, struct interaccion *interaccion, const char *objeto1, const char *objeto2, void (*mostrar_mensaje)(const char *mensaje, enum tipo_accion accion, void *aux), void *aux){

	int ejecutadas = 0;
	
	if(interaccion->accion.tipo == MOSTRAR_MENSAJE){
		mostrar_mensaje(interaccion->accion.mensaje, interaccion->accion.tipo, NULL);
		ejecutadas++;
	}
	if(interaccion->accion.tipo == ELIMINAR_OBJETO){
		for(int i = 0; i < sala->objetos->cantidad; i++){
			struct objeto *objeto = lista_elemento_en_posicion(sala->objetos, i);
			if(strcmp(objeto->nombre, objeto1) == 0){
				lista_quitar_de_posicion(sala->objetos, i);
			}
		}
		ejecutadas++;
	}
	if(interaccion->accion.tipo == DESCUBRIR_OBJETO){
		//Conocer objeto
		ejecutadas++;
	}
	if(interaccion->accion.tipo == ESCAPAR){
		sala->escape_exitoso = true;
		ejecutadas++;
	}
	if(interaccion->accion.tipo == REEMPLAZAR_OBJETO){
		for(int i = 0; i < sala->objetos->cantidad; i++){
			struct objeto *objeto_parametro = lista_elemento_en_posicion(sala->objetos, i);
			if(strcmp(objeto_parametro->nombre, objeto2) == 0){
				lista_quitar_de_posicion(sala->objetos, i);
			}
		}
		//descubrir nuevo objeto
		ejecutadas++;
	}
	return ejecutadas;
}

int sala_ejecutar_interaccion(sala_t *sala, const char *verbo, const char *objeto1, const char *objeto2, void (*mostrar_mensaje)(const char *mensaje, enum tipo_accion accion, void *aux), void *aux)
{
	if(!sala || !verbo || !objeto1 || !objeto2 || !mostrar_mensaje || !aux){
		return 0;
	}
	if(!sala_es_interaccion_valida(sala, verbo, objeto1, objeto2)){
		return 0;
	}
	int ejecutadas = 0;
	for(int i = 0; i < sala->interacciones->cantidad; i++){

		struct interaccion *interaccion = (struct interaccion *)lista_elemento_en_posicion(sala->interacciones, (size_t)i);

		if(strcmp(interaccion->verbo, verbo) == 0 &&
		strcmp(interaccion->objeto, objeto1) == 0 &&
		strcmp(interaccion->objeto_parametro, objeto2) == 0){
			ejecutadas = ejecutar_interacciones(sala, interaccion, objeto1, objeto2, mostrar_mensaje, aux);
		}			
	}
	return ejecutadas;
}

bool sala_escape_exitoso(sala_t *sala)
{
	if(!sala){
		return false;
	}
	return sala->escape_exitoso;
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