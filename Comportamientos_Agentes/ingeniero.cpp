#include "ingeniero.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>
#include <climits>

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoIngeniero::think(Sensores sensores)
{
  Action accion = IDLE;

  // Decisión del agente según el nivel
  switch (sensores.nivel)
  {
  case 0:
    accion = ComportamientoIngenieroNivel_0(sensores);
    break;
  case 1:
    accion = ComportamientoIngenieroNivel_1(sensores);
    break;
  case 2:
    accion = ComportamientoIngenieroNivel_2(sensores);
    break;
  case 3:
    accion = ComportamientoIngenieroNivel_3(sensores);
    break;
  case 4:
    accion = ComportamientoIngenieroNivel_4(sensores);
    break;
  case 5:
    accion = ComportamientoIngenieroNivel_5(sensores);
    break;
  case 6:
    accion = ComportamientoIngenieroNivel_6(sensores);
    break;
  }

  return accion;
}


// Niveles iniciales (Comportamientos reactivos simples)
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_0(Sensores sensores)
{
  // Inicializamos la matriz de mapas visitados
  if (mapaVisitados.empty() && mapaResultado.size() > 0) {
    mapaVisitados.assign(mapaResultado.size(), std::vector<int>(mapaResultado[0].size(), 0));
  }

  // actualizamos el mapa y la matriz de casillas visitadas
  if (sensores.posF != -1) {
    ActualizarMapa(sensores);
    mapaVisitados[sensores.posF][sensores.posC]++;
  }

  // Actualizamos aburrimiento
  if(mapaVisitados[sensores.posF][sensores.posC]>0)
    turnos_aburrido++;
  else
    turnos_aburrido = 0;

  // Obtenemos los datos de los sensores para observar si podemos avanzar
  ubicacion actual = {sensores.posF, sensores.posC, sensores.rumbo};
  ubicacion delante = Delante(actual);

  // Actualizamos variable tengo_zapatillas
  if (sensores.superficie[0] == 'D') {
    tengo_zapatillas = true;
  }

  // Si encontramos la casilla T. Residuos, entonces terminamos la búsqueda y paramos al player
  if (sensores.superficie[0] == 'U') {
    return IDLE; 
  }

  if (giros_pendientes > 0) {
    giros_pendientes--;     // Descontamos un giro
    giros_consecutivos = 0; // Reseteamos 
    return TURN_SR;         // Giramos para dar la vuelta
  }
  
  // BUSQUEDA DE CASILLAS OBJETIVO ADYACENTES AL AGENTE:
  // Buscamos si son viables las casillas a nuestra izquierda, centro y derecha
  char i = ViablePorAltura(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tengo_zapatillas);
  char c = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tengo_zapatillas);
  char d = ViablePorAltura(sensores.superficie[3], sensores.cota[3] - sensores.cota[0], tengo_zapatillas);

  // Comprobamos ademas que el tecnico no esté en ninguna de las casillas
  if (sensores.agentes[1] == 't') i = 'P'; 
  // si está a la izquierda o la derecha, la marcamos como precipicio para no pasar
  if (sensores.agentes[3] == 't') d = 'P';
  // if (sensores.agentes[2] == 't') c = 'P'; // si es

  // Si nos encontramos con el tecnico en frente, entonces giramos para esquivarlo
  if (sensores.agentes[2] == 't'){
    last_action = TURN_SR;
    return TURN_SR;
  } // Sin sumar giros consecutivos pues es para esquivar el tecnico

  // Evaluamos cual de las casillas es mas conveniente, 0 si ninguna 
  int pos = VeoCasillaInteresante(i, c, d, tengo_zapatillas, actual);

  if (pos == 2){
    giros_consecutivos = 0;
    return WALK;
  }else if (pos == 1){
    giros_consecutivos = 0; // ++?
    return TURN_SL;
  }else if (pos == 3){
    giros_consecutivos = 0;
    return TURN_SR;
  }

  // Llegados a este punto, pos==0 luego ninguna casilla adyacente interesante.

  // Analizamos TODA LA VISIÓN del agente por si 'U' estuviera en alguna casilla a la vista
  // diferente a las posiciones adyacentes 1, 2 y 3 que ya han sido estudiadas
  
  // Añadir que solo si NO estamos en mitad de un giro, entonces vamos a dnd nos diga la vision completa?

  // Casillas del frente (si lo vemos en frente y la casilla de delante es camino)
  if (( sensores.superficie[2] == 'U' || sensores.superficie[6] == 'U' || sensores.superficie[12] == 'U') && es_camino(c))
    return WALK;

  // Casillas de la izquierda
  // Si lo vemos a la izquierda y la casilla 1 es camino
  if ((sensores.superficie[4] == 'U' || sensores.superficie[5] == 'U' || sensores.superficie[9] == 'U' || sensores.superficie[10] == 'U' || sensores.superficie[11] == 'U') && es_camino(i))
    return TURN_SL;

  // Casillas de la derecha
  // Si lo vemos a la derecha y la casilla 3 es camino (a la que nos dirigimos)
  if ((sensores.superficie[7] == 'U' || sensores.superficie[8] == 'U' || sensores.superficie[13] == 'U' || sensores.superficie[14] == 'U' || sensores.superficie[15] == 'U') && es_camino(d))
    return TURN_SR;
  

  // Si llegamos a este punto, pos == 0 y ninguna posición de toda la visión es de T de Residuos, 
  // luego no hay ningun objetivo próximo. Pasamos a explorar:

  // Inicializamos la accion a IDLE 
  Action accion = IDLE;

  bool puedo_avanzar = (EsCasillaTransitableLevel0(delante.f, delante.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, tengo_zapatillas) && !sensores.choque);

  // Si podemos avanzar pero en frente tenemos al tecnico, giraremos. Double check de tecnico . Necesario??
  if (puedo_avanzar && (sensores.agentes[2] == 't')) {
    puedo_avanzar = false; 
  }


  // Si llevamos demasiados pasos pisando casillas ya pisadas, estamos atrapados en un bucle, provocamos giro 180º
  if (turnos_aburrido > 30) {
    turnos_aburrido = 0;
    giros_pendientes=3; // Para dar la vuelta de 180 (4 giros de 45)
    return TURN_SR;
  }

  if (puedo_avanzar){
    accion = WALK;
    giros_consecutivos = 0;
  }
  else{

    if (giros_consecutivos%2 != 0){ // si no es el primer giro de 45 grados
      accion = last_action; 
      giros_consecutivos++;
      // entonces realizamos el mismo giro que hicimos y completamos el giro de 90º
    }
    else{ // Vemos que casilla ha sido menos visitada si la izq o la derecha

      // observamos a la derecha y a la izquierda (90 grados)
      int visitas_i = INT_MAX;
      int visitas_d = INT_MAX;

      // obtenemos ubicacion de casilla derecha e izquierda (90º)
      ubicacion izq = actual;
      izq.brujula = (Orientacion) (((int) actual.brujula + 6) % 8);
      ubicacion casilla_i = Delante(izq);

      ubicacion der = actual;
      der.brujula = (Orientacion) (((int) actual.brujula + 2) % 8);
      ubicacion casilla_d = Delante(der);


      if (EsCasillaTransitableLevel0(casilla_i.f, casilla_i.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_i, tengo_zapatillas)){
        visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];

      }

      if (EsCasillaTransitableLevel0(casilla_d.f, casilla_d.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_d, tengo_zapatillas)){
        visitas_d = mapaVisitados[casilla_d.f][casilla_d.c];

      }

      if (visitas_d <= visitas_i)
        accion = TURN_SR;
      else
        accion = TURN_SL;
        
      giros_consecutivos++;
    }
  }

  // si llevamos 8 giros consecutivos (vuelta completa) entonces IDLE, porque estamos encerrados
  if (giros_consecutivos >= 8){
    accion =  IDLE; 
    giros_consecutivos=0;
  }

  // guardamos la accion en ultima_accion
  last_action = accion;
  
  return accion; // WALK, TURN_SL, TURN_SR, IDLE
}

/**
 * @brief Comprueba si una celda es de tipo camino transitable.
 * @param c Carácter que representa el tipo de superficie.
 * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
 */
bool ComportamientoIngeniero::es_camino(unsigned char c) const
{
  return (c == 'C' || c == 'D' || c == 'U');
}


bool ComportamientoIngeniero::es_caminoNivel_1(unsigned char c) const
{
  return (c == 'C' || c == 'D' || c == 'S');
  // Quitamos U (t residuos) y añadimos Sendero
}

/**
 * @brief Comportamiento reactivo del ingeniero para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 * objetivo: descubrir la mayor cantidad
posible de casillas de tipo camino ‘C’ y sendero ‘S’
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_1(Sensores sensores)
{
  // Usamos la misma lógica que en el 0 pero sin condición de parada cuando
  // pasamos por T. Residuos.

  // Inicializamos la matriz de mapas visitados
  if (mapaVisitados.empty() && mapaResultado.size() > 0) {
    mapaVisitados.assign(mapaResultado.size(), std::vector<int>(mapaResultado[0].size(), 0));
  }

  // actualizamos el mapa y la matriz de casillas visitadas
  if (sensores.posF != -1) {
    ActualizarMapa(sensores);
    mapaVisitados[sensores.posF][sensores.posC]++;
  }

  // Actualizamos el mapa y el estado del juego
  // ActualizarMapa(sensores);

  // Obtenemos los datos de los sensores para observar si podemos avanzar
  ubicacion actual = {sensores.posF, sensores.posC, sensores.rumbo};
  ubicacion delante = Delante(actual);

  // Actualizamos variable tengo_zapatillas
  if (sensores.superficie[0] == 'D') {
    tengo_zapatillas = true;
  }

  // Condicion de parada en T. Residuos eliminada

  
  // BUSQUEDA DE CASILLAS OBJETIVO:
  // Buscamos si son viables las casillas a nuestra izquierda, centro y derecha
  char i = ViablePorAltura(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tengo_zapatillas);
  char c = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tengo_zapatillas);
  char d = ViablePorAltura(sensores.superficie[3], sensores.cota[3] - sensores.cota[0], tengo_zapatillas);

  // Comprobamos ademas que el tecnico no esté en ninguna de las casillas
  if (sensores.agentes[1] == 'a') i = 'P'; 
  // si está, la 'marcamos' como precipicio para no pasar
  if (sensores.agentes[2] == 'a') c = 'P';
  if (sensores.agentes[3] == 'a') d = 'P';

  // Evaluamos cual de las casillas es mas conveniente, 0 si ninguna 
  int pos = VeoCasillaInteresanteNivel1(i, c, d, tengo_zapatillas, actual);

  if (pos == 2){
    giros_consecutivos = 0;
    return WALK;
  }else if (pos == 1){
    giros_consecutivos = 0;
    return TURN_SL;
  }else if (pos == 3){
    giros_consecutivos = 0;
    return TURN_SR;
  }
  
  // Si llegamos a este punto, pos == 0, luego no hay ningun objetivo delante
  // Pasamos a explorar:

  // Inicializamos la accion a IDLE 
  Action accion = IDLE;

  bool puedo_avanzar = (EsCasillaTransitableLevel1(delante.f, delante.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, tengo_zapatillas) && !sensores.choque);

  // Si podemos avanzar pero en frente tenemos al tecnico, giraremos
  if (puedo_avanzar && (sensores.agentes[2] == 'a')) {
    puedo_avanzar = false; 
  }

if (puedo_avanzar){
    accion = WALK;
    giros_consecutivos = 0;
  }
  else{

    // Vemos que casilla ha sido menos visitada si la izq o la derecha
    if (giros_consecutivos%2 != 0){ // si no es el primer giro de 45 grados
      accion = last_action; 
      // entonces realizamos el mismo giro que hicimos para completar el giro de 90º
      giros_consecutivos++;
    }
    else{ 
      giros_consecutivos++;
      // observamos a la derecha y a la izquierda (90 grados)
      int visitas_i = INT_MAX;
      int visitas_d = INT_MAX;

      // obtenemos ubicacion de casilla derecha e izquierda (90º)
      ubicacion izq = actual;
      izq.brujula = (Orientacion) (((int) actual.brujula + 6) % 8);
      ubicacion casilla_i = Delante(izq);

      ubicacion der = actual;
      der.brujula = (Orientacion) (((int) actual.brujula + 2) % 8);
      ubicacion casilla_d = Delante(der);


      if (EsCasillaTransitableLevel1(casilla_i.f, casilla_i.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_i, tengo_zapatillas)){
        visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];
      }

      if (EsCasillaTransitableLevel1(casilla_d.f, casilla_d.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_d, tengo_zapatillas)){
        visitas_d = mapaVisitados[casilla_d.f][casilla_d.c];
      }

      if (visitas_d <= visitas_i)
        accion = TURN_SR;
      else
        accion = TURN_SL;
        
      last_action = accion;
    }
  }


  // si llevamos 8 giros consecutivos (vuelta completa) entonces IDLE, porque estamos encerrados
  if (giros_consecutivos >= 8)
    accion =  IDLE; 

  return accion; // WALK, TURN_SL, TURN_SR, IDLE
}

// Niveles avanzados (Uso de búsqueda)
/**
 * @brief Comportamiento del ingeniero para el Nivel 2 (búsqueda).
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_2(Sensores sensores)
{
  // TODO: Implementar búsqueda para el Nivel 2.
  return IDLE;
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_3(Sensores sensores)
{
  return IDLE;
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_4(Sensores sensores)
{
  return IDLE;
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores)
{
  return IDLE;
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_6(Sensores sensores)
{
  return IDLE;
}


/**
 * @brief Determina si casilla viable por altura
 * @param casilla    tipo de terreno
 * @param dif  diferencia de altura entre casillas
 * @param zap indica si tenemos o no las zapatillas
 * @return 'P' si no es accesible por altura y casilla en otro caso
 */
char ComportamientoIngeniero::ViablePorAltura (char casilla, int dif, bool zap){
  // Si el desnivel es <= 1, o si tengo zapatillas y es <= 2, puedo pasar
  if (abs(dif) <= 1 || (zap && abs(dif) <= 2)) {
    return casilla;
  } else {
    return 'P'; // Si no puedo pasar, simulamos que es un Precipicio
  }
}

/** @brief Determina la mejor opcion entre las 3 casillas que tiene delante
 * @param i    terreno que hay en la pos 1 (45izq)
 * @param c  terreno que hay en la pos 2 (delante)
 * @param d terreno que hay en la pos 3 (45derecha)
 * @param zap indica si tenemos o no las zapatillas
 * @return 2 si es mejor WALK, 1 TURN_SL, 3 TURN_SR. 0 si nada interesante
 */

int ComportamientoIngeniero::VeoCasillaInteresante(char i, char c, char d, bool zap, ubicacion actual){
  // Buscamos si la meta se encuentra alrededor nuestra
  if (c == 'U') return 2; // en frente
  else if (i == 'U') return 1; // izquierda
  else if (d == 'U') return 3; // derecha

  // Buscamos ahora las zapatillas, solo en caso de NO tenerlas ya
  if (!zap) {
    if (c == 'D') return 2;
    else if (i == 'D') return 1;
    else if (d == 'D') return 3;
  }

  // Buscamos casillas de tipo camino (en este nivel no tenemos en cuenta energía)
  // Pero además iremos a la casilla que haya sido visitada menos veces

  ubicacion izq = actual;
  izq.brujula = (Orientacion) (((int) actual.brujula + 7) % 8);
  ubicacion casilla_i = Delante(izq);

  ubicacion casilla_c = Delante(actual);

  ubicacion der = actual;
  der.brujula = (Orientacion) (((int) actual.brujula + 1) % 8);
  ubicacion casilla_d = Delante(der);

  // Procedemos a buscar el minimo de visitas
  // Inicializamos al maximo
  int visitas_i = INT_MAX, visitas_c = INT_MAX, visitas_d = INT_MAX;

  // Si estan en los rangos adecuados entonces le asignamos su valor correspondiente
  if (i != 'P' && casilla_i.f >= 0 && casilla_i.f < mapaVisitados.size() && casilla_i.c >= 0 && casilla_i.c < mapaVisitados[0].size())
      visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];

  if (c != 'P' && casilla_c.f >= 0 && casilla_c.f < mapaVisitados.size() && casilla_c.c >= 0 && casilla_c.c < mapaVisitados[0].size())
      visitas_c = mapaVisitados[casilla_c.f][casilla_c.c];

  if (d != 'P' && casilla_d.f >= 0 && casilla_d.f < mapaVisitados.size() && casilla_d.c >= 0 && casilla_d.c < mapaVisitados[0].size())
      visitas_d = mapaVisitados[casilla_d.f][casilla_d.c];
    
    
  // elegimos la casilla menos visitada siempre que sea casilla transitable:
  int mejor_opcion = 0;  // en caso de que no encuentre nada, decide el agente
  int menor = INT_MAX;

  if (c != 'P' && EsCasillaTransitableLevel0(casilla_c.f, casilla_c.c, zap) && visitas_c < menor) {    
    menor = visitas_c;
    mejor_opcion = 2; // WALK
  }
  if (i != 'P' && EsCasillaTransitableLevel0(casilla_i.f, casilla_i.c, zap) && visitas_i < menor) {
    menor = visitas_i;
    mejor_opcion = 1; // Giramos a izquierda
  }
  if (d != 'P' && EsCasillaTransitableLevel0(casilla_d.f, casilla_d.c, zap) && visitas_d < menor) {
    menor = visitas_d;
    mejor_opcion = 3; // Giramos a derecha
  }

  return mejor_opcion;
}

/** @brief Determina la mejor opcion entre las 3 casillas que tiene delante
 * @param i    terreno que hay en la pos 1 (45izq)
 * @param c  terreno que hay en la pos 2 (delante)
 * @param d terreno que hay en la pos 3 (45derecha)
 * @param zap indica si tenemos o no las zapatillas
 * @return 2 si es mejor WALK, 1 TURN_SL, 3 TURN_SR. 0 si nada interesante
 */

int ComportamientoIngeniero::VeoCasillaInteresanteNivel1(char i, char c, char d, bool zap, ubicacion actual){

  // Buscamos las zapatillas, solo en caso de NO tenerlas ya
  if (!zap) {
    if (c == 'D') return 2;
    else if (i == 'D') return 1;
    else if (d == 'D') return 3;
  }

  // Buscamos casillas de tipo camino o sendero (en este nivel no tenemos en cuenta energía)
  // Pero además iremos a la casilla que haya sido visitada menos veces

  ubicacion izq = actual;
  izq.brujula = (Orientacion) (((int) actual.brujula + 7) % 8);
  ubicacion casilla_i = Delante(izq);

  ubicacion casilla_c = Delante(actual);

  ubicacion der = actual;
  der.brujula = (Orientacion) (((int) actual.brujula + 1) % 8);
  ubicacion casilla_d = Delante(der);

  // Procedemos a buscar el minimo de visitas
  // Inicializamos al maximo
  int visitas_i = INT_MAX, visitas_c = INT_MAX, visitas_d = INT_MAX;

  // Si estan en los rangos adecuados entonces le asignamos su valor correspondiente
  if (casilla_i.f >= 0 && casilla_i.f < mapaVisitados.size() && casilla_i.c >= 0 && casilla_i.c < mapaVisitados[0].size())
      visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];

  if (casilla_c.f >= 0 && casilla_c.f < mapaVisitados.size() && casilla_c.c >= 0 && casilla_c.c < mapaVisitados[0].size())
      visitas_c = mapaVisitados[casilla_c.f][casilla_c.c];

  if (casilla_d.f >= 0 && casilla_d.f < mapaVisitados.size() && casilla_d.c >= 0 && casilla_d.c < mapaVisitados[0].size())
      visitas_d = mapaVisitados[casilla_d.f][casilla_d.c];
    
  // elegimos la casilla menos visitada siempre que sea camino y sendero

  int mejor_opcion = 0;  // en caso de que no encuentre nada, decide el agente
  int menor = INT_MAX;

  if (c != 'P' && EsCasillaTransitableLevel1(casilla_c.f, casilla_c.c, zap) && visitas_c < menor) {
    menor = visitas_c;
    mejor_opcion = 2; // WALK
  }
  if (i != 'P' && EsCasillaTransitableLevel1(casilla_i.f, casilla_i.c, zap) && visitas_i < menor) {
    menor = visitas_i;
    mejor_opcion = 1; // TURN_SL
  }
  if (d != 'P' && EsCasillaTransitableLevel1(casilla_d.f, casilla_d.c, zap) && visitas_d < menor) {
    menor = visitas_d;
    mejor_opcion = 3; // TURN_SL
  }

  return mejor_opcion; 
}

// =========================================================================
// FUNCIONES PROPORCIONADAS
// =========================================================================

/**
 * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
 * @param sensores Datos actuales de los sensores.
 */
void ComportamientoIngeniero::ActualizarMapa(Sensores sensores)
{
  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];

  int pos = 1;
  switch (sensores.rumbo)
  {
  case norte:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF - j][sensores.posC + i] = sensores.superficie[pos];
        mapaCotas[sensores.posF - j][sensores.posC + i] = sensores.cota[pos++];
      }
    break;
  case noreste:
    mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[1];
    mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[1];
    mapaResultado[sensores.posF - 1][sensores.posC + 1] = sensores.superficie[2];
    mapaCotas[sensores.posF - 1][sensores.posC + 1] = sensores.cota[2];
    mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[3];
    mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[3];
    mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[4];
    mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[4];
    mapaResultado[sensores.posF - 2][sensores.posC + 1] = sensores.superficie[5];
    mapaCotas[sensores.posF - 2][sensores.posC + 1] = sensores.cota[5];
    mapaResultado[sensores.posF - 2][sensores.posC + 2] = sensores.superficie[6];
    mapaCotas[sensores.posF - 2][sensores.posC + 2] = sensores.cota[6];
    mapaResultado[sensores.posF - 1][sensores.posC + 2] = sensores.superficie[7];
    mapaCotas[sensores.posF - 1][sensores.posC + 2] = sensores.cota[7];
    mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[8];
    mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[8];
    mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[9];
    mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[9];
    mapaResultado[sensores.posF - 3][sensores.posC + 1] = sensores.superficie[10];
    mapaCotas[sensores.posF - 3][sensores.posC + 1] = sensores.cota[10];
    mapaResultado[sensores.posF - 3][sensores.posC + 2] = sensores.superficie[11];
    mapaCotas[sensores.posF - 3][sensores.posC + 2] = sensores.cota[11];
    mapaResultado[sensores.posF - 3][sensores.posC + 3] = sensores.superficie[12];
    mapaCotas[sensores.posF - 3][sensores.posC + 3] = sensores.cota[12];
    mapaResultado[sensores.posF - 2][sensores.posC + 3] = sensores.superficie[13];
    mapaCotas[sensores.posF - 2][sensores.posC + 3] = sensores.cota[13];
    mapaResultado[sensores.posF - 1][sensores.posC + 3] = sensores.superficie[14];
    mapaCotas[sensores.posF - 1][sensores.posC + 3] = sensores.cota[14];
    mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[15];
    mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[15];
    break;
  case este:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF + i][sensores.posC + j] = sensores.superficie[pos];
        mapaCotas[sensores.posF + i][sensores.posC + j] = sensores.cota[pos++];
      }
    break;
  case sureste:
    mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[1];
    mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[1];
    mapaResultado[sensores.posF + 1][sensores.posC + 1] = sensores.superficie[2];
    mapaCotas[sensores.posF + 1][sensores.posC + 1] = sensores.cota[2];
    mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[3];
    mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[3];
    mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[4];
    mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[4];
    mapaResultado[sensores.posF + 1][sensores.posC + 2] = sensores.superficie[5];
    mapaCotas[sensores.posF + 1][sensores.posC + 2] = sensores.cota[5];
    mapaResultado[sensores.posF + 2][sensores.posC + 2] = sensores.superficie[6];
    mapaCotas[sensores.posF + 2][sensores.posC + 2] = sensores.cota[6];
    mapaResultado[sensores.posF + 2][sensores.posC + 1] = sensores.superficie[7];
    mapaCotas[sensores.posF + 2][sensores.posC + 1] = sensores.cota[7];
    mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[8];
    mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[8];
    mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[9];
    mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[9];
    mapaResultado[sensores.posF + 1][sensores.posC + 3] = sensores.superficie[10];
    mapaCotas[sensores.posF + 1][sensores.posC + 3] = sensores.cota[10];
    mapaResultado[sensores.posF + 2][sensores.posC + 3] = sensores.superficie[11];
    mapaCotas[sensores.posF + 2][sensores.posC + 3] = sensores.cota[11];
    mapaResultado[sensores.posF + 3][sensores.posC + 3] = sensores.superficie[12];
    mapaCotas[sensores.posF + 3][sensores.posC + 3] = sensores.cota[12];
    mapaResultado[sensores.posF + 3][sensores.posC + 2] = sensores.superficie[13];
    mapaCotas[sensores.posF + 3][sensores.posC + 2] = sensores.cota[13];
    mapaResultado[sensores.posF + 3][sensores.posC + 1] = sensores.superficie[14];
    mapaCotas[sensores.posF + 3][sensores.posC + 1] = sensores.cota[14];
    mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[15];
    mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[15];
    break;
  case sur:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF + j][sensores.posC - i] = sensores.superficie[pos];
        mapaCotas[sensores.posF + j][sensores.posC - i] = sensores.cota[pos++];
      }
    break;
  case suroeste:
    mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[1];
    mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[1];
    mapaResultado[sensores.posF + 1][sensores.posC - 1] = sensores.superficie[2];
    mapaCotas[sensores.posF + 1][sensores.posC - 1] = sensores.cota[2];
    mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[3];
    mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[3];
    mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[4];
    mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[4];
    mapaResultado[sensores.posF + 2][sensores.posC - 1] = sensores.superficie[5];
    mapaCotas[sensores.posF + 2][sensores.posC - 1] = sensores.cota[5];
    mapaResultado[sensores.posF + 2][sensores.posC - 2] = sensores.superficie[6];
    mapaCotas[sensores.posF + 2][sensores.posC - 2] = sensores.cota[6];
    mapaResultado[sensores.posF + 1][sensores.posC - 2] = sensores.superficie[7];
    mapaCotas[sensores.posF + 1][sensores.posC - 2] = sensores.cota[7];
    mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[8];
    mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[8];
    mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[9];
    mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[9];
    mapaResultado[sensores.posF + 3][sensores.posC - 1] = sensores.superficie[10];
    mapaCotas[sensores.posF + 3][sensores.posC - 1] = sensores.cota[10];
    mapaResultado[sensores.posF + 3][sensores.posC - 2] = sensores.superficie[11];
    mapaCotas[sensores.posF + 3][sensores.posC - 2] = sensores.cota[11];
    mapaResultado[sensores.posF + 3][sensores.posC - 3] = sensores.superficie[12];
    mapaCotas[sensores.posF + 3][sensores.posC - 3] = sensores.cota[12];
    mapaResultado[sensores.posF + 2][sensores.posC - 3] = sensores.superficie[13];
    mapaCotas[sensores.posF + 2][sensores.posC - 3] = sensores.cota[13];
    mapaResultado[sensores.posF + 1][sensores.posC - 3] = sensores.superficie[14];
    mapaCotas[sensores.posF + 1][sensores.posC - 3] = sensores.cota[14];
    mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[15];
    mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[15];
    break;
  case oeste:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
        mapaResultado[sensores.posF - i][sensores.posC - j] = sensores.superficie[pos];
        mapaCotas[sensores.posF - i][sensores.posC - j] = sensores.cota[pos++];
      }
    break;
  case noroeste:
    mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[1];
    mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[1];
    mapaResultado[sensores.posF - 1][sensores.posC - 1] = sensores.superficie[2];
    mapaCotas[sensores.posF - 1][sensores.posC - 1] = sensores.cota[2];
    mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[3];
    mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[3];
    mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[4];
    mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[4];
    mapaResultado[sensores.posF - 1][sensores.posC - 2] = sensores.superficie[5];
    mapaCotas[sensores.posF - 1][sensores.posC - 2] = sensores.cota[5];
    mapaResultado[sensores.posF - 2][sensores.posC - 2] = sensores.superficie[6];
    mapaCotas[sensores.posF - 2][sensores.posC - 2] = sensores.cota[6];
    mapaResultado[sensores.posF - 2][sensores.posC - 1] = sensores.superficie[7];
    mapaCotas[sensores.posF - 2][sensores.posC - 1] = sensores.cota[7];
    mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[8];
    mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[8];
    mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[9];
    mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[9];
    mapaResultado[sensores.posF - 1][sensores.posC - 3] = sensores.superficie[10];
    mapaCotas[sensores.posF - 1][sensores.posC - 3] = sensores.cota[10];
    mapaResultado[sensores.posF - 2][sensores.posC - 3] = sensores.superficie[11];
    mapaCotas[sensores.posF - 2][sensores.posC - 3] = sensores.cota[11];
    mapaResultado[sensores.posF - 3][sensores.posC - 3] = sensores.superficie[12];
    mapaCotas[sensores.posF - 3][sensores.posC - 3] = sensores.cota[12];
    mapaResultado[sensores.posF - 3][sensores.posC - 2] = sensores.superficie[13];
    mapaCotas[sensores.posF - 3][sensores.posC - 2] = sensores.cota[13];
    mapaResultado[sensores.posF - 3][sensores.posC - 1] = sensores.superficie[14];
    mapaCotas[sensores.posF - 3][sensores.posC - 1] = sensores.cota[14];
    mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[15];
    mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[15];
    break;
  }
}

/**
 * @brief Determina si una casilla es transitable para el ingeniero.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable (no es muro ni precipicio).
 */
bool ComportamientoIngeniero::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas)
{
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size())
    return false;
  return es_camino(mapaResultado[f][c]); // Solo 'C', 'D', 'U' son transitables en Nivel 0
}

bool ComportamientoIngeniero::EsCasillaTransitableLevel1(int f, int c, bool tieneZapatillas)
{
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size())
    return false;
  return es_caminoNivel_1(mapaResultado[f][c]); // Solo 'C', 'D', 'S' son transitables en Nivel 1
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el ingeniero: desnivel máximo 1 sin zapatillas, 2 con zapatillas.
 * @param actual Estado actual del agente (fila, columna, orientacion, zap).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoIngeniero::EsAccesiblePorAltura(const ubicacion &actual, bool zap)
{
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size())
    return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (zap && desnivel > 2)
    return false;
  if (!zap && desnivel > 1)
    return false;
  return true;
}

bool ComportamientoIngeniero::EsAccesiblePorAltura(const ubicacion &origen, const ubicacion &destino, bool zap) {

  if (destino.f < 0 || destino.f >= mapaCotas.size() || destino.c < 0 || destino.c >= mapaCotas[0].size()) {
    return false;
  }

  int desnivel = abs(mapaCotas[destino.f][destino.c] - mapaCotas[origen.f][origen.c]);

  if (zap && desnivel > 2) {
    return false; // Con zapatillas tolera hasta un desnivel de +-2
  }
  
  if (!zap && desnivel > 1) {
    return false; // Sin zapatillas tolera hasta un desnivel de +-1
  }

  return true;
}


/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoIngeniero::Delante(const ubicacion &actual) const
{
  ubicacion delante = actual;
  switch (actual.brujula)
  {
  case 0:
    delante.f--;
    break; // norte
  case 1:
    delante.f--;
    delante.c++;
    break; // noreste
  case 2:
    delante.c++;
    break; // este
  case 3:
    delante.f++;
    delante.c++;
    break; // sureste
  case 4:
    delante.f++;
    break; // sur
  case 5:
    delante.f++;
    delante.c--;
    break; // suroeste
  case 6:
    delante.c--;
    break; // oeste
  case 7:
    delante.f--;
    delante.c--;
    break; // noroeste
  }
  return delante;
}

/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoIngeniero::PintaPlan(const list<Action> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    if (*it == WALK)
    {
      cout << "W ";
    }
    else if (*it == JUMP)
    {
      cout << "J ";
    }
    else if (*it == TURN_SR)
    {
      cout << "r ";
    }
    else if (*it == TURN_SL)
    {
      cout << "l ";
    }
    else if (*it == COME)
    {
      cout << "C ";
    }
    else if (*it == IDLE)
    {
      cout << "I ";
    }
    else
    {
      cout << "-_ ";
    }
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}

/**
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 *
 * @param plan  Lista de pasos (fila, columna, operación),
 *              donde operacion = -1 (DIG), operación = 1 (RAISE).
 */
void ComportamientoIngeniero::PintaPlan(const list<Paso> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    cout << it->fil << ", " << it->col << " (" << it->op << ")\n";
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}

/**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa 2D.
 *
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoIngeniero::VisualizaPlan(const ubicacion &st,
                                            const list<Action> &plan)
{
  listaPlanCasillas.clear();
  ubicacion cst = st;

  listaPlanCasillas.push_back({cst.f, cst.c, WALK});
  auto it = plan.begin();
  while (it != plan.end())
  {

    switch (*it)
    {
    case JUMP:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, JUMP});
    case WALK:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, WALK});
      break;
    case TURN_SR:
      cst.brujula = (Orientacion) (( (int) cst.brujula + 1) % 8);
      break;
    case TURN_SL:
      cst.brujula = (Orientacion) (( (int) cst.brujula + 7) % 8);
      break;
    }
    it++;
  }
}

/**
 * @brief Convierte un plan de tubería en la lista de casillas usada
 *        por el sistema de visualización.
 *
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tubería.
 */
void ComportamientoIngeniero::VisualizaRedTuberias(const list<Paso> &plan)
{
  listaCanalizacionTuberias.clear();
  auto it = plan.begin();
  while (it != plan.end())
  {
    listaCanalizacionTuberias.push_back({it->fil, it->col, it->op});
    it++;
  }
}