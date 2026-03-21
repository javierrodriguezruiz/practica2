#!/bin/bash

# ==========================================
# Script de Pruebas Masivas - Belkan
# ==========================================

# Variables configurables
CARPETA_MAPAS="mapas"
NIVEL=0
SEMILLA=42
TIEMPO_ESPERA=5 # Segundos a esperar entre cada mapa

echo "Iniciando batería de pruebas para el Nivel $NIVEL..."
echo "Buscando mapas en la carpeta: $CARPETA_MAPAS/"
echo "---------------------------------------------------"

# Bucle que recorre todos los archivos .map en la carpeta
for mapa in "$CARPETA_MAPAS"/*.map; do
    # Comprobar que el archivo realmente existe
    if [ -f "$mapa" ]; then
        # Extraer solo el nombre del archivo para que se lea mejor en consola
        nombre_mapa=$(basename "$mapa")
        
        echo ">> TESTEANDO MAPA: $nombre_mapa"
        
        # Ejecutar el simulador sin interfaz gráfica
        # Puedes añadir más parámetros aquí si el profesor lo exige (ej: -Tiempo 3000)
        ./practica2SG -m "$mapa" -seed "$SEMILLA" -n "$NIVEL"
        
        # Guardar el resultado de la ejecución (0 si fue éxito, otro número si falló)
        RESULTADO=$?
        
        if [ $RESULTADO -eq 0 ]; then
            echo "[OK] Ejecución finalizada correctamente."
        else
            echo "[AVISO] La ejecución devolvió un error (Normal si el mapa no es apto para Nivel 0)."
        fi
        
        echo "---------------------------------------------------"
        
        # Esperar X segundos antes de lanzar el siguiente mapa
        sleep "$TIEMPO_ESPERA"
    fi
done

echo "¡Batería de pruebas completada!"
