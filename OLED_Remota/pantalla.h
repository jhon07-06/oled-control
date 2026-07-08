#ifndef PANTALLA_H
#define PANTALLA_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "modelo.h"

#define ANCHO 128
#define ALTO 64

extern Adafruit_SSD1306 display;

void iniciarPantalla();

void mostrarPantalla(const PantallaConfig &datos);

// V1.3: se debe llamar en cada vuelta de loop() para animar el
// modoTexto="scroll". No hace nada si el modo actual no es scroll.
void actualizarPantalla();

#endif