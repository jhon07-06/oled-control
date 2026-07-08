#include "github.h"
#include "config.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

bool obtenerPantalla(PantallaConfig &datos)
{

    if (WiFi.status() != WL_CONNECTED)
        return false;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    if (!http.begin(client, url))
        return false;

    int codigo = http.GET();

    if (codigo != HTTP_CODE_OK)
    {
        http.end();
        return false;
    }

    String respuesta = http.getString();

    http.end();

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, respuesta);

    if (error)
        return false;

    JsonObject pantalla = doc["pantalla"];
    JsonObject config = pantalla["config"];

    datos.version = doc["version"] | 0;

    datos.texto = (const char*)pantalla["texto"];

    datos.tamano = config["tamano"] | 1;

    datos.alineacion = (const char*)config["alineacion"];

    datos.invertido = config["invertido"] | false;

    datos.modoTexto = (const char*)config["modoTexto"];

    // V1.6: tipo = "texto" (por defecto, compatible con JSON anteriores) o "imagen".
    datos.tipo = (const char*)(pantalla["tipo"] | "texto");

    if (datos.tipo == "imagen")
    {
        JsonObject imagen = pantalla["imagen"];

        datos.imagenAncho = imagen["ancho"] | 0;
        datos.imagenAlto = imagen["alto"] | 0;
        datos.imagenData = (const char*)(imagen["datos"] | "");
    }
    else
    {
        datos.imagenAncho = 0;
        datos.imagenAlto = 0;
        datos.imagenData = "";
    }

    return true;
}