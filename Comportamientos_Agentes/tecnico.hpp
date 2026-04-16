#ifndef COMPORTAMIENTOTECNICO_H
#define COMPORTAMIENTOTECNICO_H

#include <chrono>
#include <time.h>
#include <thread>
#include <list>

#include "comportamientos/comportamiento.hpp"

// =========================================================================
// DOCUMENTACIÓN PARA ESTUDIANTES
// =========================================================================
/*
 * CLASE: ComportamientoTecnico
 * 
 * DESCRIPCIÓN:
 * Esta clase implementa el comportamiento del agente Técnico en el mundo Belkan.
 * El técnico colabora con el ingeniero para resolver el problema de instalación de tuberías
 */
struct EstadoT{
  ubicacion site;
  bool zapatillas;

  bool operator == (const EstadoT &st) const{
    return (site==st.site && zapatillas==st.zapatillas);
  }
};

struct NodoT{
  EstadoT estado;
  list<Action> secuencia;

  bool operator==(const NodoT &node)const{
    return estado==node.estado;
  }
};


class ComportamientoTecnico : public Comportamiento {
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================
  
  /**
   * @brief Constructor para niveles 0, 1 y 6 (sin mapa completo)
   * @param size Tamaño del mapa (si es 0, se inicializa más tarde)
   */
  ComportamientoTecnico(unsigned int size = 0) : Comportamiento(size) {
    // Inicializar Variables de Estado
    tengo_zapatillas = false;
    last_action = IDLE;
    giros_consecutivos = 0;
    girar_derecha = true;
  }

  /**
   * @brief Constructor para niveles 2, 3, 4 y 5 (con mapa completo conocido)
   * @param mapaR Mapa de terreno conocido
   * @param mapaC Mapa de cotas conocido
   */
  ComportamientoTecnico(std::vector<std::vector<unsigned char>> mapaR, 
                       std::vector<std::vector<unsigned char>> mapaC): 
                       Comportamiento(mapaR, mapaC) {

    hayPlan = false;
    tengo_zapatillas=false;
  }

  ComportamientoTecnico(const ComportamientoTecnico &comport): Comportamiento(comport) {}
  ~ComportamientoTecnico() {}

  /**
   * @brief Bucle principal de decisión del técnico.
   * Estudia los sensores y decide la siguiente acción.
   * 
   * EJEMPLO DE USO:
   * Action accion = think(sensores);
   * return accion; // El motor ejecutará esta acción
   */
  Action think(Sensores sensores);

  ComportamientoTecnico *clone() {
    return new ComportamientoTecnico(*this);
  }

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================
  
/**
 * @brief Comportamiento del técnico para el Nivel 0.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_0(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_1(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 2.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_2(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_3(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_4(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_5(Sensores sensores);
  
/**
 * @brief Comportamiento del técnico para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_6(Sensores sensores);

/**
 * @brief Comportamiento del técnico para el Nivel E.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
  Action ComportamientoTecnicoNivel_E(Sensores sensores);

bool CasillaAccesibleTecnico(const EstadoT &st, const vector<vector<unsigned char>> &terreno, const
  vector<vector<unsigned char>> &altura);
  EstadoT applyT(Action accion, const EstadoT & st, const vector<vector<unsigned char>> &terreno, const
vector<vector<unsigned char>> &altura);
bool Find (const NodoT & st, const list<NodoT> &lista);

EstadoT NextCasillaTecnico(const EstadoT &st);


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
list<Action> B_Anchura(const EstadoT &inicio, const EstadoT &final, const vector<vector<unsigned char>> &terreno, vector<vector<unsigned char>> &altura);

protected:
  // =========================================================================
  // FUNCIONES PROPORCIONADAS
  // =========================================================================

  /**
   * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
   * IMPORTANTE: Esta función ya está implementada. Actualiza mapaResultado y mapaCotas
   * con la información de los 16 sensores.
   */
  void ActualizarMapa(Sensores sensores);

  /**
   * @brief Determina si una casilla es transitable para el técnico.
   * NOTA: El técnico puede tener reglas de transitabilidad diferentes al ingeniero.
   * @param f Fila de la casilla.
   * @param c Columna de la casilla.
   * @param tieneZapatillas Indica si el agente posee las zapatillas.
   * @return true si la casilla es transitable.
   */
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);
  bool EsCasillaTransitableLevel1(int f, int c, bool tieneZapatillas);


  /**
   * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
   * REGLA PARA TÉCNICO: Desnivel máximo siempre 1 (independiente de zapatillas).
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return true si el desnivel con la casilla de delante es admisible.
   */
  bool EsAccesiblePorAltura(const ubicacion &actual);
  bool EsAccesiblePorAltura(const ubicacion &origen, const ubicacion &destino);

  /**
   * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return Estado con la fila y columna de la casilla de enfrente.
   */
  ubicacion Delante(const ubicacion &actual) const;

  /**
   * @brief Comprueba si una celda es de tipo transitable por defecto.
   * @param c Carácter que representa el tipo de superficie.
   * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
   */
  bool es_camino(unsigned char c) const;
  bool es_caminoNivel1(unsigned char c) const;


    /**
 * @brief Imprime por consola la secuencia de acciones de un plan para un agente.
 * @param plan  Lista de acciones del plan.
 */
  void PintaPlan(const list<Action> &plan);


/**
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 * @param plan  Lista de pasos (fila, columna, operación).
 */
  void PintaPlan(const list<Paso> &plan);


  /**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa gráfico.
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
  void VisualizaPlan(const ubicacion &st, const list<Action> &plan);

  /**
 * @brief Determina si casilla viable por altura
 * @param casilla    tipo de terreno
 * @param dif  diferencia de altura entre casillas
 * @return 'P' si no es accesible por altura y casilla en otro caso
 */
  char ViablePorAltura (char casilla, int dif);

  /**
 * @brief Determina la mejor opcion entre las 3 casillas que tiene delante
 * @param i    terreno que hay en la pos 1 (45izq)
 * @param c  terreno que hay en la pos 2 (delante)
 * @param d terreno que hay en la pos 3 (45derecha)
 * @return 2 si es mejor WALK, 1 TURN_SL, 3 TURN_SR. 0 si nada interesante
 */
  int VeoCasillaInteresante(char i, char c, char d, ubicacion actual);

  int VeoCasillaInteresanteNivel1(char i, char c, char d, bool zap, ubicacion actual);

private:
  // =========================================================================
  // VARIABLES DE ESTADO (PUEDEN SER EXTENDIDAS POR EL ALUMNO)
  // =========================================================================

  bool tengo_zapatillas; 
  Action last_action;
  int giros_consecutivos; // Para evitar bucles girando, si == 4, vuelta completa, buscamos otra alternativa
  
  // Variable que nos indica si giramos a la derecha o no para evitar bucles infinitos. Se decidirá en el futuro de forma aleatoria con rand.
  bool girar_derecha; 
  
  // Matriz para guardar las veces que hemos visitado cada casilla
  std::vector<std::vector<int>> mapaVisitados;


  // NIVEL E
  bool hayPlan;
  list<Action> plan;
};

#endif