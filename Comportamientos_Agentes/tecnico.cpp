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
    // case 3: accion = ComportamientoTecnicoNivel_E(sensores); break;
    case 3: accion = ComportamientoTecnicoNivel_3(sensores); break;
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

  // Actualizamos variable tengo_zapatillas (Aunque irrelevanta para tecnico en este nivel)
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
  char i = ViablePorAltura(sensores.superficie[1], sensores.cota[1] - sensores.cota[0]);
  char c = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0]);
  char d = ViablePorAltura(sensores.superficie[3], sensores.cota[3] - sensores.cota[0]);

  // Comprobamos ademas que el ingeniero no esté en ninguna de las casillas
  if (sensores.agentes[1] == 'i') i = 'P'; 
  // si está, la marcamos como precipicio para no pasar
  if (sensores.agentes[2] == 'i') c = 'P';
  if (sensores.agentes[3] == 'i') d = 'P';
  // No esquivamos con este agente, simplemente marcamos para no ir. Ingeniero es quien esquiva girando

  
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

  // Llegados a este punto, pos==0 luego ninguna casilla adyacente interesante.
  
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
 
  // Si llevamos demasiados pasos pisando casillas ya pisadas, estamos atrapados en un bucle, provocamos giro 180º
  if (turnos_aburrido > umbral_aburrimiento) {
    turnos_aburrido = 0;
    giros_pendientes=3; // Para dar la vuelta de 180 (4 giros de 45)
    return TURN_SR;
  }
    

  if (puedo_avanzar){
    accion = WALK;
    giros_consecutivos = 0;
  }
  else{   // Vemos que casilla ha sido menos visitada si la izq o la derecha

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
      
        giros_consecutivos++;
    }
  }


  // si llevamos 8 giros consecutivos (vuelta completa) entonces IDLE, porque estamos encerrados
  if (giros_consecutivos >= 8){
    accion =  IDLE;
    giros_consecutivos = 0;
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
  if (sensores.agentes[1] == 'i') i = 'P'; 
  // si está, la 'marcamos' como precipicio para no pasar
  if (sensores.agentes[2] == 'i') c = 'P';
  if (sensores.agentes[3] == 'i') d = 'P';

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
  Action accion = IDLE;

  if (sensores.superficie[0] == 'D') {
    tengo_zapatillas = true;
  }

  if (!hayPlan){
    // Invocar al metodo de busqueda
    EstadoT inicio, fin;
    inicio.site.f = sensores.posF;
    inicio.site.c = sensores.posC;
    inicio.site.brujula = sensores.rumbo;
    inicio.zapatillas = tengo_zapatillas;

    fin.site.f = sensores.BelPosF;
    fin.site.c = sensores.BelPosC;
    
    plan = AlgoritmoAEstrella(inicio, fin, mapaResultado, mapaCotas);
    VisualizaPlan(inicio.site,plan);
    hayPlan = plan.size() != 0;
  }

  if (hayPlan && plan.size()>0){
    accion = plan.front();

    if (accion == WALK && sensores.agentes[2] == 'i')
      return IDLE;
    
    plan.pop_front();
  }

  if (plan.size()==0){
    hayPlan = false;
  }

  return accion;
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
    
    // Si el ingeniero hace COME, actualizamos todo para ir hacia alla
    if (sensores.venpaca) {
      destino_f = sensores.GotoF;
      destino_c = sensores.GotoC;
      tengo_orden = true; 
      hayPlan = false;
    }

    // Si tenemos una orden activa
    if (tengo_orden) {
        
      // Si todavia no hemos llegado a donde el ingeniero nos llamó
      if (sensores.posF != destino_f || sensores.posC != destino_c) {
        if (!hayPlan) {  // si no hay plan al destino lo creamos (Nivel 3)
          EstadoT inicio, fin;
          inicio.site.f = sensores.posF;
          inicio.site.c = sensores.posC;
          inicio.site.brujula = sensores.rumbo;
          inicio.zapatillas = tengo_zapatillas;
          fin.site.f = destino_f;
          fin.site.c = destino_c;
          
          plan = AlgoritmoAEstrella(inicio, fin, mapaResultado, mapaCotas);
          hayPlan = !plan.empty();
        }
        if (hayPlan && !plan.empty()) { // si hay, seguimos ejecutándolo
          Action a = plan.front();
          
          // Caso de choque con ingeniero
          if (a == WALK && sensores.agentes[2] == 'i'){
            
            int f_fr = sensores.posF, c_fr = sensores.posC;
            switch (sensores.rumbo) {
              case 0: f_fr--; break; case 1: f_fr--; c_fr++; break; case 2: c_fr++; break; case 3: f_fr++; c_fr++; break;
              case 4: f_fr++; break; case 5: f_fr++; c_fr--; break; case 6: c_fr--; break; case 7: f_fr--; c_fr--; break;
            } // averiguamos la casilla donde está el ingeniero (como está en frente)
            // solo es ver hacia donde miramos para calcularlo (ej: 0 (Norte) fila--)

            // Si la casilla que nos bloquea es exactamente nuestro destino,
            // NO calculamos el A*, simplemente esperamos pacientemente a que el ing se mueva de ahi
            if (f_fr == destino_f && c_fr == destino_c) {
              return IDLE; 
            }
            
            char original = mapaResultado[f_fr][c_fr]; 
            mapaResultado[f_fr][c_fr] = 'P';  // Lo marcamos como precipicio   
            
            EstadoT inicio, fin;
            inicio.site.f = sensores.posF; inicio.site.c = sensores.posC;
            inicio.site.brujula = sensores.rumbo; inicio.zapatillas = tengo_zapatillas;
            fin.site.f = destino_f; fin.site.c = destino_c;
            
            // Recalculamos el A* para ir, pero con dicha casilla ahora marcada como 'P'
            list<Action> plan_alternativo = AlgoritmoAEstrella(inicio, fin, mapaResultado, mapaCotas); 
            mapaResultado[f_fr][c_fr] = original; 
            
            if (!plan_alternativo.empty()) { // ejecutamos plan alternativo
                plan = plan_alternativo;
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
    // Si ya hemos llegado al destino 
    else {
        // Si estamos ya encuadrados, repetimos INSTALL hasta poder instalar tuberia
        if (sensores.enfrente) {
          return INSTALL; 
        }

        // Si el ingeniero no está justo delante, rotamos a la derecha hasta encontrarlo
        if (sensores.agentes[2] != 'i') {
          return TURN_SR; 
        }
      }
    }

    return IDLE;
}


bool ComportamientoTecnico::es_caminoNivel_6(unsigned char c) const {
  // El técnico explora todo lo que no sea caída, muro o bosque (a menos que tenga zapatillas, 
  // pero para no complicar, con evitar P, M y B suele bastar para abrir mapa).
  return (c != 'P' && c != 'M' && c != 'B'); // QUITAMOS prohibicion agua por energia 
}

bool ComportamientoTecnico::EsCasillaTransitableLevel6(int f, int c, bool tieneZapatillas) {
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size()) 
    return false;

  return es_caminoNivel_6(mapaResultado[f][c]) || (mapaResultado[f][c] == 'B' && tieneZapatillas); 
}


/**
 * @brief Comportamiento del técnico para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */


 Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {
  // 1. Actualización constante del mapa
  if (sensores.posF != -1) {
    ActualizarMapa(sensores);
    if (sensores.superficie[0] == 'D') tengo_zapatillas = true;
  }

  // 2. Lógica del COME
  if (sensores.venpaca) {
    cont_come++;
    modo_construccion = true;
    if (cont_come > 1) agua_permitida = true; // Si nos llama en plena obra, vía libre al agua
  }

  // 3. Ahorro de energía en los primeros turnos
  if (turnos_IDLE < 250 && !modo_construccion) {
    turnos_IDLE++;
    return IDLE;
  }

  Action accion_final = IDLE;

  if (modo_construccion) {
    vector<pair<int,int>> mod_agua;
    
    // Si estamos en el primer viaje y aún intentamos salvar la batería evitando el agua
    bool evitar_agua = (cont_come <= 1 && !agua_permitida);

    if (evitar_agua) {
      for (int f = 0; f < mapaResultado.size(); f++) {
        for (int c = 0; c < mapaResultado[f].size(); c++) {
          if (mapaResultado[f][c] == 'A') {
            mapaResultado[f][c] = 'M'; // Tapamos SOLO el agua conocida
            mod_agua.push_back({f, c});
          }
        }
      }
    }

    accion_final = ComportamientoTecnicoNivel_5(sensores); 

    // Restauramos el agua
    for (auto &p : mod_agua) {
      mapaResultado[p.first][p.second] = 'A';
    }

    // ¡EL FIX ANTI-ATASCOS! 
    // Si intentamos rodear el agua pero el A* no encontró camino, significa que NO HAY RUTA SECA.
    if (evitar_agua && plan.empty() && tengo_orden) {
        agua_permitida = true; // Levantamos el veto al agua para siempre
        accion_final = ComportamientoTecnicoNivel_5(sensores); // Recalculamos dejando que pise el agua
    }

  } else {
    accion_final = AdaptadaComportamientoTecnicoNivel_1(sensores);
  }  

  // --- MEGA FILTRO SALVAVIDAS ---
  if (accion_final == WALK) {
    unsigned char obj = sensores.superficie[2];
    int desnivel = abs(sensores.cota[2] - sensores.cota[0]);
    bool altura_mala = (desnivel > 1); 

    // ¡IMPORTANTE! El '?' NO SE BLOQUEA. Si es niebla, damos el paso con fe.
    bool obstaculo_mortal = (obj == 'P' || obj == 'M' || (obj == 'B' && !tengo_zapatillas) || altura_mala);
    
    // El agua solo frena al agente si la tenemos prohibida
    bool prohibir_agua = (!agua_permitida && obj == 'A'); 

    if (obstaculo_mortal || prohibir_agua) {
      plan.clear(); hayPlan = false; return IDLE;
    } else if (sensores.agentes[2] == 'i') {
      return IDLE;
    }
  }
  
  return accion_final;
}

/*
 Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {
  // 1. Actualización constante del mapa y objetos
  if (sensores.posF != -1) {
    ActualizarMapa(sensores);
    if (sensores.superficie[0] == 'D') tengo_zapatillas = true;
  }

    // --- LA LÓGICA DEL CONTADOR DE COME ---
  if (sensores.venpaca) {
    cont_come++;
    modo_construccion = true; 
  }

  // 2. Ahorro de energía inicial
  if (turnos_IDLE < 250 && !modo_construccion) { // turnos_IDLE < 250 && !sensores.venpaca
    turnos_IDLE++;
    return IDLE;
  }

  Action accion_final = IDLE;

  if (modo_construccion) {
    // === ENVOLTORIO ANTI-AGUA CONDICIONADO AL PRIMER VIAJE ===
    vector<pair<int,int>> aguas;
    
    // Solo bloqueamos el agua si es la PRIMERA orden del ingeniero (el viaje largo)
    if (cont_come <= 1) {
      for (int f = 0; f < mapaResultado.size(); f++) {
        for (int c = 0; c < mapaResultado[f].size(); c++) {
          if (mapaResultado[f][c] == 'A') {
            mapaResultado[f][c] = 'M';
            aguas.push_back({f, c});
          }
        }
      }
    }

    accion_final = ComportamientoTecnicoNivel_5(sensores); 

    // Restauramos el agua
    if (cont_come <= 1) {
      for (auto &p : aguas) {
        if (mapaResultado[p.first][p.second] == 'M') {
          mapaResultado[p.first][p.second] = 'A';
        }
      }
    }

  } else {
    // Si aún no hay órdenes, exploramos
    accion_final = AdaptadaComportamientoTecnicoNivel_1(sensores);
  }  

  // 3. --- MEGA FILTRO SALVAVIDAS CONDICIONADO ---
  if (accion_final == WALK) {
    unsigned char obj = sensores.superficie[2];
    int desnivel = abs(sensores.cota[2] - sensores.cota[0]);
    bool altura_mala = (desnivel > 1); 

    // Obstáculos mortales SIEMPRE se bloquean
    bool obstaculo_mortal = (obj == 'P' || obj == 'M' || (obj == 'B' && !tengo_zapatillas) || altura_mala);
    
    // El agua SOLO se bloquea si estamos en el primer viaje
    bool agua_prohibida = (cont_come <= 1 && obj == 'A');

    if (obstaculo_mortal || agua_prohibida) {
      plan.clear(); hayPlan = false; return IDLE;
    } else if (sensores.agentes[2] == 'i') {
      return IDLE;
    }
  }
  
  return accion_final;
}
  */



/*
Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {

  // Actualización de mapa y de zapatillas
  if (sensores.posF != -1) {
    ActualizarMapa(sensores);
    if (sensores.superficie[0] == 'D') tengo_zapatillas = true;
  }

  // revisamos si estamos en modo construccion
  if (sensores.venpaca || tengo_orden) modo_construccion = true;

  // Si no estamso en modo construccion y turnos < 250, entonces IDLE, guardamos energia
  if (!modo_construccion && turnos_IDLE < 250) {
    turnos_IDLE++;
    return IDLE;
  }

  Action accion_final = IDLE; 

  if (modo_construccion) {
    // ¡FUERA ENVOLTORIOS!
    accion_final = ComportamientoTecnicoNivel_5(sensores); 
  } else {
    accion_final = AdaptadaComportamientoTecnicoNivel_1(sensores);
  }  

  // --- MEGA FILTRO SALVAVIDAS ---
  if (accion_final == WALK) {
    unsigned char obj = sensores.superficie[2];
    int desnivel = abs(sensores.cota[2] - sensores.cota[0]);
    bool altura_mala = (desnivel > 1); 

    if (obj == 'P' || obj == 'M' || (obj == 'B' && !tengo_zapatillas) || obj == 'A' || altura_mala) {  // QUITADO OBJ == A
      plan.clear(); 
      hayPlan = false;
      return IDLE;
    } else if (sensores.agentes[2] == 'i') {
      return IDLE;  // esperamos a que el ingeniero se quite
    }
  }
  return accion_final;
}
*/


/*
Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {
  // Actualización general
  if (sensores.posF != -1) ActualizarMapa(sensores);

  if (sensores.venpaca || tengo_orden) {
    modo_construccion = true; 
  }

  if (modo_construccion) {
    // === PATRÓN ENVOLTORIO PARA PROTEGER EL NIVEL 5 ===
    
    vector<pair<int,int>> modificadas;
    for (int f = 0; f < mapaResultado.size(); f++) {
      for (int c = 0; c < mapaResultado[f].size(); c++) {
        if (mapaResultado[f][c] == '?') {
          mapaResultado[f][c] = 'M';
          modificadas.push_back({f, c});
        }
      }
    }

    // El Nivel 5 planificará su ruta sin pisar las casillas ciegas
    Action accion_final = ComportamientoTecnicoNivel_5(sensores); 

    // Restauramos
    for (auto &p : modificadas) {
      if (mapaResultado[p.first][p.second] == 'M') {
        mapaResultado[p.first][p.second] = '?';
      }
    }

    return accion_final;

  } else {
    return AdaptadaComportamientoTecnicoNivel_1(sensores);
  }  
}
*/

int ComportamientoTecnico::VeoCasillaInteresanteNivel6(char i, char c, char d, bool zap, ubicacion actual, int belF, int belC){
  if (!zap) {
    if (c == 'D') return 2;
    else if (i == 'D') return 1;
    else if (d == 'D') return 3;
  }

  ubicacion izq = actual; izq.brujula = (Orientacion) (((int) actual.brujula + 7) % 8);
  ubicacion casilla_i = Delante(izq);
  ubicacion casilla_c = Delante(actual);
  ubicacion der = actual; der.brujula = (Orientacion) (((int) actual.brujula + 1) % 8);
  ubicacion casilla_d = Delante(der);

  // 1. Calculamos distancias e imán PRIMERO
  int dist_actual = (belF != -1) ? abs(actual.f - belF) + abs(actual.c - belC) : 0;
  
  // ¡NUEVO! Si estamos prácticamente encima, apagamos el imán para siempre
  if (dist_actual <= 3 && belF != -1) belkanita_encontrada = true; 
  bool usar_iman = !belkanita_encontrada && (belF != -1); 

  int dist_i = (belF != -1) ? abs(casilla_i.f - belF) + abs(casilla_i.c - belC) : 0;
  int dist_c = (belF != -1) ? abs(casilla_c.f - belF) + abs(casilla_c.c - belC) : 0;
  int dist_d = (belF != -1) ? abs(casilla_d.f - belF) + abs(casilla_d.c - belC) : 0;

  // 2. Asignamos visitas con la PENALIZACIÓN DE AGUA DINÁMICA
  int visitas_i = INT_MAX, visitas_c = INT_MAX, visitas_d = INT_MAX;

  if (casilla_i.f >= 0 && casilla_i.f < mapaVisitados.size() && casilla_i.c >= 0 && casilla_i.c < mapaVisitados[0].size()) {
      visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];
      if (mapaResultado[casilla_i.f][casilla_i.c] == 'H') visitas_i += 3;
      if (mapaResultado[casilla_i.f][casilla_i.c] == 'A') {
          // Si nos acerca al imán, ¡el agua vale la pena! Penalización mínima.
          if (usar_iman && dist_i < dist_actual) visitas_i += 1; 
          else visitas_i += 50; // Si no, huimos del agua
      }
  }
  if (casilla_c.f >= 0 && casilla_c.f < mapaVisitados.size() && casilla_c.c >= 0 && casilla_c.c < mapaVisitados[0].size()) {
      visitas_c = mapaVisitados[casilla_c.f][casilla_c.c];
      if (mapaResultado[casilla_c.f][casilla_c.c] == 'H') visitas_c += 3;
      if (mapaResultado[casilla_c.f][casilla_c.c] == 'A') {
          if (usar_iman && dist_c < dist_actual) visitas_c += 1;
          else visitas_c += 50;
      }
  }
  if (casilla_d.f >= 0 && casilla_d.f < mapaVisitados.size() && casilla_d.c >= 0 && casilla_d.c < mapaVisitados[0].size()) {
      visitas_d = mapaVisitados[casilla_d.f][casilla_d.c];
      if (mapaResultado[casilla_d.f][casilla_d.c] == 'H') visitas_d += 3;
      if (mapaResultado[casilla_d.f][casilla_d.c] == 'A') {
          if (usar_iman && dist_d < dist_actual) visitas_d += 1;
          else visitas_d += 50;
      }
  }

  // 3. Elegimos la mejor opción
  int mejor_opcion = 0;  
  int menor_visitas = INT_MAX;
  int menor_distancia = INT_MAX; 

  if (c != 'P' && EsCasillaTransitableLevel6(casilla_c.f, casilla_c.c, zap)) {
    if (visitas_c < menor_visitas || (usar_iman && visitas_c == menor_visitas && dist_c < menor_distancia)) {
      menor_visitas = visitas_c; menor_distancia = dist_c; mejor_opcion = 2; 
    }
  }
  if (i != 'P' && EsCasillaTransitableLevel6(casilla_i.f, casilla_i.c, zap)) {
    if (visitas_i < menor_visitas || (usar_iman && visitas_i == menor_visitas && dist_i < menor_distancia)) {
      menor_visitas = visitas_i; menor_distancia = dist_i; mejor_opcion = 1; 
    }
  }
  if (d != 'P' && EsCasillaTransitableLevel6(casilla_d.f, casilla_d.c, zap)) {
    if (visitas_d < menor_visitas || (usar_iman && visitas_d == menor_visitas && dist_d < menor_distancia)) {
      menor_visitas = visitas_d; menor_distancia = dist_d; mejor_opcion = 3; 
    }
  }

  return mejor_opcion; 
}

/*
int ComportamientoTecnico::VeoCasillaInteresanteNivel6(char i, char c, char d, bool zap, ubicacion actual, int belF, int belC) {

  // Buscamos las zapatillas si no las tenemos aun 
  if (!tengo_zapatillas) {
    if (c == 'D') return 2;
    else if (i == 'D') return 1;
    else if (d == 'D') return 3;
  }

  // Calculamos ubicaciones de las casillas adyacentes
  ubicacion izq = actual;
  izq.brujula = (Orientacion) (((int) actual.brujula + 7) % 8);
  ubicacion casilla_i = Delante(izq);

  ubicacion casilla_c = Delante(actual);

  ubicacion der = actual;
  der.brujula = (Orientacion) (((int) actual.brujula + 1) % 8);
  ubicacion casilla_d = Delante(der);

  // Procedemos a buscar el minimo de visitas
  int visitas_i = INT_MAX, visitas_c = INT_MAX, visitas_d = INT_MAX;

  // Asignamos el valor correspondiente si están en el rango
  if (casilla_i.f >= 0 && casilla_i.f < mapaVisitados.size() && casilla_i.c >= 0 && casilla_i.c < mapaVisitados[0].size())
      visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];

  if (casilla_c.f >= 0 && casilla_c.f < mapaVisitados.size() && casilla_c.c >= 0 && casilla_c.c < mapaVisitados[0].size())
      visitas_c = mapaVisitados[casilla_c.f][casilla_c.c];

  if (casilla_d.f >= 0 && casilla_d.f < mapaVisitados.size() && casilla_d.c >= 0 && casilla_d.c < mapaVisitados[0].size())
      visitas_d = mapaVisitados[casilla_d.f][casilla_d.c];
    
  // --- INSTINTO DIRECCIONAL CONDICIONADO ---
  int dist_actual = (belF != -1) ? abs(actual.f - belF) + abs(actual.c - belC) : 0;
  
  // Umbral dinámico mejorado:
  int umbral_iman = 15; // Por defecto para mapas pequeños (<= 30)
  if (mapaResultado.size() > 30) {
    umbral_iman = mapaResultado.size() / 3;
  }
  
  bool lejos = (dist_actual > umbral_iman); // Apagamos el imán si estamos dentro del umbral

  int dist_i = (belF != -1) ? abs(casilla_i.f - belF) + abs(casilla_i.c - belC) : 0;
  int dist_c = (belF != -1) ? abs(casilla_c.f - belF) + abs(casilla_c.c - belC) : 0;
  int dist_d = (belF != -1) ? abs(casilla_d.f - belF) + abs(casilla_d.c - belC) : 0;

  // Elegimos la casilla menos visitada y desempatamos por cercanía
  int mejor_opcion = 0;  
  int menor_visitas = INT_MAX;
  int menor_distancia = INT_MAX; 

  // Evaluamos FRENTE
  if (c != 'P' && EsCasillaTransitableLevel6(casilla_c.f, casilla_c.c, zap)) {
    if (visitas_c < menor_visitas || (visitas_c == menor_visitas && dist_c < menor_distancia)) {
      menor_visitas = visitas_c;
      menor_distancia = dist_c;
      mejor_opcion = 2; // WALK
    }
  }
  
  // Evaluamos IZQUIERDA
  if (i != 'P' && EsCasillaTransitableLevel6(casilla_i.f, casilla_i.c, zap)) {
    if (visitas_i < menor_visitas || (lejos && visitas_i == menor_visitas && dist_i < menor_distancia)) {
      menor_visitas = visitas_i;
      menor_distancia = dist_i;
      mejor_opcion = 1; // TURN_SL
    }
  }
  
  // Evaluamos DERECHA
  if (d != 'P' && EsCasillaTransitableLevel6(casilla_d.f, casilla_d.c, zap)) {
    if (visitas_d < menor_visitas || (lejos && visitas_d == menor_visitas && dist_d < menor_distancia)) {
      menor_visitas = visitas_d;
      menor_distancia = dist_d;
      mejor_opcion = 3; // TURN_SR
    }
  }

  return mejor_opcion; 
}
*/


Action ComportamientoTecnico::AdaptadaComportamientoTecnicoNivel_1(Sensores sensores) {
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
  if (sensores.agentes[1] == 'i') i = 'P'; 
  // si está, la 'marcamos' como precipicio para no pasar
  if (sensores.agentes[2] == 'i') c = 'P';
  if (sensores.agentes[3] == 'i') d = 'P';

  // Evaluamos cual de las casillas es mas conveniente, 0 si ninguna 
  int pos = VeoCasillaInteresanteNivel6(i, c, d, tengo_zapatillas, actual, sensores.BelPosF, sensores.BelPosC);

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

  bool puedo_avanzar = (EsCasillaTransitableLevel6(delante.f, delante.c, tengo_zapatillas) && EsAccesiblePorAltura(actual) && !sensores.choque);

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


      if (EsCasillaTransitableLevel6(casilla_i.f, casilla_i.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_i)){
        visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];
        if (mapaResultado[casilla_i.f][casilla_i.c] == 'A') visitas_i += 50;
        if (mapaResultado[casilla_i.f][casilla_i.c] == 'H') visitas_i += 3;
      }

      if (EsCasillaTransitableLevel6(casilla_d.f, casilla_d.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_d)){
        visitas_d = mapaVisitados[casilla_d.f][casilla_d.c];
        if (mapaResultado[casilla_d.f][casilla_d.c] == 'A') visitas_d += 50;
        if (mapaResultado[casilla_d.f][casilla_d.c] == 'H') visitas_d += 3;
      }

      // --- IMÁN DE UN SOLO USO PARA GIROS ---
      // 1. Calculamos distancias e imán antes para poder juzgar el agua
      int dist_actual = (sensores.BelPosF != -1) ? abs(actual.f - sensores.BelPosF) + abs(actual.c - sensores.BelPosC) : 0;
      if (dist_actual <= 3 && sensores.BelPosF != -1) belkanita_encontrada = true;
      bool usar_iman = !belkanita_encontrada && (sensores.BelPosF != -1);

      int dist_i = (sensores.BelPosF != -1) ? abs(casilla_i.f - sensores.BelPosF) + abs(casilla_i.c - sensores.BelPosC) : 0;
      int dist_d = (sensores.BelPosF != -1) ? abs(casilla_d.f - sensores.BelPosF) + abs(casilla_d.c - sensores.BelPosC) : 0;

      // 2. Extraemos visitas de izquierda
      if (EsCasillaTransitableLevel6(casilla_i.f, casilla_i.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_i)){
        visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];
        if (mapaResultado[casilla_i.f][casilla_i.c] == 'H') visitas_i += 3;
        if (mapaResultado[casilla_i.f][casilla_i.c] == 'A') {
          if (usar_iman && dist_i < dist_actual) visitas_i += 1;
          else visitas_i += 50;
        }
      }

      // 3. Extraemos visitas de derecha
      if (EsCasillaTransitableLevel6(casilla_d.f, casilla_d.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_d)){
        visitas_d = mapaVisitados[casilla_d.f][casilla_d.c];
        if (mapaResultado[casilla_d.f][casilla_d.c] == 'H') visitas_d += 3;
        if (mapaResultado[casilla_d.f][casilla_d.c] == 'A') {
          if (usar_iman && dist_d < dist_actual) visitas_d += 1;
          else visitas_d += 50;
        }
      }

      // 4. INSTINTO DIRECCIONAL PARA GIROS DE 90 GRADOS
      if (visitas_d < visitas_i) {
        accion = TURN_SR;
      } else if (visitas_i < visitas_d) {
        accion = TURN_SL;
      } else {
        if (usar_iman) {
          accion = (dist_d <= dist_i) ? TURN_SR : TURN_SL;
        } else {
          accion = TURN_SR; 
        }
      }
        
      last_action = accion;
    }
  }


  // si llevamos 8 giros consecutivos (vuelta completa) entonces IDLE, porque estamos encerrados
  if (giros_consecutivos >= 8)
    accion =  IDLE; 

  return accion; // WALK, TURN_SL, TURN_SR, IDLE
}



/*
Action ComportamientoTecnico::AdaptadaComportamientoTecnicoNivel_1(Sensores sensores) {
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
  if (sensores.agentes[1] == 'i') i = 'P'; 
  // si está, la 'marcamos' como precipicio para no pasar
  if (sensores.agentes[2] == 'i') c = 'P';
  if (sensores.agentes[3] == 'i') d = 'P';

  // Evaluamos cual de las casillas es mas conveniente, 0 si ninguna 
  int pos = VeoCasillaInteresanteNivel6(i, c, d, tengo_zapatillas, actual, sensores.BelPosF, sensores.BelPosC);

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

  bool puedo_avanzar = (EsCasillaTransitableLevel6(delante.f, delante.c, tengo_zapatillas) && EsAccesiblePorAltura(actual) && !sensores.choque);

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


      if (EsCasillaTransitableLevel6(casilla_i.f, casilla_i.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_i)){
        visitas_i = mapaVisitados[casilla_i.f][casilla_i.c];
      }

      if (EsCasillaTransitableLevel6(casilla_d.f, casilla_d.c, tengo_zapatillas) && EsAccesiblePorAltura(actual, casilla_d)){
        visitas_d = mapaVisitados[casilla_d.f][casilla_d.c];
      }

      if (visitas_d <= visitas_i)
        accion = TURN_SR;
      else
        accion = TURN_SL;  // meter que sea atraido a la belkanita?
      
      // -----------------------------------------------------------
      /*
      // --- INSTINTO DIRECCIONAL CONDICIONADO PARA GIROS DE 90 GRADOS ---
      if (visitas_d < visitas_i) {
        accion = TURN_SR;
      } else if (visitas_i < visitas_d) {
        accion = TURN_SL;
      } else {
        int dist_actual = (sensores.BelPosF != -1) ? abs(actual.f - sensores.BelPosF) + abs(actual.c - sensores.BelPosC) : 0;
        
        int umbral_iman = 15;  // umbral de 15 para mapas pequeños
        if (mapaResultado.size() > 30) {
          umbral_iman = mapaResultado.size() / 3; // mapa size / 3 para mapas "grandes"
        }
        
        if (dist_actual > umbral_iman && sensores.BelPosF != -1) {
          int dist_i = abs(casilla_i.f - sensores.BelPosF) + abs(casilla_i.c - sensores.BelPosC);
          int dist_d = abs(casilla_d.f - sensores.BelPosF) + abs(casilla_d.c - sensores.BelPosC);
          accion = (dist_d <= dist_i) ? TURN_SR : TURN_SL;
        } else {
          accion = TURN_SR; 
        }
      }
      
      ----------------------------------------------------------
        
      last_action = accion;
    }
  }


  // si llevamos 8 giros consecutivos (vuelta completa) entonces IDLE, porque estamos encerrados
  if (giros_consecutivos >= 8)
    accion =  IDLE; 

  return accion; // WALK, TURN_SL, TURN_SR, IDLE
}
*/


list<Action> AvanzaSaltosDeCaballo(){
  list<Action> secuencia;
  secuencia.push_back(WALK);
  secuencia.push_back(WALK);
  secuencia.push_back(TURN_SR);
  secuencia.push_back(TURN_SR);
  secuencia.push_back(WALK);
  return secuencia;
}

// Método para calcular cual será el coste total de una acción teniendo en cuenta
// a el tipo de casilla y que tipo de acción realiza
int ComportamientoTecnico::CosteEnergiaT(Action accion, const EstadoT &origen, const EstadoT &destino, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
  int coste = 0;
  unsigned char t_origen = terreno[origen.site.f][origen.site.c]; // El coste depende del ORIGEN
  
  if (accion == WALK) {
    // Coste base por terreno y WALK
    if (t_origen == 'A') coste = 60;
    else if (t_origen == 'H') coste = 6;
    else if (t_origen == 'S') coste = 3;
    else return 1; // Camino, Muro, Precipicio, Bosque, etc.
    // Hacemos return porque no hay que sumarle coste de desnivel
    
    // Modificador por desnivel
    int alt_origen = (int)altura[origen.site.f][origen.site.c];
    int alt_destino = (int)altura[destino.site.f][destino.site.c];
    
    if (alt_destino > alt_origen) coste += 5;      // Subida
    else if (alt_destino < alt_origen) coste -= 2; // Bajada
  } 
  else if (accion == TURN_SL || accion == TURN_SR) {
    // Los giros no tienen modificador de altura
    if (t_origen == 'A') coste = 5;
    else if (t_origen == 'H') coste = 2;
    else if (t_origen == 'S') coste = 1;
    else coste = 1;
  } 
  else if(accion == JUMP){  // NOTA: NO SIRVE PARA TECNICO PERO PARA FUTUROS NIVELES QUIZAS ME SIRVE
    // Coste base por terreno y WALK
    if (t_origen == 'A') coste = 90;
    else if (t_origen == 'H') coste = 10;
    else if (t_origen == 'S') coste = 4;
    else coste = 3; // Camino, Sendero, Muro, Precipicio, Bosque, etc. (aunque algunas no se puedan pisar, pon el default) bosque con zapatillas ?????
    
    // Modificador por desnivel
    int alt_origen = (int)altura[origen.site.f][origen.site.c];
    int alt_destino = (int)altura[destino.site.f][destino.site.c];
    
    if (alt_destino > alt_origen) coste += 5;      // Subida
    else if (alt_destino < alt_origen) coste -= 2; // Bajada
  }
  
  return coste;
}

// Usamos en vez de la heuristica de la Distancia Mínima de Manhattan vista en clase de practicas
// la heuristica de Chebyshev que tambien tiene en cuenta diagonales
// Este valor h(n) estima la distancia hasta la meta y se sumará al coste acumulado g(n)
// para obtener el valor total f(n) con el que se ordenará el nodo en el algoritmo A*
int ComportamientoTecnico::Heuristica(const EstadoT &actual, const EstadoT &meta) {
  // Usamos Chebyshev porque el agente puede moverse en las 8 direcciones (diagonales)
  return std::max(abs(actual.site.f - meta.site.f), abs(actual.site.c - meta.site.c));
}

list<Action> ComportamientoTecnico::AlgoritmoAEstrella(const EstadoT &inicio, const EstadoT &final, const vector<vector<unsigned char>> &terreno, vector<vector<unsigned char>> &altura){
  // Nodos por visitar
  priority_queue<NodoT, vector<NodoT>, std::greater<NodoT>> abierta; //ordenados orden ascendente
  
  // Nodos ya visitados
  set<EstadoT> cerrada;

  NodoT primero;
  primero.estado = inicio;

  if (terreno[inicio.site.f][inicio.site.c] == 'D') {
    primero.estado.zapatillas = true; // comprobamos zapatillas 
  }

  primero.coste = 0;
  primero.f = primero.coste + Heuristica(inicio, final);
  abierta.push(primero);


  while (!abierta.empty()){
    NodoT current_node = abierta.top();
    abierta.pop();

    if (current_node.estado.site.f == final.site.f && current_node.estado.site.c == final.site.c){
      return current_node.secuencia;
    }

    if (cerrada.find(current_node.estado) != cerrada.end()){  // Si ya estaba añadido
        continue; // pasamos a la sig iteracion sin añadirlo
    }
    cerrada.insert(current_node.estado); // si no estaba añadido, lo añadimos y calculamos los hijos

    // Generamos ahora a los hijos del nodo
    // WALK
    if (CasillaAccesibleTecnico(current_node.estado, terreno, altura)){
      NodoT walk = current_node;
      walk.estado = applyT(WALK, current_node.estado, terreno, altura);

      if (terreno[walk.estado.site.f][walk.estado.site.c] == 'D') {
        walk.estado.zapatillas = true;
      }

      if (cerrada.find(walk.estado) == cerrada.end()){
        walk.coste = current_node.coste + CosteEnergiaT(WALK, current_node.estado, walk.estado, terreno, altura);
        walk.f = walk.coste + Heuristica(walk.estado, final);
        walk.secuencia.push_back(WALK);
        abierta.push(walk);
      }
    }

    // TURN_SL
    NodoT sl = current_node;
    sl.estado = applyT(TURN_SL, current_node.estado, terreno, altura);
    
    if (cerrada.find(sl.estado) == cerrada.end()){
      sl.coste = current_node.coste + CosteEnergiaT(TURN_SL, current_node.estado, sl.estado, terreno, altura);
      sl.f = sl.coste + Heuristica(sl.estado, final);

      sl.secuencia.push_back(TURN_SL);
      abierta.push(sl); // Lo añadimos a la lista abierta que es la que luego "investigamos"
    }

    // TURN_SR
    NodoT sr = current_node;
    sr.estado = applyT(TURN_SR, current_node.estado, terreno, altura);
    
    if (cerrada.find(sr.estado) == cerrada.end()){
      sr.coste = current_node.coste + CosteEnergiaT(TURN_SR, current_node.estado, sr.estado, terreno, altura);
      sr.f = sr.coste + Heuristica(sr.estado, final);

      sr.secuencia.push_back(TURN_SR);
      abierta.push(sr); 
    }
  }

  // Si la cola se vacía y no hemos salido, es decir, no hay solucion, entonces
  list<Action> path_vacio;
  return path_vacio; // devolvemos lista de acciones vacía.
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

bool ComportamientoTecnico::CasillaAccesibleTecnico(const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  
  EstadoT next = NextCasillaTecnico(st);
  
  // Comprobar que no nos salimos del mapa
  if (next.site.f < 0 || next.site.f >= terreno.size() || 
    next.site.c < 0 || next.site.c >= terreno[0].size()) {
    return false;
  }

  // OPTIMISMO Caso nivel 6 asumimos que podemos pasar aunque sea '?'
  unsigned char t_curr = terreno[st.site.f][st.site.c];
  if (terreno[next.site.f][next.site.c] == '?' || t_curr == '?') return true;

  bool check1 = false, check2 = false, check3 = false;
  check1 = terreno[next.site.f][next.site.c] != 'P' and terreno[next.site.f][next.site.c] != 'M';
  check2 = terreno[next.site.f][next.site.c] != 'B' or (terreno[next.site.f][next.site.c] == 'B' and st.zapatillas);
  
  check3 = abs((int)altura[next.site.f][next.site.c] - (int)altura[st.site.f][st.site.c]) <= 1;
  
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

list<Action> ComportamientoTecnico::B_Anchura_V2(const EstadoT &inicio, const EstadoT &final, const vector<vector<unsigned char>> &terreno, vector<vector<unsigned char>> &altura){
  NodoT current_node;
  list<NodoT> frontier;
  set<NodoT> explored;
  list<Action> path;

  current_node.estado = inicio;
  frontier.push_back(current_node);
  bool SolutionFound = (current_node.estado.site.f == final.site.f && current_node.estado.site.c == final.site.c);

  while (!SolutionFound and !frontier.empty()){
      frontier.pop_front();
      explored.insert(current_node);

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
      else if (explored.find(child_Walk) == explored.end()){
          // Se mete en la lista de frontier después de añadir a secuencia la acción
          child_Walk.secuencia.push_back(WALK);
          frontier.push_back(child_Walk);
      }

      if (!SolutionFound){
          // El hijo resultante de aplicar la accion TURN_SR
          NodoT child_TurnSR = current_node;
          child_TurnSR.estado = applyT(TURN_SR, current_node.estado, terreno, altura);
          if (explored.find(child_TurnSR) == explored.end()){
              child_TurnSR.secuencia.push_back(TURN_SR);
              frontier.push_back(child_TurnSR);
          }

          // El hijo resultante de aplicar la accion TURN_SL
          NodoT child_TurnSL = current_node;
          child_TurnSL.estado = applyT(TURN_SL, current_node.estado, terreno, altura);
          if (explored.find(child_TurnSL) == explored.end()){
              child_TurnSL.secuencia.push_back(TURN_SL);
              frontier.push_back(child_TurnSL);
          }
      }

      // Paso a evaluar el siguiente nodo en la lista "frontier"
      if (!SolutionFound and !frontier.empty()){
          current_node = frontier.front();
          while(explored.find(current_node) != explored.end() && !frontier.empty()){
            frontier.pop_front();
            current_node = frontier.front();
          }
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

