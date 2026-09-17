# Práctica 2 de *Inteligencia Artificial*, curso 2025/2026

**SPANISH** — **Descripción:**  
Este repositorio contiene mi solución a la práctica de agentes reactivos, deliberativos e híbridos de la asignatura de Inteligencia Artificial (UGR). El objetivo es controlar dos agentes autónomos — un **Ingeniero** y un **Técnico** — que cooperan en un entorno tipo grid (mundo "Belkan") para explorar el terreno, moverse de forma eficiente y construir una red de tuberías, bajo distintos niveles de observabilidad del mapa (desde percepción local por sensores hasta mapa completo conocido). Se han implementado Búsqueda en Anchura, Dijkstra y A* para resolver problemas de exploración, búsqueda de objetivos, planificación y coordinación entre agentes.

**ENGLISH** — **Overview:**  
This repository contains my solution for the reactive, deliberative and hybrid agents assignment of the Artificial Intelligence course (UGR). The goal is to control two autonomous agents — an **Engineer** and a **Technician** — that cooperate in a grid-based environment ("Belkan" world) to explore terrain, move efficiently, and build a pipeline network, across several levels of map observability (from local sensor-based perception to full map knowledge). I implemented Breadth-First Search, Dijkstra and A* to solve exploration, goal-search, planning and multi-agent coordination problems.

## 🧠 Mi contribución (My Work)

### SPANISH

Implementé por completo la lógica de decisión de ambos agentes, en `Comportamientos_Agentes/ingeniero.{cpp,hpp}` y `Comportamientos_Agentes/tecnico.{cpp,hpp}`. Todo lo demás (motor de juego, interfaz gráfica, estructura base) fue proporcionado por el equipo docente.

**Algoritmos de búsqueda implementados:**
* 🔎 **Búsqueda en anchura (BFS)** — cálculo de caminos más cortos sobre un espacio de estados extendido (posición + orientación + inventario), usado en los niveles con reglas de movimiento simples.
* 🧭 **Dijkstra** — caminos de coste mínimo cuando el coste de moverse entre casillas no es uniforme (terreno y desnivel).
* ⭐ **A\* con heurísticas admisibles** (distancia de Chebyshev para movimiento en 8 direcciones), en tres variantes:
  * Función de coste que modela energía real: distinto coste según tipo de terreno, penalización por subir desnivel, bonificación por bajar.
  * A* con **restricción de paso obligatorio**: la meta solo se acepta si el camino ha pasado antes por un tipo de casilla concreto (estado aumentado, no solo posición).
  * A* **multiobjetivo** aplicado al diseño de una red de tuberías, combinando longitud, coste energético e impacto ecológico tanto en la heurística como en el desempate de la cola de prioridad.
* 🤖 **Agentes híbridos reactivo-deliberativos**: en los niveles con visión parcial (sensores locales), los agentes usan reglas reactivas y detección de bucles mientras construyen su modelo interno del mapa; en cuanto disponen de mapa completo, pasan a planificar con BFS/Dijkstra/A* antes de actuar.
* 🤝 **Coordinación multiagente**: Ingeniero y Técnico son procesos de decisión independientes sincronizados mediante variables de estado compartido (turnos de espera, señales de "terreno preparado", modo construcción) — el Ingeniero planifica la red de tuberías y el Técnico ejecuta desplazamientos y construcción bajo demanda.

### ENGLISH

I fully implemented the decision-making logic of both agents, in `Comportamientos_Agentes/ingeniero.{cpp,hpp}` and `Comportamientos_Agentes/tecnico.{cpp,hpp}`. Everything else (game engine, graphical interface, base structure) was provided by the teaching team.

**Search algorithms implemented:**
* 🔎 **Breadth-First Search (BFS)** — shortest-path computation over an extended state space (position + orientation + inventory), used in the levels with simpler movement rules.
* 🧭 **Dijkstra** — minimum-cost paths when the cost of moving between cells is non-uniform (terrain type and elevation changes).
* ⭐ **A\* with admissible heuristics** (Chebyshev distance for 8-directional movement), in three variants:
  * A cost function modeling real energy consumption: different costs per terrain type, penalty for climbing, bonus for descending.
  * A* with a **mandatory waypoint constraint**: the goal is only accepted if the path has previously passed through a specific cell type (augmented state, not just position).
  * **Multi-objective** A* applied to pipeline network design, combining length, energy cost and ecological impact both in the heuristic and in the priority-queue tie-breaking.
* 🤖 **Hybrid reactive-deliberative agents**: in levels with partial vision (local sensors), agents rely on reactive rules and loop detection while building their internal map model; once the full map is known, they switch to planning with BFS/Dijkstra/A* before acting.
* 🤝 **Multi-agent coordination**: Engineer and Technician are independent decision processes synchronized through shared state variables (waiting turns, "terrain ready" signals, construction mode) — the Engineer plans the pipeline network and the Technician executes movement and construction on demand.

## 📚 Código Base (Base Code & Acknowledgments)

**SPANISH** — El código base, el motor del juego/simulador y las instrucciones originales de despliegue fueron proporcionados por el equipo docente de la Universidad de Granada (UGR).

**ENGLISH** — The base code, the game/simulator engine and the original deployment instructions were provided by the teaching team of the University of Granada (UGR).

🔗 [ugr-ccia-IA/2026_practica2](https://github.com/ugr-ccia-IA/2026_practica2)







--------------------------------------------------

## Prerrequisitos

### Crear una cuenta en [GitHub](https://github.com/). 
Para ello, puedes usar tu correo personal, el de *@correo.ugr.es* o el de *@go.ugr.es*.


### 1. Añadir tu clave SSH a GitHub
Hay varias maneras de conectarte desde tu ordenador a GitHub. Si utilizas un navegador, usarás tu usuario y contraseña. Desde el terminal, lo más cómodo es utilizar una clave SSH. Puedes crear una nueva si no tienes, o reutilizar una ya existente. Tienes toda la información para realizar la configuración en: 
[Conectar a GitHub con SSH](https://docs.github.com/es/authentication/connecting-to-github-with-ssh)


### 2. Crear tu copia personal del repositorio de la asignatura
Cada estudiante debe tener su propia copia del repositorio para poder trabajar sobre ella. En adelante, a tu copia la llamaremos *origin*, y al repositorio original de la asignatura lo llamaremos *upstream* (NOTA: Estas son convenciones que la mayoría de los desarrolladores usan, pero los puedes llamar como quieras). 

> La forma usual de crear tu copia del repositorio es realizando un *fork*. Sin embargo, dado que realizar un *fork* de un repositorio con visibilidad pública obliga al que la copia sea también pública, nosotros usaremos un procedimiento diferente que nos permite que nuestra copia del repositorio sea privada.

Para realizar la copia, una vez que tengas creada tu cuenta en GitHub, haz click en <https://github.com/new/import> y rellena tal y como se ve en la imagen de abajo. El repositorio que quieres importar es `https://github.com/ugr-ccia-IA/practica2`. ¡Asegúrate de que tu repositorio es privado!

![Importar repositorio practica2](doc/img/import_new_repo.png)


### 3. Clonar tu repositorio en tu máquina
Una vez hecho el paso anterior, tendrás tu repositorio personal de la práctica1 en GitHub; puedes descargarlo a tu ordenador usando:
`git clone git@github.com:TU_USUARIO_GITHUB/practica2.git` (si no has configurado tu clave SSH, esto no funcionará).


### 4. Modificar el código y guardar los cambios
Es el momento de empezar a modificar ficheros. Abre el fichero README.md (este fichero), ve al final y añade una línea que diga "Esto lo puse yo."
Una vez lo hayas modificado, guarda el fichero, y ejecuta los siguientes comandos en el terminal estando dentro de la carpeta `practica2`:

```
git add . 
git commit -m "Modificando README.md"
git push origin main 
```

Los tres comandos anteriores le indican a git que 1) queremos guardar una nueva versión con todos los ficheros modificados de la carpeta, 2) que haga esa versión y le ponga el comentario "Cambiando el enlace del botón", y 3) que envíe esta nueva versión a la copia de nuestro repositorio alojada en GitHub.

Este proceso es el que debes repetir cada vez que vayas avanzando en la implementación de la práctica: add, commit, push.



### 5. Enlazar tu repositorio personal con el de la asignatura
Aunque tu repositorio y el de la asignatura (recuerda que los conocemos por *origin* y *upstream* respectivamente) sean independientes, nos va a interesar que estén enlazados. De esta forma, podrás aplicar fácilmente sobre tu repositorio (*origin*) cualquier actualización que los profesores realicemos en *upstream*. Para enlazarlos, ejecuta lo siguiente dentro de la carpeta de tu repositorio:

`git remote add upstream git@github.com:ugr-ccia-IA/practica2.git`


### Actualizar tu repositorio con cambios realizados en el de la asignatura
Una vez tengas los repositiorios enlazados, lo único que debes hacer para aplicar posibles cambios en el repositorio de la asignatura en tu repositorio (cambios de *upstream* en *origin*) es: `git pull upstream main`

Hacer esto no sobreescribirá tus avances en la implementación de la práctica, puesto que tú no deberías haber modificado ninguna parte del código diferente a la que se indica en el guión.

Si quieres que esos cambios también se guarden en github, a continuación ejecuta: `git push origin main`


> Si quieres saber más sobre Git y GitHub, en Internet existen multitud de recursos, incluidos videos y tutoriales. Para realizar esta práctica sólo necesitas lo básico (hacer commits), pero hay muchas cosas más que se pueden hacer con estas herramientas (uso de ramas, gestión de conflictos, etc.) 
El propio GitHub pone a tu disposición un [breve curso](https://classroom.github.com/a/W33pQ3pa) (en inglés) para aprender lo básico.


## Realización de la práctica
El guión (disponible en [PRADO](https://pradogrado2526.ugr.es/)) contiene toda la información sobre en qué consiste la práctica2. Leelo con atención.

Junto a ellos, también tienes a tu disposición una pequeña presentación de resumen, y un tutorial. Debes revisarlos pues continen los primeros pasos a realizar.


### Instalación local (linux)

Una vez que tengas tu repositorio (el fork que has realizado) en tu ordenador, puedes compilar el código usando `./install.sh` (esto instalará todas las dependencias, y ejecutará `cmake` y `make`. ).
A continuación, puedes lanzar el software con interfaz gráfica con `./practica2`, o sin ella con `./practica2SG`.

Cuando realices cualquier modificación en el código, debes recompilar, así que usa `make clean` y `make -j$(nproc)`.




## Más información
Hemos creado un [fichero con preguntas frecuentes](./FAQ.md) que han ido apareciendo en las distintas sesiones de prácticas.

