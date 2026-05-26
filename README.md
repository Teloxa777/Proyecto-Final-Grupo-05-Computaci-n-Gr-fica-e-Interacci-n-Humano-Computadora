# Tank City 3D

Proyecto final de la asignatura **Computación Gráfica e Interacción Humano Computadora**.

**Tank City 3D** es una reinterpretación tridimensional del videojuego clásico **Tank City / Battle City**, desarrollada en C++ con OpenGL. El proyecto integra un escenario 3D tipo tile, tanque jugador, enemigos, disparos, colisiones, ítems coleccionables, cámaras alternables, iluminación dinámica, sombras, HUD, animaciones y efectos visuales.

---

## Descripción del proyecto

El jugador controla un tanque dentro de un escenario urbano construido mediante una cuadrícula de tiles. El entorno incluye muros de ladrillo, bloques metálicos, zonas de agua, hojas, adoquines, enemigos, proyectiles, ítems coleccionables y una base principal representada por un águila.

El objetivo principal es destruir a los enemigos, proteger la base y aprovechar los ítems disponibles para mejorar las posibilidades de supervivencia.

---

## Características principales

- Escenario 3D basado en mapa tipo tile.
- Tanque controlado por el jugador.
- Enemigos con comportamiento básico.
- Sistema de disparos del jugador y enemigos.
- Colisiones con obstáculos, enemigos y proyectiles.
- Ítems coleccionables:
  - Vida / Tank
  - Estrella
  - Granada
  - Casco
  - Reloj
  - Pala
- HUD en pantalla con contador de ítems, vidas y cronómetros.
- Cámara libre.
- Cámara siguiendo al tanque.
- Cámara en primera persona.
- Iluminación dinámica mediante sol.
- Sombras dinámicas.
- Agua animada mediante shader.
- Indicadores animados para localizar enemigos.
- Pantallas de inicio, victoria, derrota y pantalla secreta.

---

## Configuración recomendada

El proyecto fue configurado y probado en Visual Studio con la siguiente configuración:

**Debug / x86**

Se recomienda ejecutar el proyecto con esta configuración para asegurar la correcta carga de librerías, modelos, shaders y archivos DLL necesarios.

La configuración **Release** puede requerir ajustes adicionales en rutas de inclusión, bibliotecas y dependencias, por lo que no se utiliza como configuración principal de entrega.

---

## Requisitos

Para ejecutar correctamente el proyecto se necesita:

- Windows.
- Visual Studio.
- Tarjeta gráfica compatible con OpenGL 3.3 o superior.
- Configuración recomendada: **Debug / x86**.
- Librerías utilizadas:
  - GLEW
  - GLFW
  - GLM
  - GDI+
- Archivos del proyecto:
  - Carpeta `Models`
  - Carpeta `Shader`
  - Archivos `.cpp` y `.h`
  - Modelos `.obj`
  - Texturas
  - Archivos `.dll` necesarios

---

## Cómo descargar el proyecto

Clonar el repositorio desde GitHub:

```bash
git clone https://github.com/Teloxa777/Proyecto-Final-Grupo-05-Computaci-n-Gr-fica-e-Interacci-n-Humano-Computadora.git
