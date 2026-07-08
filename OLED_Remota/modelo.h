#ifndef MODELO_H
#define MODELO_H

#include <Arduino.h>

struct PantallaConfig {

    int version = -1;

    String texto = "";

    int tamano = 1;

    String alineacion = "izquierda";

    bool invertido = false;

    String modoTexto = "ajustar";

    // V1.6: soporte para imágenes monocromáticas.
    // tipo = "texto" (comportamiento actual) o "imagen".
    String tipo = "texto";

    // Bitmap 1bpp empaquetado (mismo formato que Adafruit_GFX::drawBitmap),
    // codificado en Base64 dentro del JSON.
    String imagenData = "";

    int imagenAncho = 0;
    int imagenAlto = 0;

};

#endif