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

  // Calculamos umbral de aburrimiento en funcion de como de grande sea el mapa
  int umbral_aburrimiento = mapaResultado.size(); 
  if (umbral_aburrimiento<30) umbral_aburrimiento = 30;  //  Para mapas pequeños

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

  // Si encontramos la casilla T. Residuos, entonces terminamos la búsqueda y paramos al agente
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

  // Si nos encontramos con el tecnico en frente, entonces giramos para esquivarlo
  if (sensores.agentes[2] == 't'){
    last_action = TURN_SR;
    return TURN_SR;
  } // Sin sumar giros consecutivos pues es para esquivar el tecnico

  // Evaluamos cual de las casillas es mas conveniente, 0 si ninguna 
  // Dentro del metodo usamos memoria de casillas visitadas para explorar nuevas casillas
  int pos = VeoCasillaInteresante(i, c, d, tengo_zapatillas, actual);

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

  // Llegados a este punto, pos==0 luego ninguna casilla adyacente interesante.

  // Analizamos TODA LA VISIÓN del agente por si 'U' estuviera en alguna casilla a la vista
  // diferente a las posiciones adyacentes 1, 2 y 3 que ya han sido estudiadas
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

  // Si podemos avanzar pero en frente tenemos al tecnico, giraremos.
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
  else{  // Vemos que casilla ha sido menos visitada si la izq o la derecha

    if (giros_consecutivos%2 != 0){ // si no es el primer giro de 45 grados
      accion = last_action;
      giros_consecutivos++;
      // entonces realizamos el mismo giro que hicimos y completamos el giro de 90º
    }
    else{

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

  // guardamos la accion en last_action
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
  if (sensores.agentes[1] == 't') i = 'P'; 
  // si está, la 'marcamos' como precipicio para no pasar
  if (sensores.agentes[2] == 't'){
    last_action = TURN_SR;
    return TURN_SR;
  }
  if (sensores.agentes[3] == 't') d = 'P';

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
  if (puedo_avanzar && (sensores.agentes[2] == 't')) {
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
  Action accion = IDLE;

  if (!hayPlan){
    // Invocar al metodo de busqueda
    EstadoI inicio, fin;
    inicio.site.f = sensores.posF;
    inicio.site.c = sensores.posC;
    inicio.site.brujula = sensores.rumbo;
    inicio.zapatillas = tengo_zapatillas;

    fin.site.f = sensores.BelPosF;
    fin.site.c = sensores.BelPosC;
    
    plan = B_Anchura_V2(inicio, fin, mapaResultado, mapaCotas);
    VisualizaPlan(inicio.site,plan);
    hayPlan = plan.size() != 0;
  }
  if (hayPlan && plan.size()>0){
    accion = plan.front();
    plan.pop_front();
  }

  if (plan.size()==0){
    hayPlan = false;
  }

  return accion;
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
  if (!hayPlan){
    // Invocar al metodo de busqueda
    EstadoTub inicio;
    inicio.f = sensores.BelPosF;
    inicio.c = sensores.BelPosC;
    inicio.altura = (int)mapaCotas[inicio.f][inicio.c];

    // Buscamos las casillas de Tratamiento de Residuos
    vector<pair<int, int>> plantas;
    for (int f = 0; f < mapaResultado.size(); f++) {
      for (int c = 0; c < mapaResultado[f].size(); c++) {
        if (mapaResultado[f][c] == 'U')
          plantas.push_back({f, c});
      }
    }
    
    list<Paso> pasos = AlgoritmoAEstrellaTub(inicio, plantas, mapaResultado, mapaCotas, sensores);
    
    if (pasos.size() > 0){
      VisualizaRedTuberias(pasos); // El simulador valida y termina el nivel
      hayPlan = true;
    } else {
      hayPlan = false;
    }
  }
    

  return IDLE; // el agente no hace nada tras calcular y mostrar el plan de pasos
}


/**
 * @brief Comportamiento del ingeniero para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */


 Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores) {
  
  if (plan_tuberia.empty()) {
    EstadoTub inicio = {(int)sensores.BelPosF, (int)sensores.BelPosC, (int)mapaCotas[sensores.BelPosF][sensores.BelPosC]};
    vector<pair<int, int>> plantas;

    for (int f = 0; f < mapaResultado.size(); f++) 
      for (int c = 0; c < mapaResultado[f].size(); c++) 
        if (mapaResultado[f][c] == 'U') plantas.push_back({f, c});

    list<Paso> lista_plan = AlgoritmoAEstrellaTub(inicio, plantas, mapaResultado, mapaCotas, sensores);
    plan_tuberia.assign(lista_plan.begin(), lista_plan.end());
    tramo_actual = 0;
    esperando_tecnico = false; 
    esperando_install = false; 
    llamado_en_ini = false; 
    hayPlan = false;
    if (!plan_tuberia.empty()) VisualizaRedTuberias(lista_plan);
  }

  if (tramo_actual >= plan_tuberia.size() - 1) return IDLE;

  int f_ini = plan_tuberia[tramo_actual].fil;
  int c_ini = plan_tuberia[tramo_actual].col;
  int f_fin = plan_tuberia[tramo_actual + 1].fil;
  int c_fin = plan_tuberia[tramo_actual + 1].col;

  // Condicion 1: ¿Estamos en la casilla FIN y ya hemos llamado al tecnico?
  if (sensores.posF == f_fin && sensores.posC == c_fin && esperando_tecnico) {
      
    if (plan_tuberia[tramo_actual + 1].op != 0 && !terreno_preparado) {
      terreno_preparado = true;
      return (plan_tuberia[tramo_actual + 1].op == -1) ? DIG : RAISE;
    }

    int brujula_dest = -1; 
    if (c_ini > sensores.posC) brujula_dest = 2;      
    else if (c_ini < sensores.posC) brujula_dest = 6; 
    else if (f_ini > sensores.posF) brujula_dest = 4; 
    else if (f_ini < sensores.posF) brujula_dest = 0; 

    if (sensores.rumbo != (Orientacion)brujula_dest) {
      int diff = (brujula_dest - (int)sensores.rumbo + 8) % 8;
      return (diff <= 4) ? TURN_SR : TURN_SL;
    }

    if (sensores.enfrente) { 
      if (!esperando_install) {
        esperando_install = true;
        return IDLE; 
      } else {
        esperando_install = false;
        tramo_actual++; 
        esperando_tecnico = false; 
        terreno_preparado = false; 
        
        if (tramo_actual >= plan_tuberia.size() - 1) plan_tuberia.clear(); 
        return INSTALL; 
      }
    } else {
      esperando_install = false; 
    }
    return IDLE; 
  }

  // Condicion 2: ¿Estamos en la casilla INICIO y no hemos llamado al tecnico?
  else if (sensores.posF == f_ini && sensores.posC == c_ini && !esperando_tecnico) {
      
    if (!llamado_en_ini) {
      llamado_en_ini = true;
      return COME; 
    }

    if (tramo_actual == 0 && plan_tuberia[tramo_actual].op != 0 && !terreno_preparado) {
      terreno_preparado = true;
      return (plan_tuberia[tramo_actual].op == -1) ? DIG : RAISE;
    }
    
    terreno_preparado = false; 
    llamado_en_ini = false; 
    esperando_tecnico = true; 
    hayPlan = false; 
    // return IDLE; 
  }

  // Condicion 3: Viajes 
  else {
    int target_f = esperando_tecnico ? f_fin : f_ini;
    int target_c = esperando_tecnico ? c_fin : c_ini;

    int df = target_f - sensores.posF;
    int dc = target_c - sensores.posC;

    // OPTIMIZACIÓN MÁXIMA: Si el objetivo está pegado a nosotros, 
    // calculamos la ruta matemáticamente al instante. Adiós a los 3-4s de parón.
    if (abs(df) <= 1 && abs(dc) <= 1 && (df != 0 || dc != 0)) {
      int bruj_req = -1;
      if (df == -1 && dc == 0) bruj_req = 0;
      else if (df == -1 && dc == 1) bruj_req = 1;
      else if (df == 0 && dc == 1) bruj_req = 2;
      else if (df == 1 && dc == 1) bruj_req = 3;
      else if (df == 1 && dc == 0) bruj_req = 4;
      else if (df == 1 && dc == -1) bruj_req = 5;
      else if (df == 0 && dc == -1) bruj_req = 6;
      else if (df == -1 && dc == -1) bruj_req = 7;

      if (sensores.rumbo != bruj_req) {
        int diff = (bruj_req - (int)sensores.rumbo + 8) % 8;
        return (diff <= 4) ? TURN_SR : TURN_SL;
      } else {
        if (sensores.agentes[2] == 't') return IDLE; // Esperar al técnico pacientemente
        return WALK;
      }
    }
    // Si está lejos (solo el viaje inicial), usamos Búsqueda en Anchura
    else {
      if (!hayPlan) { 
        EstadoI ini_est = {{sensores.posF, sensores.posC, sensores.rumbo}, tengo_zapatillas};
        EstadoI fin_est = {{target_f, target_c, (Orientacion)0}, tengo_zapatillas};
        plan = B_Anchura_V2(ini_est, fin_est, mapaResultado, mapaCotas);
        hayPlan = !plan.empty();
      }
      
      if (hayPlan && !plan.empty()) { 
        Action a = plan.front();
        
        // FIN DEL BUCLE INFINITO DE CPU: Si chocamos, simplemente esperamos
        if (a == WALK && sensores.agentes[2] == 't') return IDLE; 
        
        plan.pop_front();
        return a;
      } else {
        hayPlan = false;
      }
    }
  }

  return IDLE;
}

/*
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores) {
  
  // Planificacion inicial de tuberias
  if (plan_tuberia.empty()) {
    EstadoTub inicio = {(int)sensores.BelPosF, (int)sensores.BelPosC, (int)mapaCotas[sensores.BelPosF][sensores.BelPosC]};
    vector<pair<int, int>> plantas;

    for (int f = 0; f < mapaResultado.size(); f++) 
      for (int c = 0; c < mapaResultado[f].size(); c++) 
        if (mapaResultado[f][c] == 'U') plantas.push_back({f, c});

    list<Paso> lista_plan = AlgoritmoAEstrellaTub(inicio, plantas, mapaResultado, mapaCotas, sensores);
    plan_tuberia.assign(lista_plan.begin(), lista_plan.end());
    tramo_actual = 0;
    esperando_tecnico = false; 
    esperando_install = false; 
    llamado_en_ini = false; // Reset de la llamada anticipada
    hayPlan = false;
    if (!plan_tuberia.empty()) VisualizaRedTuberias(lista_plan);
  }

  // Si ya hemos terminado toda la red, nos quedamos en IDLE
  if (tramo_actual >= plan_tuberia.size() - 1) return IDLE;

  int f_ini = plan_tuberia[tramo_actual].fil;
  int c_ini = plan_tuberia[tramo_actual].col;
  int f_fin = plan_tuberia[tramo_actual + 1].fil;
  int c_fin = plan_tuberia[tramo_actual + 1].col;

  // Condicion 1: ¿Estamos en la casilla FIN (f_fin) y ya hemos llamado al tecnico?
  if (sensores.posF == f_fin && sensores.posC == c_fin && esperando_tecnico) {
      
    // Acondicionar NUESTRO propio terreno en f_fin si hace falta
    if (plan_tuberia[tramo_actual + 1].op != 0 && !terreno_preparado) {
      terreno_preparado = true;
      return (plan_tuberia[tramo_actual + 1].op == -1) ? DIG : RAISE;
    }

    // Nos orientamos hacia el Técnico
    int brujula_dest = -1; 
    if (c_ini > sensores.posC) brujula_dest = 2;      
    else if (c_ini < sensores.posC) brujula_dest = 6; 
    else if (f_ini > sensores.posF) brujula_dest = 4; 
    else if (f_ini < sensores.posF) brujula_dest = 0; 

    // OPTIMIZACIÓN: Giro Inteligente. Calcula la distancia más corta
    if (sensores.rumbo != (Orientacion)brujula_dest) {
      int diff = (brujula_dest - (int)sensores.rumbo + 8) % 8;
      return (diff <= 4) ? TURN_SR : TURN_SL;
    }

    // Instalación con sincronización perfecta de 1 turno
    if (sensores.enfrente) { 
      if (!esperando_install) {
        esperando_install = true;
        return IDLE; 
      } else {
        esperando_install = false;
        tramo_actual++; 
        esperando_tecnico = false; 
        terreno_preparado = false; 
        
        if (tramo_actual >= plan_tuberia.size() - 1) {
          plan_tuberia.clear(); 
        }
        return INSTALL; 
      }
    } else {
      esperando_install = false; 
    }
    return IDLE; 
  }

  // Condicion 2: si estamos en la casilla de inicio y no hemos llamado al tecnico
  else if (sensores.posF == f_ini && sensores.posC == c_ini && !esperando_tecnico) {
      
    // OPTIMIZACIÓN: Llamada anticipada ANTES de excavar
    if (!llamado_en_ini) {
      llamado_en_ini = true;
      return COME; 
    }

    // Excavamos solo si es el Tramo 0 para proteger las cotas
    if (tramo_actual == 0 && plan_tuberia[tramo_actual].op != 0 && !terreno_preparado) {
      terreno_preparado = true;
      return (plan_tuberia[tramo_actual].op == -1) ? DIG : RAISE;
    }
    
    // Preparativos listos, pasamos al viaje
    terreno_preparado = false; 
    llamado_en_ini = false; 
    esperando_tecnico = true; 
    hayPlan = false; 
    return IDLE; 
  }

  // Condicion 3: Viajes 
  else {
    int target_f = esperando_tecnico ? f_fin : f_ini;
    int target_c = esperando_tecnico ? c_fin : c_ini;

    if (!hayPlan) { 
      EstadoI ini_est = {{sensores.posF, sensores.posC, sensores.rumbo}, tengo_zapatillas};
      EstadoI fin_est = {{target_f, target_c, (Orientacion)0}, tengo_zapatillas};
      plan = B_Anchura_V2(ini_est, fin_est, mapaResultado, mapaCotas);
      hayPlan = !plan.empty();
    }
    
    if (hayPlan && !plan.empty()) { 
      Action a = plan.front();
      
      // ESQUIVAR AL TÉCNICO (Recalculando ruta)
      /*
      if (a == WALK && sensores.agentes[2] == 't') {
        int f_fr = sensores.posF, c_fr = sensores.posC;
        switch (sensores.rumbo) {
          case 0: f_fr--; break; case 1: f_fr--; c_fr++; break; case 2: c_fr++; break; case 3: f_fr++; c_fr++; break;
          case 4: f_fr++; break; case 5: f_fr++; c_fr--; break; case 6: c_fr--; break; case 7: f_fr--; c_fr--; break;
        }

        char orig = mapaResultado[f_fr][c_fr];
        mapaResultado[f_fr][c_fr] = 'P'; 
        
        EstadoI ini_est = {{sensores.posF, sensores.posC, sensores.rumbo}, tengo_zapatillas};
        EstadoI fin_est = {{target_f, target_c, (Orientacion)0}, tengo_zapatillas};
        
        list<Action> plan_alternativo = B_Anchura_V2(ini_est, fin_est, mapaResultado, mapaCotas);
        mapaResultado[f_fr][c_fr] = orig; 
        
        // Asignamos el plan si hay desvío, si no esperamos sin machacar la CPU
        if (!plan_alternativo.empty()) {
          plan = plan_alternativo;
          a = plan.front();
          plan.pop_front();
          return a;
        } else {
          return IDLE; 
        }
      }
        //---------
      plan.pop_front();
      return a;
    } else {
      hayPlan = false;
    }
  }

  return IDLE;
}
*/

/*

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores) {
  
  // Planificacion inicial de tuberias
  if (plan_tuberia.empty()) {
    EstadoTub inicio = {(int)sensores.BelPosF, (int)sensores.BelPosC, (int)mapaCotas[sensores.BelPosF][sensores.BelPosC]};
    vector<pair<int, int>> plantas;

    for (int f = 0; f < mapaResultado.size(); f++) 
      for (int c = 0; c < mapaResultado[f].size(); c++) 
        if (mapaResultado[f][c] == 'U') plantas.push_back({f, c});

    list<Paso> lista_plan = AlgoritmoAEstrellaTub(inicio, plantas, mapaResultado, mapaCotas, sensores);
    plan_tuberia.assign(lista_plan.begin(), lista_plan.end());
    tramo_actual = 0;
    esperando_tecnico = false; 
    esperando_install = false; 
    llamado_en_ini = false; // Reset de la optimizacion
    hayPlan = false;
    if (!plan_tuberia.empty()) VisualizaRedTuberias(lista_plan);
  }

  // Si ya hemos terminado toda la red, nos quedamos en IDLE
  if (tramo_actual >= plan_tuberia.size() - 1) return IDLE;

  int f_ini = plan_tuberia[tramo_actual].fil;
  int c_ini = plan_tuberia[tramo_actual].col;
  int f_fin = plan_tuberia[tramo_actual + 1].fil;
  int c_fin = plan_tuberia[tramo_actual + 1].col;

  // Condicion 1: ¿Estamos en la casilla FIN (f_fin) y ya hemos llamado al tecnico?
  if (sensores.posF == f_fin && sensores.posC == c_fin && esperando_tecnico) {
      
    // Acondicionar NUESTRO propio terreno en f_fin si hace falta
    if (plan_tuberia[tramo_actual + 1].op != 0 && !terreno_preparado) {
      terreno_preparado = true;
      return (plan_tuberia[tramo_actual + 1].op == -1) ? DIG : RAISE;
    }

    // Nos orientamos hacia el Técnico
    int brujula_dest = -1; 
    if (c_ini > sensores.posC) brujula_dest = 2;      
    else if (c_ini < sensores.posC) brujula_dest = 6; 
    else if (f_ini > sensores.posF) brujula_dest = 4; 
    else if (f_ini < sensores.posF) brujula_dest = 0; 

    // OPTIMIZACIÓN: Giro Inteligente. Calcula si es mejor girar a izq o der.
    if (sensores.rumbo != (Orientacion)brujula_dest) {
      int diff = (brujula_dest - (int)sensores.rumbo + 8) % 8;
      return (diff <= 4) ? TURN_SR : TURN_SL;
    }

    // Instalación con la Sincronización del motor
    if (sensores.enfrente) { 
      if (!esperando_install) {
        esperando_install = true;
        return IDLE; 
      } else {
        esperando_install = false;
        tramo_actual++; 
        esperando_tecnico = false; 
        terreno_preparado = false; 
        
        if (tramo_actual >= plan_tuberia.size() - 1) {
          plan_tuberia.clear(); 
        }
        return INSTALL; 
      }
    } else {
      esperando_install = false; 
    }
    return IDLE; 
  }

  // Condicion 2: si estamos en la casilla de inicio y no hemos llamado al tecnico
  else if (sensores.posF == f_ini && sensores.posC == c_ini && !esperando_tecnico) {
      
    // OPTIMIZACIÓN: Llamamos al técnico nada más pisar la casilla, antes de cavar.
    if (!llamado_en_ini) {
      llamado_en_ini = true;
      return COME; 
    }

    // Excavamos solo si es el Tramo 0
    if (tramo_actual == 0 && plan_tuberia[tramo_actual].op != 0 && !terreno_preparado) {
      terreno_preparado = true;
      return (plan_tuberia[tramo_actual].op == -1) ? DIG : RAISE;
    }
    
    // Ya hemos llamado y preparado: pasamos a fase de viaje
    terreno_preparado = false; 
    llamado_en_ini = false; // Reset para el siguiente tramo
    esperando_tecnico = true; 
    hayPlan = false; 
    return IDLE; // Cedemos el turno, en el próximo entraremos en Viajes
  }

  // Condicion 3: Viajes 
  else {
    int target_f = esperando_tecnico ? f_fin : f_ini;
    int target_c = esperando_tecnico ? c_fin : c_ini;

    if (!hayPlan) { 
      EstadoI ini_est = {{sensores.posF, sensores.posC, sensores.rumbo}, tengo_zapatillas};
      EstadoI fin_est = {{target_f, target_c, (Orientacion)0}, tengo_zapatillas};
      plan = B_Anchura_V2(ini_est, fin_est, mapaResultado, mapaCotas);
      hayPlan = !plan.empty();
    }
    
    if (hayPlan && !plan.empty()) { 
      Action a = plan.front();
      
      // ESQUIVAR OBSTÁCULO: Tu versión original que funciona para sobrevivir
      if (a == WALK && sensores.agentes[2] == 't') {
        int f_fr = sensores.posF, c_fr = sensores.posC;
        switch (sensores.rumbo) {
          case 0: f_fr--; break; case 1: f_fr--; c_fr++; break; case 2: c_fr++; break; case 3: f_fr++; c_fr++; break;
          case 4: f_fr++; break; case 5: f_fr++; c_fr--; break; case 6: c_fr--; break; case 7: f_fr--; c_fr--; break;
        }

        char orig = mapaResultado[f_fr][c_fr];
        mapaResultado[f_fr][c_fr] = 'P'; 
        
        EstadoI ini_est = {{sensores.posF, sensores.posC, sensores.rumbo}, tengo_zapatillas};
        EstadoI fin_est = {{target_f, target_c, (Orientacion)0}, tengo_zapatillas};
        plan = B_Anchura_V2(ini_est, fin_est, mapaResultado, mapaCotas);
        hayPlan = !plan.empty();
        mapaResultado[f_fr][c_fr] = orig; 
        
        if (hayPlan && !plan.empty()) {
          a = plan.front();
          plan.pop_front();
          return a;
        } else {
          return IDLE; 
        }
      }
      plan.pop_front();
      return a;
    } else {
      hayPlan = false;
    }
  }

  return IDLE;
}
  */
/*
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores){

  vector<pair<int, int>> plantas;
  list<Paso> lista_plan;

  // Nivel 4. Si no hay plan de tuberias lo buscamos 
    if (plan_tuberia.empty()){
      // Iniciamos el plan en la pos de la belkanita
      EstadoTub inicio = {(int)sensores.BelPosF, (int)sensores.BelPosC, (int)mapaCotas[sensores.BelPosF][sensores.BelPosC]};

      // vector de plantas de tratamiento de residuos
      for (int f = 0; f < mapaResultado.size(); f++) 
        for (int c = 0; c < mapaResultado[f].size(); c++) 
          if (mapaResultado[f][c] == 'U') plantas.push_back({f, c});

      lista_plan = AlgoritmoAEstrellaTub(inicio, plantas, mapaResultado, mapaCotas, sensores);
      plan_tuberia.assign(lista_plan.begin(), lista_plan.end());
      tramo_actual = 0;
      if (plan_tuberia.size() > 0)
        VisualizaRedTuberias(lista_plan);  // pintamos el plan
      hayPlan = false;  // reiniciamos para el futuro
    }

  if (tramo_actual != plan_tuberia.size()-1){ // mientras no estemos en el ultimo tramo del plan

    // Ingeniero va a la casilla del tramo actual del plan para DIG/RAISE y llamar al tec
    EstadoI fin = {{plan_tuberia[tramo_actual].fil, plan_tuberia[tramo_actual].col}}; // Estado al que queremos llegar (inicio de la tuberia)
    EstadoI ini = {{sensores.posF, sensores.posC, sensores.rumbo}, tengo_zapatillas};
    plan = B_Anchura_V2(ini, fin, mapaResultado, mapaCotas);
    VisualizaPlan(ini.site,plan);
    hayPlan = plan.size() != 0;

    // Hasta que no estemos en la casilla de inicio, vamos ejecutando el plan, pero sin que este avisado el tecnico
    if (sensores.posF != fin.site.f || sensores.posC != fin.site.c && hayPlan){
      if (hayPlan && plan.size() > 0){
        // Ingeniero va a dicha casilla mediante el algoritmo diseñado en el nivel 2
        Action accion = plan.front();
        plan.pop_front();
        return accion;
      }
    }

    // el ingeniero ya está en la casilla inicial
    hayPlan = false;

    // Realizamos si es necesario DIG / RAISE
    int op_ini = plan_tuberia[0].op;

    if (op_ini != 0){ // si es dig o raise lo ejecutamos
      Action accion = (op_ini == -1) ? DIG : RAISE;
      return accion;
    }
    
    // Nos vamos a nuestra casilla (plan(tramo_actual + 1)) mediante el alg del nivel 2 de nuevo
    fin = {plan_tuberia[tramo_actual + 1].fil, plan_tuberia[tramo_actual + 1].col};
    ini = {{sensores.posF, sensores.posC, sensores.rumbo}, tengo_zapatillas};

    // Llamamos al Técnico y nos vamos a nuestra casilla
    if (sensores.posF == fin.site.f && sensores.posC == fin.site.c && !sensores.venpaca && avisado == 0){
      avisado = 1;  // avisamos y COME
      return COME;
    }

    // misma logica que antes
    plan = B_Anchura_V2(ini, fin, mapaResultado, mapaCotas);
    VisualizaPlan(ini.site,plan);
    hayPlan = plan.size() != 0;

    // Hasta que no estemos en la casilla de inicio, vamos ejecutando el plan 
    if (sensores.posF != plan_tuberia[tramo_actual + 1].fil || sensores.posC != plan_tuberia[tramo_actual + 1].col){
      if (hayPlan && plan.size() > 0){
        // Ingeniero va a dicha casilla mediante el algoritmo diseñado en el nivel 2
        Action accion = plan.front();
        plan.pop_front();
        return accion;
      }
    }

    // Ingeniero gira hasta que esté el tecnico en frente
    if (!sensores.agentes[2] == 'i'){
      return TURN_SR;
    }

    // Solo cuando este en frente, en el sentido de mirándose--> INSTALL
    if (sensores.enfrente){
      tramo_actual++;
      avisado = 0; // reseteamos para la proxima llamada del ingeniero
      return INSTALL; // instalamos y nos movemos al siguiente tramo
    }
  } // if grande para plan tuberias
  
  
  return IDLE; // nivel finalizado
}
*/

/*
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores)
{
  vector<pair<int, int>> plantas;
  list<Paso> lista_plan;

  switch (estado_actual)
  {
    case PLANIFICANDO:
    {
      if (plan_tuberia.empty()){
        EstadoTub inicio = {(int)sensores.BelPosF, (int)sensores.BelPosC, (int)mapaCotas[sensores.BelPosF][sensores.BelPosC]};

        for (int f = 0; f < mapaResultado.size(); f++) 
          for (int c = 0; c < mapaResultado[f].size(); c++) 
            if (mapaResultado[f][c] == 'U') plantas.push_back({f, c});

        lista_plan = AlgoritmoAEstrellaTub(inicio, plantas, mapaResultado, mapaCotas, sensores);
        plan_tuberia.assign(lista_plan.begin(), lista_plan.end());
        tramo_actual = 0;
        hayPlan = false; 
      }
      estado_actual = PREPARANDO_INICIO; 
      return IDLE;
    }
    
    case PREPARANDO_INICIO:
    {
      // Preparamos la casilla inicial de la Belkanita si lo requiere
      int op_ini = plan_tuberia[0].op;
      if (tramo_actual == 0 && op_ini != 0 && !terreno_preparado) {
          terreno_preparado = true;
          if (op_ini == -1) return DIG;
          if (op_ini == 1) return RAISE;
      }
      terreno_preparado = false; 
      estado_actual = LLAMANDO_TECNICO; 
      return IDLE;
    }

    case LLAMANDO_TECNICO:
    {
      // Estamos pisando tramo_actual. Llamamos al técnico aquí.
      hayPlan = false; 
      estado_actual = YENDO_A_DESTINO;
      return COME;
    }
    
    case YENDO_A_DESTINO:
    {
      // Viajamos a la siguiente casilla (tramo_actual + 1)
      int dest_f = plan_tuberia[tramo_actual + 1].fil;
      int dest_c = plan_tuberia[tramo_actual + 1].col;

      if (sensores.posF != dest_f || sensores.posC != dest_c){
        if (!hayPlan){ 
          EstadoI ini = {{sensores.posF, sensores.posC, sensores.rumbo}, tengo_zapatillas};
          plan = B_Anchura_V2(ini, {{dest_f, dest_c, (Orientacion)0}, tengo_zapatillas}, mapaResultado, mapaCotas);
          hayPlan = true;
        }
        if (!plan.empty()) { 
          Action sig_accion = plan.front(); 
          if (sig_accion == WALK && sensores.agentes[2] == 't') return IDLE; // Seguridad anticaídas
          plan.pop_front();
          return sig_accion;  
        } else {
          hayPlan = false;
          return IDLE;
        }
      } else { 
        // Ya hemos llegado a i+1
        estado_actual = PREPARANDO_DESTINO;
      }
      return IDLE;
    }

    case PREPARANDO_DESTINO:
    {
      // Preparamos i+1 mientras lo pisamos
      int op = plan_tuberia[tramo_actual + 1].op;
      if (op != 0 && !terreno_preparado) {
          terreno_preparado = true;
          if (op == -1) return DIG;
          if (op == 1) return RAISE;
      }
      terreno_preparado = false; 
      estado_actual = ALINEANDO;
      return IDLE;
    }
    
    case ALINEANDO: 
    {
      // Nos orientamos hacia ATRÁS, donde está el técnico (tramo_actual)
      int tec_f = plan_tuberia[tramo_actual].fil;
      int tec_c = plan_tuberia[tramo_actual].col;
      int brujula_dest = -1; 

      if (tec_c > (int)sensores.posC) brujula_dest = 2; // Este
      else if (tec_c < (int)sensores.posC) brujula_dest = 6; // Oeste
      else if (tec_f > (int)sensores.posF) brujula_dest = 4; // Sur
      else if (tec_f < (int)sensores.posF) brujula_dest = 0; // Norte

      if (sensores.rumbo != (Orientacion)brujula_dest) return TURN_SR;

      estado_actual = INSTALANDO;
      return IDLE;
    }
    
    case INSTALANDO: 
    {
      // ¡Sincronización instantánea guiada por el simulador!
      if (!sensores.enfrente) {
          return IDLE; 
      }

      tramo_actual++;
      hayPlan = false;

      if (tramo_actual >= (int)plan_tuberia.size() - 1) estado_actual = PLANIFICANDO; 
      else estado_actual = LLAMANDO_TECNICO; 

      return INSTALL;
    }
  }
  return IDLE;
}
*/
/**
 * @brief Comportamiento del ingeniero para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_6(Sensores sensores)
{
  return IDLE;
}


// Heuristica para la eleccion de tuberias, usamos la distancia de manhattan ya que 
// ahora si que solo se pueden usar orientaciones ortogonales (norte,sur,este,oeste)
int ComportamientoIngeniero::HeuristicaTuberia(int f, int c, const vector<pair<int, int>> &plantas) {
  int min_dist = 999999;

  for (auto p : plantas) {
    // Distancia Manhattan pura (para tuberías es ideal porque solo van en cruz)
    int dist = abs(f - p.first) + abs(c - p.second);
    if (dist < min_dist) {
      min_dist = dist;
    }
  }

  return min_dist; // Devolvemos la distancia a la planta más óptima
}

// Método para calcular el coste de ENERGÍA de alterar/instalar en una casilla
int ComportamientoIngeniero::CosteEnergiaTub(int op, unsigned char t_destino) {
  int coste = 0;
  
  if (op == 0) { // INSTALL normal (sin alterar terreno)
    if (t_destino == 'A') coste = 60;
    else if (t_destino == 'H') coste = 45;
    else if (t_destino == 'S') coste = 25;
    else if (t_destino == 'C' || t_destino == 'U') coste = 15;
    else coste = 30; 
  } 
  else if (op == -1){ // DIG
    if (t_destino == 'H') coste = 65;
    else if (t_destino == 'S') coste = 40;
    else if (t_destino == 'C' || t_destino == 'U') coste = 25;
    else coste = 50;
  }
  else if (op == 1){ // RAISE
    if (t_destino == 'H') coste = 55;
    else if (t_destino == 'S') coste = 30;
    else if (t_destino == 'C' || t_destino == 'U') coste = 10;
    else coste = 40;  
  }
  // No hay coste adicional por altura
  return coste;
}


int ComportamientoIngeniero::ImpactoEcologicoTub(int op, unsigned char t_destino) {
  int impacto = 0;
  
  if (op == 0) { // INSTALL
    if (t_destino == 'A') impacto = 50;
    else if (t_destino == 'H') impacto = 45;
    else if (t_destino == 'S') impacto = 25;
    else if (t_destino == 'C' || t_destino == 'U') impacto = 15;
    else impacto = 30;
  } 
  else if (op == -1){ // DIG
    if (t_destino == 'H') impacto = 65;
    else if (t_destino == 'S') impacto = 40;
    else if (t_destino == 'C' || t_destino == 'U') impacto = 25;
    else impacto = 50;
  } 
  else if (op == 1){ // RAISE
    if (t_destino == 'H') impacto = 55;
    else if (t_destino == 'S') impacto = 30;
    else if (t_destino == 'C' || t_destino == 'U') impacto = 10;
    else impacto = 40;
  }
  
  return impacto;
}

bool ComportamientoIngeniero::CasillaAccesibleTuberia(int f, int c, const vector<vector<unsigned char>> &terreno) {
  // Comprobamos límites del mapa
  if (f < 0 || f >= terreno.size() || c < 0 || c >= terreno[0].size()) return false;
  
  unsigned char t = terreno[f][c];
  // Rechazamos Precipicios, Muros y Bosques
  if (t == 'P' || t == 'M' || t == 'B') return false; 
  
  return true;
}

#include <map>

list<Paso> ComportamientoIngeniero::AlgoritmoAEstrellaTub(const EstadoTub &inicio, const vector<pair<int, int>> &plantas, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura, Sensores sensores) {
  
  priority_queue<NodoTub, vector<NodoTub>, std::greater<NodoTub>> abierta;
  
  // Usamos un map para de cada Estado poder guardar el impacto mínimo alcanzado
  map<EstadoTub, int> memoria_eco;

  unsigned char casilla_ini = terreno[inicio.f][inicio.c];
  int alt_original_ini = (int)altura[inicio.f][inicio.c];

  // Creamos los nodos de INSTALL, DIG, RAISE en la casilla de inicio
  for (int op = -1; op <= 1; op++) {

    if (casilla_ini == 'A' && op != 0) continue; // dig y raise en agua prohibido
    if (op == 1 && alt_original_ini >= 9) continue;
    if (op == -1 && alt_original_ini <= 1) continue; // restricciones de altura

    // Al inicio solo almacenamos la modificación del terreno
    int impacto_ini = (op != 0) ? ImpactoEcologicoTub(op, casilla_ini) : 0;
    int energia_ini = (op != 0) ? CosteEnergiaTub(op, casilla_ini) : 0; // Si hay modificacion si "cobramos"
    // Si es install sumaremos sus costes en el siguiente nodo

    if (impacto_ini > sensores.max_ecologico) continue; // Controlamos impacto ecologico

    NodoTub primero;
    primero.estado = {inicio.f, inicio.c, alt_original_ini + op};
    primero.longitud = 0;
    primero.energia = energia_ini;
    primero.ecologico = impacto_ini;
    primero.f = primero.longitud + HeuristicaTuberia(inicio.f, inicio.c, plantas);
    primero.secuencia.push_back({inicio.f, inicio.c, op});

    abierta.push(primero); // añadimos el nodo a la lista de abiertos
  }

  while (!abierta.empty()) {
    NodoTub current = abierta.top();
    abierta.pop();

    // Condicion de parada: llegar a una 'U'
    for (const auto &p : plantas) {
      if (current.estado.f == p.first && current.estado.c == p.second) {
        return current.secuencia;
      }
    }

    // Si ya pasamos por este nodo con un impacto igual o menor, descartamos esta rama (poda)
    if (memoria_eco.count(current.estado) && memoria_eco[current.estado] <= current.ecologico) {
      continue;
    }

    // Guardamos en el map el impacto eco (menor hasta ahora o primero registrado) 
    memoria_eco[current.estado] = current.ecologico; 

    // Arrays para ccombinarlos y obtener N, S, E, O
    int df[] = {-1, 1, 0, 0}, dc[] = {0, 0, 1, -1};

    // Probamos con las distintas direcciones N, S, E, O
    for (int dir = 0; dir < 4; dir++) {
      int next_f = current.estado.f + df[dir];
      int next_c = current.estado.c + dc[dir];

      // Comprobamos si es accesible la casilla
      if (!CasillaAccesibleTuberia(next_f, next_c, terreno)) continue;

      for (int op = -1; op <= 1; op++) { // Bucle para probar DIG, INSTALL, RAISE
        int alt_dest_orig = (int)altura[next_f][next_c];
        int alt_dest_final = alt_dest_orig + op;

        // Regla de consistencia de altura h_fin == h_curr o h_fin == h_curr - 1
        if (alt_dest_final != current.estado.altura && alt_dest_final != current.estado.altura - 1) continue;
        // si es distinto y no es igual q la acctual - 1. Entonces descartamos.

        unsigned char t_origen = terreno[current.estado.f][current.estado.c];
        unsigned char t_destino = terreno[next_f][next_c];

        // Negamos operacion DIG o RAISE en agua
        if ((t_destino == 'A') && op != 0) continue;

        // Comprobamos restricciones de altura
        if (op == 1 && alt_dest_orig >= 9) continue;
        if (op == -1 && alt_dest_orig <= 1) continue;

        // Impacto = INSTALL(origen) + INSTALL(destino) + MOD(destino)
        int i_paso = ImpactoEcologicoTub(0, t_origen) + ImpactoEcologicoTub(0, t_destino);
        int e_paso = CosteEnergiaTub(0, t_origen) + CosteEnergiaTub(0, t_destino);
        // Aqui queda calculado el INSTALL en casilla origen y destino

        if (op != 0) { // Si hubiera DIG o RAISE en la destino, lo añadimos también
          i_paso += ImpactoEcologicoTub(op, t_destino);
          e_paso += CosteEnergiaTub(op, t_destino);
        }

        // Acumulados del nodo
        int n_impacto = current.ecologico + i_paso;
        int n_energia = current.energia + e_paso;

        // Comprobamos impacto y energia
        if (n_impacto > sensores.max_ecologico || n_energia > sensores.energia) continue;


        // Una vez pasadas todas las podas, entonces creamos el EstadoTub del nodo hijo
        EstadoTub st_hijo = {next_f, next_c, alt_dest_final};

        // Poda preventiva: si ya conocemos un camino mejor a este hijo, ni lo metemos
        if (!memoria_eco.count(st_hijo) || memoria_eco[st_hijo] > n_impacto) {
          NodoTub hijo;
          hijo.estado = st_hijo;
          hijo.longitud = current.longitud + 1;
          hijo.energia = n_energia;
          hijo.ecologico = n_impacto;
          hijo.f = hijo.longitud + HeuristicaTuberia(next_f, next_c, plantas);
          hijo.secuencia = current.secuencia;
          hijo.secuencia.push_back({next_f, next_c, op});

          abierta.push(hijo);
        }
      }
    }
  }
  return list<Paso>();
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

EstadoI ComportamientoIngeniero::NextCasillaIngeniero(const EstadoI &st){
  EstadoI siguiente = st;

  switch (st.site.brujula)
  {
    case norte:
    siguiente.site.f = st.site.f - 1;
    break;
    case noreste:
    siguiente.site.f = st.site.f - 1;
    siguiente.site.c = st.site.c + 1;
    break;
    case este:
    siguiente.site.c = st.site.c + 1;
    break;
    case sureste:
    siguiente.site.f = st.site.f + 1;
    siguiente.site.c = st.site.c + 1;
    break;
    case sur:
    siguiente.site.f = st.site.f + 1;
    break;
    case suroeste:
    siguiente.site.f = st.site.f + 1;
    siguiente.site.c = st.site.c - 1;
    break;
    case oeste:
    siguiente.site.c = st.site.c - 1;
    break;
    case noroeste:
    siguiente.site.f = st.site.f - 1;
    siguiente.site.c = st.site.c - 1;
  }

  return siguiente;
}


bool ComportamientoIngeniero::CasillaAccesibleIngeniero(const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
  EstadoI next = applyT(WALK, st, terreno, altura); // Simulamos un paso
  
  // Comprobamos limites
  if (next.site.f < 0 || next.site.f >= terreno.size() || next.site.c < 0 || next.site.c >= terreno[0].size()) return false;
  
  unsigned char t = terreno[next.site.f][next.site.c];
  if (t == 'P' || t == 'M' || t == 'B') return false; // Intransitables son precipicio, muro y bosque
  
  int dif = abs((int)altura[next.site.f][next.site.c] - (int)altura[st.site.f][st.site.c]);
  if ((!st.zapatillas && dif <= 1) || (st.zapatillas && dif <= 2)) return true; // con zap <=2
  
  return false;
}

bool ComportamientoIngeniero::CasillaAccesibleSalto(const EstadoI &st, 
  const vector<vector<unsigned char>> &terreno, 
  const vector<vector<unsigned char>> &altura) {

  // Casilla intermedia: solo comprobar tipo, NO altura
  EstadoI intermedio = applyT(WALK, st, terreno, altura);
  if (intermedio.site.f < 0 || intermedio.site.f >= (int)terreno.size() ||
      intermedio.site.c < 0 || intermedio.site.c >= (int)terreno[0].size()) return false;
  
  unsigned char ti = terreno[intermedio.site.f][intermedio.site.c];
  if (ti == 'P' || ti == 'M' || ti == 'B') return false;

  // Casilla destino: tipo y diferencia de altura respecto al INICIO
  EstadoI destino = applyT(JUMP, st, terreno, altura);
  if (destino.site.f < 0 || destino.site.f >= (int)terreno.size() ||
      destino.site.c < 0 || destino.site.c >= (int)terreno[0].size()) return false;

  unsigned char td = terreno[destino.site.f][destino.site.c];
  if (td == 'P' || td == 'M' || td == 'B') return false;

  // Diferencia de altura entre INICIO y DESTINO FINAL
  int dif = abs((int)altura[destino.site.f][destino.site.c] - 
                (int)altura[st.site.f][st.site.c]);
  int maxDif = st.zapatillas ? 2 : 1;
  return dif <= maxDif;
}


EstadoI ComportamientoIngeniero::applyT(Action accion, const EstadoI & st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  EstadoI next = st;
  bool f_dentro, c_dentro;
  switch(accion){
    case WALK:
      next = NextCasillaIngeniero(st);

      f_dentro = (next.site.f >= 0 && next.site.f < terreno.size());
      c_dentro = (next.site.c >= 0 && next.site.c < terreno[0].size());
      
      if (f_dentro && c_dentro && terreno[next.site.f][next.site.c] == 'D')
        next.zapatillas = true;
      break;

    case JUMP:
      
      // Avanzamos ahora dos casillas puesto que es un salto
      next = NextCasillaIngeniero(st);
      next = NextCasillaIngeniero(next);

      f_dentro = (next.site.f >= 0 && next.site.f < terreno.size());
      c_dentro = (next.site.c >= 0 && next.site.c < terreno[0].size());
      
      if (f_dentro && c_dentro && terreno[next.site.f][next.site.c] == 'D')
        next.zapatillas = true;

      break;

    case TURN_SR:
      next.site.brujula = (Orientacion) ((next.site.brujula+1)%8);
      break;

    case TURN_SL:
      next.site.brujula = (Orientacion) ((next.site.brujula+7)%8);
      break;
  }

  return next;
}


bool ComportamientoIngeniero::Find (const NodoI & st, const list<NodoI> &lista){
  auto it = lista.begin();

  while (it != lista.end() and !((*it) == st)){
    it++;
  }

  return (it != lista.end());
}

list<Action> ComportamientoIngeniero::B_Anchura_V2(const EstadoI &inicio, const EstadoI &final, const vector<vector<unsigned char>> &terreno, vector<vector<unsigned char>> &altura){
  
  queue<NodoI> frontier;
  set<EstadoI> explored; 
  list<Action> path;

  NodoI current_node;
  current_node.estado = inicio;
  
  // Zapatillas iniciales
  if (terreno[inicio.site.f][inicio.site.c] == 'D') {
    current_node.estado.zapatillas = true;
  }
  
  frontier.push(current_node);
  explored.insert(current_node.estado);

  while (!frontier.empty()){
    current_node = frontier.front();
    frontier.pop();

    // Condicion de meta, compronamos
    if (current_node.estado.site.f == final.site.f && current_node.estado.site.c == final.site.c) {
      return current_node.secuencia; // devolvemos sec en caso de haber llegado a la meta
    }

    // Hijo JUMP, primera opción porque es lo mas "rentable" para llegar en menos pasos
    if (CasillaAccesibleSalto(current_node.estado, terreno, altura)) {
      NodoI child = current_node;
      child.estado = applyT(JUMP, current_node.estado, terreno, altura);
      if (explored.find(child.estado) == explored.end()) {
        explored.insert(child.estado);
        child.secuencia.push_back(JUMP); // Siempre 
        frontier.push(child);
      }
    }

    // Hijo WALK
    if (CasillaAccesibleIngeniero(current_node.estado, terreno, altura)) {
      NodoI child = current_node;
      child.estado = applyT(WALK, current_node.estado, terreno, altura);
      if (explored.find(child.estado) == explored.end()) {
        explored.insert(child.estado);
        child.secuencia.push_back(WALK);
        frontier.push(child);
      }
    }

    // Hijo TURN_SL
    NodoI child_SL = current_node;
    child_SL.estado = applyT(TURN_SL, current_node.estado, terreno, altura);
    if (explored.find(child_SL.estado) == explored.end()) {
      explored.insert(child_SL.estado);
      child_SL.secuencia.push_back(TURN_SL);
      frontier.push(child_SL);
    }

    // Hijo TURN_SR
    NodoI child_SR = current_node;
    child_SR.estado = applyT(TURN_SR, current_node.estado, terreno, altura);
    if (explored.find(child_SR.estado) == explored.end()) {
      explored.insert(child_SR.estado);
      child_SR.secuencia.push_back(TURN_SR);
      frontier.push(child_SR);
    }
  }

  return path;
}
  

void ComportamientoIngeniero::AnularMatriz(vector<vector<unsigned char>> &m){
  for (int i = 0; i < m[0].size(); i++){
    for (int j = 0; j < m.size(); j++){
      m[i][j] = 0;
    }
  }
}

list<Action> ComportamientoIngeniero::CaminoDijkstra(const EstadoI& inicio, const EstadoI &final, const vector<vector<unsigned char>>& terreno, const vector<vector<unsigned char>>& altura) {
  priority_queue<NodoI, vector<NodoI>, greater<NodoI>> frontier;
  set<EstadoI> explored; // Guardamos estados ya procesados
  
  NodoI current;
  current.estado = inicio;
  current.coste = 0;
  if (terreno[inicio.site.f][inicio.site.c] == 'D') current.estado.zapatillas = true;
  
  frontier.push(current);

  while (!frontier.empty()) {
      current = frontier.top();
      frontier.pop();

      // 1. CONDICIÓN DE SALIDA (VITAL): Comprobar meta AL SACAR de la cola
      if (current.estado.site.f == final.site.f && current.estado.site.c == final.site.c) {
          return current.secuencia;
      }

      // 2. Si ya hemos explorado este estado con un coste menor, saltamos
      if (explored.find(current.estado) != explored.end()) continue;
      explored.insert(current.estado);

      // 3. Generar hijos (WALK, TURN_SR, TURN_SL)
      vector<Action> acciones = {WALK, TURN_SR, TURN_SL};
      for (Action a : acciones) {
          if (a == WALK && !CasillaAccesibleIngeniero(current.estado, terreno, altura)) continue;

          EstadoI nuevo = applyT(a, current.estado, terreno, altura);
          
          // Si pisamos zapatillas mentalmente, el estado se actualiza
          if (a == WALK && terreno[nuevo.site.f][nuevo.site.c] == 'D') nuevo.zapatillas = true;

          NodoI hijo;
          hijo.estado = nuevo;
          hijo.coste = current.coste + 1; // En Nivel 2 cada acción vale 1
          hijo.secuencia = current.secuencia;
          hijo.secuencia.push_back(a);

          if (explored.find(hijo.estado) == explored.end()) {
              frontier.push(hijo);
          }
      }
  }
  return list<Action>();
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

// Meter el coste de install cuando hago dig y raise tambien 
// mirar en monitor.cpp para ver como lo corrigen