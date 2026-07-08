#include <ESP8266WiFi.h>

#include "config.h"
#include "modelo.h"
#include "github.h"
#include "pantalla.h"

PantallaConfig pantallaActual;

unsigned long ultimoChequeo = 0;

// V1.1: guarda la versión que ya está dibujada en la OLED,
// para no volver a renderizar si el JSON remoto no cambió.
int versionMostrada = -1;

void setup() {

  Serial.begin(115200);

  iniciarPantalla();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Conectando WiFi...");
  display.display();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi OK");
  display.display();

}

void loop() {

  // V1.3: anima el modoTexto="scroll" en cada vuelta del loop.
  // No interfiere con los demás modos ni con el chequeo periódico de abajo.
  actualizarPantalla();

  if (millis() - ultimoChequeo >= TIEMPO_ACTUALIZACION) {

    ultimoChequeo = millis();

    PantallaConfig nuevaConfig;

    if (obtenerPantalla(nuevaConfig)) {

      // Solo redibuja si la versión cambió respecto a la última mostrada.
      // Esto evita parpadeos innecesarios cuando el JSON remoto sigue igual.
      if (nuevaConfig.version != versionMostrada) {

        pantallaActual = nuevaConfig;

        mostrarPantalla(pantallaActual);

        versionMostrada = nuevaConfig.version;

      }

    }

  }

}