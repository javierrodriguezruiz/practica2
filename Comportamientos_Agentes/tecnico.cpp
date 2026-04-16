#include "tecnico.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>
#include <climits>

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoTecnico::think(Sensores sensores) {
  Action accion = IDLE;


  // Decisión del agente según el nivel
  switch (sensores.nivel) {
    case 0: accion = ComportamientoTecnicoNivel_0(sensores); break;
    case 1: accion = ComportamientoTecnicoNivel_1(sensores); break;
    case 2: accion = ComportamientoTecnicoNivel_2(sensores); break;
    case 3: accion = ComportamientoTecnicoNivel_E(sensores); break;
    // case 3: accion = ComportamientoTecnicoNivel_3(sensores); break;
    case 4: accion = ComportamientoTecnicoNivel_4(sensores); break;
    case 5: accion = ComportamientoTecnicoNivel_5(sensores); break;
    case 6: accion = ComportamientoTecnicoNivel_6(sensores); break;
  }

  return accion;
}


// Niveles del técnico
Action ComportamientoTecnico::ComportamientoTecnicoNivel_0(Sensores sensores) {
  
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

  // Actualizamos variable tengo_zapatillas (Aunque irrelevanta para tecnico en este nivel)
  if (sensores.superficie[0] == 'D') {
    tengo_zapatillas = true;
  }

  // Si encontramos la casilla T. Residuos, entonces terminamos la búsqueda y paramos al player
  if (sensores.superficie[0] == 'U') {
    return IDLE; 
  }


  // BUSQUEDA DE CASILLAS OBJETIVO:
  // Buscamos si son viables las casillas a nuestra izquierda, centro y derecha
  char i = ViablePorAltura(sensores.superficie[1], sensores.cota[1] - sensores.cota[0]);
  char c = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0]);
  char d = ViablePorAltura(sensores.superficie[3], sensores.cota[3] - sensores.cota[0]);

  // Comprobamos ademas que el ingeniero no esté en ninguna de las casillas
  if (sensores.agentes[1] == 'i') i = 'P'; 
  // si está, la marcamos como precipicio para no pasar
  if (sensores.agentes[2] == 'i') c = 'P';
  if (sensores.agentes[3] == 'i') d = 'P';

  
  // Evaluamos cual de las casillas es mas conveniente, 0 si ninguna 
  // Dentro del metodo usamos memoria de casillas visitadas para explorar nuevas casillas
  int pos = VeoCasillaInteresante(i, c, d, actual);
  
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

  
  // Añado aquí lo de que si la veo voy hacia ella ?
  // Analizamos TODA LA VISIÓN del agente por si 'U' estuviera en alguna casilla a la vista
  // salvo las posiciones adyacentes 1, 2 y 3 que ya han sido estudiadas
  // Casillas del frente (si lo vemos en frente y la casilla de delante es camino)
  if ((sensores.superficie[2] == 'U' || sensores.superficie[6] == 'U' || sensores.superficie[12] == 'U') && (es_camino(c) || (c == 'B' && tengo_zapatillas)))
    return WALK;
  
  // Casillas de la izquierda
  // Si lo vemos a la izquierda y la casilla 1 es camino
  if ((sensores.superficie[4] == 'U' || sensores.superficie[5] == 'U' || sensores.superficie[9] == 'U' || sensores.superficie[10] == 'U' || sensores.superficie[11] == 'U') && (es_camino(i) || (i == 'B' && tengo_zapatillas)))
    return TURN_SL;

  // Casillas de la derecha
  // Si lo vemos a la derecha y la casilla 3 es camino (a la que nos dirigimos)
  if ((sensores.superficie[7] == 'U' || sensores.superficie[8] == 'U' || sensores.superficie[13] == 'U' || sensores.superficie[14] == 'U' || sensores.superficie[15] == 'U') && (es_camino(d) || (d == 'B' && tengo_zapatillas)))
    return TURN_SR;
  

  // Si llegamos a este punto, pos == 0, luego no hay ningun objetivo delante
  // Pasamos a explorar:

  // Inicializamos la accion a IDLE 
  Action accion = IDLE;

  bool puedo_avanzar = (EsCasillaTransitableLevel0(delante.f, delante.c, tengo_zapatillas) && EsAccesiblePorAltura(actual) && !sensores.choque);

  // Si podemos avanzar pero en frente tenemos al ingeniero, giraremos
  if (puedo_avanzar && (sensores.agentes[2] == 'i')) {
    puedo_avanzar = false; 
  }
  
  if (puedo_avanzar){
    accion = WALK;
    giros_consecutivos = 0;
  }
  else{

    // Vemos que casilla ha sido menos visitada si la izq o la derecha
    if (giros_consecutivos%2 != 0){ // si no es el primer giro de 45 grados
      accion = last_action; // entonces realizamos el mismo giro que hicimos
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


      if (EsCasillaTransitableLevel0(casilla_i.f, casilla_i.c, tengo_zapatillas) && EsAccesiblePorAltura(casilla_i)){
        visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];

      }

      if (EsCasillaTransitableLevel0(casilla_d.f, casilla_d.c, tengo_zapatillas) && EsAccesiblePorAltura(casilla_d)){
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

/**
 * @brief Comprueba si una celda es de tipo camino transitable.
 * @param c Carácter que representa el tipo de superficie.
 * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
 */
bool ComportamientoTecnico::es_camino(unsigned char c) const {
  return (c == 'C' || c == 'D' || c == 'U' || c == 'S');
}

bool ComportamientoTecnico::es_caminoNivel1(unsigned char c) const {
  return (c == 'C' || c == 'D' || c == 'S');
}




/**
 * @brief Comportamiento reactivo del técnico para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_1(Sensores sensores) {
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
  char i = ViablePorAltura(sensores.superficie[1], sensores.cota[1] - sensores.cota[0]);
  char c = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0]);
  char d = ViablePorAltura(sensores.superficie[3], sensores.cota[3] - sensores.cota[0]);

  // Comprobamos ademas que el ingeniero no esté en ninguna de las casillas
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

  bool puedo_avanzar = (EsCasillaTransitableLevel1(delante.f, delante.c, tengo_zapatillas) && EsAccesiblePorAltura(actual) && !sensores.choque);

  // Si podemos avanzar pero en frente tenemos al ingeniero, giraremos
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
      accion = last_action; // entonces realizamos el mismo giro que hicimos
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


      if (EsCasillaTransitableLevel1(casilla_i.f, casilla_i.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_i)){
        visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];

      }

      if (EsCasillaTransitableLevel1(casilla_d.f, casilla_d.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_d)){
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


/**
 * @brief Comportamiento del técnico para el Nivel 2.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_2(Sensores sensores) {
  return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_3(Sensores sensores) {
  return IDLE;
}
// Para el nivel 3 usaremos la busqueda de coste uniforme

/**
 * @brief Comportamiento del técnico para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_4(Sensores sensores) {
  return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_5(Sensores sensores) {
  return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {
  return IDLE;
}

list<Action> AvanzaSaltosDeCaballo(){
  list<Action> secuencia;
  secuencia.push_back(WALK);
  secuencia.push_back(WALK);
  secuencia.push_back(TURN_SR);
  secuencia.push_back(TURN_SR);
  secuencia.push_back(WALK);
  return secuencia;
}

/**
 * @brief Comportamiento reactivo del técnico para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_E(Sensores sensores) {
  Action accion = IDLE;

  if (!hayPlan){
    // Invocar al metodo de busqueda
    EstadoT inicio, fin;
    inicio.site.f = sensores.posF;
    inicio.site.c = sensores.posC;
    inicio.site.brujula = sensores.rumbo;
    inicio.zapatillas = tengo_zapatillas;
    fin.site.f = sensores.BelPosF;
    fin.site.c = sensores.BelPosC;
    plan = B_Anchura(inicio, fin, mapaResultado, mapaCotas);
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

EstadoT ComportamientoTecnico::NextCasillaTecnico(const EstadoT &st){
  EstadoT siguiente = st;

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

bool ComportamientoTecnico::CasillaAccesibleTecnico(const EstadoT &st, const vector<vector<unsigned char>> &terreno, const
  vector<vector<unsigned char>> &altura){
  
  EstadoT next = NextCasillaTecnico(st);
  bool check1 = false, check2 = false, check3 = false;
  check1 = terreno[next.site.f][next.site.c] != 'P' and terreno[next.site.f][next.site.c] != 'M';
  check2 = terreno[next.site.f][next.site.c] != 'B' or (terreno[next.site.f][next.site.c] == 'B' and st.zapatillas);
  check3 = abs(altura[next.site.f][next.site.c] - altura[st.site.f][st.site.c]) <= 1;
  
  return check1 and check2 and check3;
}

EstadoT ComportamientoTecnico::applyT(Action accion, const EstadoT & st, const vector<vector<unsigned char>> &terreno, const
vector<vector<unsigned char>> &altura){
  EstadoT next = st;
  switch(accion){
    case WALK:
      if (CasillaAccesibleTecnico(st,terreno,altura)){
      next = NextCasillaTecnico(st);
      }
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

bool ComportamientoTecnico::Find (const NodoT & st, const list<NodoT> &lista){
  auto it = lista.begin();

  while (it != lista.end() and !((*it) == st)){
    it++;
  }

  return (it != lista.end());
}

/**
 * @brief Primera aprox a la búsqueda en anchura
 * @param inicio Estado Inicial de la busqueda
 * @param final Estado Final de la busqueda
 * @param terreno Matriz que contiene la información del terreno
 * @param altura Matriz que contiene la altura del mapa.
 * 
 * @return La secuencia de acciones para llegar al estado final
 * @note Devuelve un plan vacío si no es posible encontrar un plan válido
 */
list<Action> ComportamientoTecnico::B_Anchura(const EstadoT &inicio, const EstadoT &final, const vector<vector<unsigned char>> &terreno, vector<vector<unsigned char>> &altura){
  NodoT current_node;
  list<NodoT> frontier;
  list<NodoT> explored;
  list<Action> path;

  current_node.estado = inicio;
  frontier.push_back(current_node);
  bool SolutionFound = (current_node.estado.site.f == final.site.f && current_node.estado.site.c == final.site.c);

  while (!SolutionFound and !frontier.empty()){
      frontier.pop_front();
      explored.push_back(current_node);

      // Compruebo si estoy en una casilla que da las zapatillas
      if (terreno[current_node.estado.site.f][current_node.estado.site.c] == 'D'){
          current_node.estado.zapatillas = true;
      }

      // Genero el hijo resultante de aplicar la acción WALK
      NodoT child_Walk = current_node;
      child_Walk.estado = applyT(WALK, current_node.estado, terreno, altura);
      if (child_Walk.estado.site.f == final.site.f and child_Walk.estado.site.c == final.site.c){
          // El hijo generado es solucion
          child_Walk.secuencia.push_back(WALK);
          current_node = child_Walk;
          SolutionFound = true;
      }
      else if (!Find(child_Walk, frontier) and !Find(child_Walk, explored)){
          // Se mete en la lista de frontier después de añadir a secuencia la acción
          child_Walk.secuencia.push_back(WALK);
          frontier.push_back(child_Walk);
      }

      if (!SolutionFound){
          // El hijo resultante de aplicar la accion TURN_SR
          NodoT child_TurnSR = current_node;
          child_TurnSR.estado = applyT(TURN_SR, current_node.estado, terreno, altura);
          if (!Find(child_TurnSR, frontier) and !Find(child_TurnSR, explored)){
              child_TurnSR.secuencia.push_back(TURN_SR);
              frontier.push_back(child_TurnSR);
          }

          // El hijo resultante de aplicar la accion TURN_SL
          NodoT child_TurnSL = current_node;
          child_TurnSL.estado = applyT(TURN_SL, current_node.estado, terreno, altura);
          if (!Find(child_TurnSL, frontier) and !Find(child_TurnSL, explored)){
              child_TurnSL.secuencia.push_back(TURN_SL);
              frontier.push_back(child_TurnSL);
          }
      }

      // Paso a evaluar el siguiente nodo en la lista "frontier"
      if (!SolutionFound and !frontier.empty()){
          current_node = frontier.front();
          SolutionFound = (current_node.estado.site.f == final.site.f and current_node.estado.site.c == final.site.c);
      }
  }
  // Devuelvo el camino encontrado.

  if (SolutionFound)
    path = current_node.secuencia;

  return path;
}


char ComportamientoTecnico::ViablePorAltura(char casilla, int dif) {
  // El técnico solo puede superar desniveles de 1, sin importar los objetos
  if (abs(dif) <= 1) {
    return casilla;
  } else {
    return 'P';
  }
}

int ComportamientoTecnico::VeoCasillaInteresante(char i, char c, char d, ubicacion actual) {
  // Buscamos la meta
  if (c == 'U') return 2;
  else if (i == 'U') return 1;
  else if (d == 'U') return 3;

  // Buscamos las zapatillas si no las tenemos aun 
  if (!tengo_zapatillas) {
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

  // Si estan en los rangos adecuados entonces accedemos y le asignamos su valor correspondiente
  if (casilla_i.f >= 0 && casilla_i.f < mapaVisitados.size() && casilla_i.c >= 0 && casilla_i.c < mapaVisitados[0].size())
      visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];

  if (casilla_c.f >= 0 && casilla_c.f < mapaVisitados.size() && casilla_c.c >= 0 && casilla_c.c < mapaVisitados[0].size())
      visitas_c = mapaVisitados[casilla_c.f][casilla_c.c];

  if (casilla_d.f >= 0 && casilla_d.f < mapaVisitados.size() && casilla_d.c >= 0 && casilla_d.c < mapaVisitados[0].size())
      visitas_d = mapaVisitados[casilla_d.f][casilla_d.c];
    
  // elegimos la casilla menos visitada siempre que sea camino y sendero

  int mejor_opcion = 0;  // en caso de que no encuentre nada, decide el agente
  int menor = INT_MAX;

  if (c != 'P' && EsCasillaTransitableLevel0(casilla_c.f, casilla_c.c,tengo_zapatillas) && visitas_c < menor) {
    menor = visitas_c;
    mejor_opcion = 2; // WALK
  }
  if (i != 'P' && EsCasillaTransitableLevel0(casilla_i.f, casilla_i.c,tengo_zapatillas) && visitas_i < menor) {
    menor = visitas_i;
    mejor_opcion = 1; // Giramos a izquierda
  }
  if (d != 'P' && EsCasillaTransitableLevel0(casilla_d.f, casilla_d.c,tengo_zapatillas) && visitas_d < menor) {
    menor = visitas_d;
    mejor_opcion = 3; // Giramos a derecha
  }

  return mejor_opcion; 
}

int ComportamientoTecnico::VeoCasillaInteresanteNivel1(char i, char c, char d, bool zap, ubicacion actual) {

  // Buscamos las zapatillas si no las tenemos aun 
  if (!tengo_zapatillas) {
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

  // Si estan en los rangos adecuados entonces accedemos y le asignamos su valor correspondiente
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
void ComportamientoTecnico::ActualizarMapa(Sensores sensores) {
  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];

  int pos = 1;
  switch (sensores.rumbo) {
    case norte:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++) {
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
        for (int i = -j; i <= j; i++) {
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
        for (int i = -j; i <= j; i++) {
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
        for (int i = -j; i <= j; i++) {
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
 * @brief Determina si una casilla es transitable para el técnico.
 * En esta práctica, si el técnico tiene zapatillas, el bosque ('B') es transitable.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable.
 */
bool ComportamientoTecnico::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas) {
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size()) 
    return false;

  return es_camino(mapaResultado[f][c]) || (mapaResultado[f][c] == 'B' && tieneZapatillas);  
  // Solo 'C', 'S', 'D', 'U' son transitables en Nivel 0
}

bool ComportamientoTecnico::EsCasillaTransitableLevel1(int f, int c, bool tieneZapatillas) {
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size()) 
    return false;

  return es_caminoNivel1(mapaResultado[f][c]) || (mapaResultado[f][c] == 'B' && tieneZapatillas);  
  // Solo 'C', 'S', 'D' son transitables en Nivel 1
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el técnico: desnivel máximo siempre 1.
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoTecnico::EsAccesiblePorAltura(const ubicacion &actual) {
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size()) return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (desnivel > 1) return false;
  return true;
}

// Funcion auxiliar cuando queremos saber si dos casillas son accesibles por altura
bool ComportamientoTecnico::EsAccesiblePorAltura(const ubicacion &origen, const ubicacion &destino) {
  if (destino.f < 0 || destino.f >= mapaCotas.size() || destino.c < 0 || destino.c >= mapaCotas[0].size()) {
      return false;
  }
  
  // Calculo del desnivel absoluto
  int desnivel = abs(mapaCotas[destino.f][destino.c] - mapaCotas[origen.f][origen.c]);
  
  // El Técnico solo puede superar un desnivel de +-1 (Zapatillas no le afectan a la altura)
  return desnivel <= 1; 
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoTecnico::Delante(const ubicacion &actual) const {
  ubicacion delante = actual;
  switch (actual.brujula) {
    case 0: delante.f--; break;                        // norte
    case 1: delante.f--; delante.c++; break;     // noreste
    case 2: delante.c++; break;                     // este
    case 3: delante.f++; delante.c++; break;     // sureste
    case 4: delante.f++; break;                        // sur
    case 5: delante.f++; delante.c--; break;     // suroeste
    case 6: delante.c--; break;                     // oeste
    case 7: delante.f--; delante.c--; break;     // noroeste
  }
  return delante;
}


/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::PintaPlan(const list<Action> &plan)
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
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa 2D.
 *
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::VisualizaPlan(const ubicacion &st,
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

