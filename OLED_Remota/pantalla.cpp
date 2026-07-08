#include "pantalla.h"

#include <vector>

Adafruit_SSD1306 display(ANCHO, ALTO, &Wire, -1);

// ---------------------------------------------------------------
// V1.2 - modoTexto = "ajustar"
// Divide el texto en varias líneas para que quepa en el ancho
// de la OLED, partiendo por palabras (word-wrap).
// ---------------------------------------------------------------
static std::vector<String> dividirEnLineas(const String &texto)
{
    std::vector<String> lineas;
    String lineaActual = "";

    int inicio = 0;
    int n = texto.length();

    while (inicio < n)
    {
        int espacio = texto.indexOf(' ', inicio);
        String palabra;

        if (espacio == -1)
        {
            palabra = texto.substring(inicio);
            inicio = n;
        }
        else
        {
            palabra = texto.substring(inicio, espacio);
            inicio = espacio + 1;
        }

        String candidata = (lineaActual.length() == 0)
                                ? palabra
                                : lineaActual + " " + palabra;

        int16_t x1, y1;
        uint16_t w, h;

        display.getTextBounds(candidata, 0, 0, &x1, &y1, &w, &h);

        if (w <= ANCHO || lineaActual.length() == 0)
        {
            // La línea candidata cabe, o es una sola palabra que ya
            // es más ancha que la pantalla (se deja igual, sin cortar).
            lineaActual = candidata;
        }
        else
        {
            lineas.push_back(lineaActual);
            lineaActual = palabra;
        }
    }

    if (lineaActual.length() > 0)
        lineas.push_back(lineaActual);

    if (lineas.empty())
        lineas.push_back("");

    return lineas;
}

static int calcularX(const String &linea, const String &alineacion)
{
    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(linea, 0, 0, &x1, &y1, &w, &h);

    int x = 0;

    if (alineacion == "centro")
    {
        x = (ANCHO - w) / 2;
    }
    else if (alineacion == "derecha")
    {
        x = ANCHO - w;
    }
    else
    {
        x = 0;
    }

    if (x < 0)
        x = 0;

    return x;
}

// ---------------------------------------------------------------
// V1.4 - modoTexto = "reducir"
// Reduce el tamaño de letra (desde datos.tamano hacia abajo, hasta 1)
// hasta que el texto completo quepa en el ancho de la OLED.
// Una sola línea.
// ---------------------------------------------------------------
static void dibujarReducir(const PantallaConfig &datos)
{
    int tam = datos.tamano;
    if (tam < 1)
        tam = 1;

    int16_t x1, y1;
    uint16_t w, h;

    display.setTextSize(tam);
    display.getTextBounds(datos.texto, 0, 0, &x1, &y1, &w, &h);

    while (w > ANCHO && tam > 1)
    {
        tam--;
        display.setTextSize(tam);
        display.getTextBounds(datos.texto, 0, 0, &x1, &y1, &w, &h);
    }

    int x = calcularX(datos.texto, datos.alineacion);
    int y = (ALTO - h) / 2;

    if (y < 0)
        y = 0;

    display.setCursor(x, y);
    display.print(datos.texto);
}

// ---------------------------------------------------------------
// V1.5 - modoTexto = "recortar"
// Si el texto no cabe en el ancho de la OLED, lo corta y agrega "..."
// al final. Mantiene el tamano indicado en datos.tamano (a diferencia
// de "reducir", que en vez de cortar, achica la letra).
// ---------------------------------------------------------------
static String recortarConElipsis(const String &texto)
{
    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(texto, 0, 0, &x1, &y1, &w, &h);

    if (w <= ANCHO)
        return texto;

    String recortado = texto;

    while (recortado.length() > 0)
    {
        recortado.remove(recortado.length() - 1);

        String candidata = recortado + "...";

        display.getTextBounds(candidata, 0, 0, &x1, &y1, &w, &h);

        if (w <= ANCHO)
            return candidata;
    }

    return "...";
}

static void dibujarRecortar(const PantallaConfig &datos)
{
    String textoFinal = recortarConElipsis(datos.texto);

    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(textoFinal, 0, 0, &x1, &y1, &w, &h);

    int x = calcularX(textoFinal, datos.alineacion);
    int y = (ALTO - h) / 2;

    if (y < 0)
        y = 0;

    display.setCursor(x, y);
    display.print(textoFinal);
}

// ---------------------------------------------------------------
// V1.6 - tipo = "imagen"
// Decodificador Base64 propio (sin librerías externas) y dibujo de
// un bitmap monocromático (1 bpp) mediante Adafruit_GFX::drawBitmap.
//
// Formato esperado del bitmap ya decodificado: empaquetado por filas,
// MSB primero, cada fila redondeada al byte superior (igual que los
// arreglos que genera el "Image2cpp" para pantallas monocromáticas).
// ---------------------------------------------------------------
static int valorCaracterBase64(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1; // '=' de relleno u otro carácter no válido (se ignora)
}

// Decodifica una cadena Base64 estándar hacia un buffer ya reservado.
// Devuelve la cantidad de bytes efectivamente escritos en "salida".
static size_t decodificarBase64(const String &entrada, uint8_t *salida, size_t capacidadSalida)
{
    size_t o = 0;
    uint32_t buffer = 0;
    int bits = 0;

    for (size_t i = 0; i < entrada.length(); i++)
    {
        char c = entrada.charAt(i);

        int valor = valorCaracterBase64(c);
        if (valor < 0)
            continue; // ignora '=', saltos de línea, espacios, etc.

        buffer = (buffer << 6) | (uint32_t)valor;
        bits += 6;

        if (bits >= 8)
        {
            bits -= 8;

            if (o >= capacidadSalida)
                return o; // buffer lleno, se corta aquí

            salida[o++] = (uint8_t)((buffer >> bits) & 0xFF);
        }
    }

    return o;
}

static void dibujarImagen(const PantallaConfig &datos)
{
    if (datos.imagenAncho <= 0 || datos.imagenAlto <= 0 || datos.imagenData.length() == 0)
        return; // no hay imagen válida que dibujar

    int bytesPorFila = (datos.imagenAncho + 7) / 8;
    size_t tamanoEsperado = (size_t)bytesPorFila * (size_t)datos.imagenAlto;

    uint8_t *bitmap = (uint8_t *)malloc(tamanoEsperado);

    if (!bitmap)
        return; // memoria insuficiente, no se dibuja nada

    size_t decodificados = decodificarBase64(datos.imagenData, bitmap, tamanoEsperado);

    if (decodificados < tamanoEsperado)
    {
        // Datos incompletos o corruptos: se evita dibujar basura en pantalla.
        free(bitmap);
        return;
    }

    int x = (ANCHO - datos.imagenAncho) / 2;
    int y = (ALTO - datos.imagenAlto) / 2;

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    display.drawBitmap(x, y, bitmap, datos.imagenAncho, datos.imagenAlto, SSD1306_WHITE);

    free(bitmap);
}

// ---------------------------------------------------------------
// V1.3 - modoTexto = "scroll"
// Texto deslizante horizontal (marquee), de derecha a izquierda,
// en bucle. Requiere estado propio porque se redibuja en cada
// llamada a actualizarPantalla() (ver .ino: se llama en cada loop()),
// no solo cuando cambia la versión del JSON.
// ---------------------------------------------------------------
static PantallaConfig configActual;

static int scrollX = 0;
static uint16_t anchoTextoScroll = 0;
static unsigned long ultimoScroll = 0;

static const unsigned long SCROLL_INTERVALO_MS = 30; // cada cuánto se mueve
static const int SCROLL_PASO_PX = 2;                 // píxeles por paso

static void dibujarScroll()
{
    display.clearDisplay();
    display.invertDisplay(configActual.invertido);
    display.setTextSize(configActual.tamano);

    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(configActual.texto, 0, 0, &x1, &y1, &w, &h);

    int y = (ALTO - h) / 2;
    if (y < 0)
        y = 0;

    display.setCursor(scrollX, y);
    display.print(configActual.texto);

    display.display();
}

static void prepararScroll(const PantallaConfig &datos)
{
    int16_t x1, y1;
    uint16_t w, h;

    display.setTextSize(datos.tamano);
    display.getTextBounds(datos.texto, 0, 0, &x1, &y1, &w, &h);

    anchoTextoScroll = w;
    scrollX = ANCHO; // arranca fuera de la pantalla, entrando por la derecha
    ultimoScroll = millis();
}

// Se llama en cada vuelta de loop() (ver .ino). No hace nada si el
// modoTexto actual no es "scroll", para no interferir con los demás modos.
void actualizarPantalla()
{
    if (configActual.tipo == "imagen")
        return; // las imágenes no se animan, solo se redibujan al cambiar versión

    if (configActual.modoTexto != "scroll")
        return;

    if (millis() - ultimoScroll < SCROLL_INTERVALO_MS)
        return;

    ultimoScroll = millis();

    scrollX -= SCROLL_PASO_PX;

    // Cuando el texto sale completamente por la izquierda, reinicia
    // el recorrido desde el borde derecho.
    if (scrollX < -(int)anchoTextoScroll)
        scrollX = ANCHO;

    dibujarScroll();
}

void iniciarPantalla()
{
    Wire.begin(14, 12);
    delay(500);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);

    display.setTextWrap(false); // <-- agregar esta línea
}

void mostrarPantalla(const PantallaConfig &datos)
{
    // Se guarda siempre, porque actualizarPantalla() (scroll) la necesita
    // fuera de esta función, en cada vuelta de loop().
    configActual = datos;

    // V1.6: tipo = "imagen". No entra a ninguna ruta de texto (ajustar,
    // scroll, reducir, recortar); dibuja el bitmap y termina.
    if (datos.tipo == "imagen")
    {
        display.clearDisplay();
        display.invertDisplay(datos.invertido);

        dibujarImagen(datos);

        display.display();
        return;
    }

    if (datos.modoTexto == "scroll")
    {
        prepararScroll(datos);
        dibujarScroll(); // primer frame; el resto lo anima actualizarPantalla()
        return;
    }

    display.clearDisplay();

    display.invertDisplay(datos.invertido);

    display.setTextSize(datos.tamano);

    if (datos.modoTexto == "ajustar")
    {
        std::vector<String> lineas = dividirEnLineas(datos.texto);

        int16_t x1, y1;
        uint16_t w, h;

        // Altura de referencia de una línea (usa una muestra con
        // ascendentes/descendentes para una medida estable).
        display.getTextBounds("Ag", 0, 0, &x1, &y1, &w, &h);

        int alturaLinea = h + 2; // pequeño espacio entre líneas
        int alturaBloque = alturaLinea * (int)lineas.size();

        int y = (ALTO - alturaBloque) / 2;

        if (y < 0)
            y = 0;

        for (size_t i = 0; i < lineas.size(); i++)
        {
            int x = calcularX(lineas[i], datos.alineacion);

            display.setCursor(x, y + (int)(i * alturaLinea));

            display.print(lineas[i]);
        }
    }
    else if (datos.modoTexto == "reducir")
    {
        dibujarReducir(datos);
    }
    else if (datos.modoTexto == "recortar")
    {
        dibujarRecortar(datos);
    }
    else
    {
        // Comportamiento de una sola línea (fallback para modoTexto
        // desconocido o no reconocido).
        int16_t x1, y1;
        uint16_t w, h;

        display.getTextBounds(
            datos.texto,
            0,
            0,
            &x1,
            &y1,
            &w,
            &h);

        int x = calcularX(datos.texto, datos.alineacion);

        int y = (ALTO - h) / 2;

        if (y < 0)
            y = 0;

        display.setCursor(x, y);

        display.print(datos.texto);
    }

    display.display();
}