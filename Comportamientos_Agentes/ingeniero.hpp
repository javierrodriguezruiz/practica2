#ifndef COMPORTAMIENTOINGENIERO_H
#define COMPORTAMIENTOINGENIERO_H

#include <chrono>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <time.h>

#include "comportamientos/comportamiento.hpp"


struct EstadoI{
  ubicacion site;
  bool zapatillas;

  bool operator == (const EstadoI &st) const{
    return (site==st.site && zapatillas==st.zapatillas);
  }
  bool operator<(const EstadoI &st) const {
    if (site.f != st.site.f) return site.f < st.site.f;
    if (site.c != st.site.c) return site.c < st.site.c;
    if (site.brujula != st.site.brujula) return site.brujula < st.site.brujula;
    return zapatillas < st.zapatillas;
  }
};

struct NodoI {
  EstadoI estado;
  list<Action> secuencia;
  int coste; //coste acumulado en dicho nodo

  bool operator == (const NodoI &otro) const{
    return estado == otro.estado;
  }


  bool operator >(const NodoI &otro) const {
    return coste > otro.coste;
  }

  // Mantenemos el operador < para el set de explorados
  bool operator <(const NodoI &node) const {
    if (estado.site.f < node.estado.site.f) return true;
    else if (estado.site.f == node.estado.site.f && estado.site.c < node.estado.site.c) return true;
    else if (estado.site.f == node.estado.site.f && estado.site.c == node.estado.site.c && estado.site.brujula < node.estado.site.brujula) return true;
    else if (estado.site.f == node.estado.site.f && estado.site.c == node.estado.site.c && estado.site.brujula == node.estado.site.brujula && estado.zapatillas < node.estado.zapatillas) return true;
    else return false;
  }
};


class ComportamientoIngeniero : public Comportamiento {
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================
  
  /**
   * @brief Constructor para niveles 0, 1 y 6 (sin mapa completo)
   * @param size Tamaño del mapa (si es 0, se inicializa más tarde)
   */
  ComportamientoIngeniero(unsigned int size = 0) : Comportamiento(size) {
    // Inicializar Variables de Estado
    tengo_zapatillas = false;
    last_action = IDLE;
    giros_consecutivos = 0;
    turnos_aburrido=0;
    giros_pendientes=0;
    hayPlan=false;
    plan=list<Action>();
  }

  /**
   * @brief Constructor para niveles 2, 3, 4 y 5 (con mapa completo conocido)
   * @param mapaR Mapa de terreno conocido
   * @param mapaC Mapa de cotas conocido
   */
  ComportamientoIngeniero(std::vector<std::vector<unsigned char>> mapaR, 
                         std::vector<std::vector<unsigned char>> mapaC): 
                         Comportamiento(mapaR, mapaC) {
  }
  

  ComportamientoIngeniero(const ComportamientoIngeniero &comport)
      : Comportamiento(comport) {}
  ~ComportamientoIngeniero() {}

  /**
   * @brief Bucle principal de decisión del agente.
   * Estudia los sensores y decide la siguiente acción.
   * 
   * EJEMPLO DE USO:
   * Action accion = think(sensores);
   * return accion; // El motor ejecutará esta acción
   */
  Action think(Sensores sensores);

  ComportamientoIngeniero *clone() {
    return new ComportamientoIngeniero(*this);
  }

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================

  // Funciones específicas para cada nivel (para ser implementadas por el alumno)
  
  /**
   * @brief Implementación del Nivel 0.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_0(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 1.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_1(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 2.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */ 
  Action ComportamientoIngenieroNivel_2(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 3.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_3(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 4.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_4(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 5.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_5(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 6.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_6(Sensores sensores);

protected:
  // =========================================================================
  // FUNCIONES PROPORCIONADAS
  // =========================================================================

  /**
   * @brief Actualiza la información del mapa interno basándose en los sensores.
   * IMPORTANTE: Esta función ya está implementada. Actualiza mapaResultado y mapaCotas
   * con la información de los 16 sensores (casilla actual + 15 casillas alrededor).
   */
  void ActualizarMapa(Sensores sensores);

  /**
   * @brief Comprueba si una casilla es transitable.
   * @param f Fila de la casilla.
   * @param c Columna de la casilla.
   * @param tieneZapatillas Indica si el agente posee zapatillas.
   * @return true si la casilla es transitable (no es muro ni precipicio).
   */
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);
  bool EsCasillaTransitableLevel1(int f, int c, bool tieneZapatillas);

  /**
   * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
   * REGLAS: Desnivel máximo 1 sin zapatillas, 2 con zapatillas.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return true si el desnivel con la casilla de delante es admisible.
   */
  bool EsAccesiblePorAltura(const ubicacion &actual, bool zap);
  // auxiliar
  bool EsAccesiblePorAltura(const ubicacion &actual, const ubicacion &destino, bool zap);


  /**
   * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return Estado con la fila y columna de la casilla de enfrente.
   */
  ubicacion Delante(const ubicacion &actual) const;

  bool es_camino(unsigned char c) const;
  bool es_caminoNivel_1(unsigned char c) const;

  /**
 * @brief Imprime por consola la secuencia de acciones de un plan para un agente.
 * @param plan  Lista de acciones del plan.
 */
  void PintaPlan(const list<Action> &plan);


  void AnularMatriz(vector<vector<unsigned char>> &m);

list<Action> CaminoDijkstra(const EstadoI& inicio, const EstadoI &final,const vector<vector<unsigned char>>& terreno, const vector<vector<unsigned char>>& altura);
  /**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa gráfico.
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
  void VisualizaPlan(const ubicacion &st, const list<Action> &plan);

  EstadoI NextCasillaIngeniero(const EstadoI &st);

  bool CasillaAccesibleIngeniero(const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);

  bool CasillaAccesibleSalto(const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);

  EstadoI applyT(Action accion, const EstadoI & st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);

bool Find (const NodoI & st, const list<NodoI> &lista);

/**
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 * @param plan  Lista de pasos (fila, columna, operación).
 */
  void PintaPlan(const list<Paso> &plan);

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
//list<Action> B_Anchura(const EstadoI &inicio, const EstadoI &final, const vector<vector<unsigned char>> &terreno, vector<vector<unsigned char>> &altura);

list<Action> B_Anchura_V2(const EstadoI &inicio, const EstadoI &final, const vector<vector<unsigned char>> &terreno, vector<vector<unsigned char>> &altura);


  /**
 * @brief Convierte un plan de tubería en la lista de casillas usada
 *        por el sistema de visualización.
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tubería.
 */
  void VisualizaRedTuberias(const list<Paso> &plan);

  /**
 * @brief Determina si casilla viable por altura
 * @param casilla    tipo de terreno
 * @param dif  diferencia de altura entre casillas
 * @param zap indica si tenemos o no las zapatillas
 * @return 'P' si no es accesible por altura y casilla en otro caso
 */
  char ViablePorAltura (char casilla, int dif, bool zap);

  /**
 * @brief Determina la mejor opcion entre las 3 casillas que tiene delante
 * @param i    terreno que hay en la pos 1 (45izq)
 * @param c  terreno que hay en la pos 2 (delante)
 * @param d terreno que hay en la pos 3 (45derecha)
 * @param zap indica si tenemos o no las zapatillas
 * @param actual ubicacion actual del agente
 * @return 2 si es mejor WALK, 1 TURN_SL, 3 TURN_SR. 0 si nada interesante
 */
  int VeoCasillaInteresante(char i, char c, char d, bool zap, ubicacion actual);

  int VeoCasillaInteresanteNivel1(char i, char c, char d, bool zap, ubicacion actual);


private:
  // =========================================================================
  // VARIABLES DE ESTADO (PUEDEN SER EXTENDIDAS POR EL ALUMNO)
  // =========================================================================

  bool tengo_zapatillas; 
  Action last_action;
  int giros_consecutivos; // Para evitar bucles girando
  
  int turnos_aburrido;
  int giros_pendientes;

  // Matriz para guardar las veces que hemos visitado cada casilla
  std::vector<std::vector<int>> mapaVisitados;

  // Nivel 2
  bool hayPlan;
  list<Action> plan;
};

#endif
