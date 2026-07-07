/**
 * Adafruit GFX Font Metrics
 * Métricas exactas de las fuentes de Adafruit para replicar pixel-perfect
 * el comportamiento de la OLED real en el simulador web.
 * 
 * Basado en:
 * - Adafruit_GFX.cpp (font rendering)
 * - FreeSans font metrics
 * 
 * Cada tamaño es: { width: pixels_por_caracter, height: pixels, baseline: offset_y }
 */

export const fontMetrics = {
  // Tamaño 1: 5 píxeles de ancho, 8 píxeles de alto (font: 5x8)
  1: {
    width: 6,      // Ancho promedio con espacio
    height: 8,
    charWidth: 5,
    baseline: 6,
    lineHeight: 10 // con espaciado
  },
  // Tamaño 2: 10 píxeles de ancho, 16 píxeles de alto (2x scale)
  2: {
    width: 12,
    height: 16,
    charWidth: 10,
    baseline: 12,
    lineHeight: 19
  },
  // Tamaño 3: 15 píxeles de ancho, 24 píxeles de alto (3x scale)
  3: {
    width: 18,
    height: 24,
    charWidth: 15,
    baseline: 18,
    lineHeight: 28
  },
  // Tamaño 4: 20 píxeles de ancho, 32 píxeles de alto (4x scale)
  4: {
    width: 24,
    height: 32,
    charWidth: 20,
    baseline: 24,
    lineHeight: 37
  }
};

/**
 * Calcula el ancho en píxeles de un texto en un tamaño dado
 * Usa métricas de Adafruit GFX para precisión
 */
export function calcularAnchoTexto(texto, tamano) {
  const metrics = fontMetrics[tamano];
  if (!metrics) return 0;
  
  let ancho = 0;
  for (let i = 0; i < texto.length; i++) {
    const char = texto[i];
    // Caracteres especiales o espacios: ancho reducido
    if (char === ' ') {
      ancho += metrics.width * 0.5; // espacios más estrechos
    } else if (char === '\t') {
      ancho += metrics.width * 4; // tab = 4 espacios
    } else {
      // Mayoría de caracteres: ancho estándar
      // Se puede refinar con tabla de kerning si es necesario
      ancho += metrics.width;
    }
  }
  return ancho;
}

/**
 * Calcula la altura en píxeles de un bloque de texto
 */
export function calcularAltoTexto(lineas, tamano) {
  const metrics = fontMetrics[tamano];
  if (!metrics || lineas.length === 0) return 0;
  
  return metrics.lineHeight * lineas.length;
}

/**
 * Divide un texto en líneas para que se ajuste al ancho de la OLED
 * Usa word-wrap: corta por palabras, no por caracteres
 */
export function dividirEnLineas(texto, tamano, anchoMax = 128) {
  const palabras = texto.length ? texto.split(' ') : [''];
  const lineas = [];
  let actual = '';

  for (const palabra of palabras) {
    const candidata = actual.length ? actual + ' ' + palabra : palabra;
    const ancho = calcularAnchoTexto(candidata, tamano);

    if (ancho <= anchoMax) {
      actual = candidata;
    } else {
      if (actual.length > 0) lineas.push(actual);
      actual = palabra;
    }
  }

  if (actual.length > 0) lineas.push(actual);
  if (lineas.length === 0) lineas.push('');
  return lineas;
}

/**
 * Encuentra el tamaño más grande que permite ajustar el texto en el ancho
 * Usado para modo 'reducir'
 */
export function calcularTamanoReducido(texto, tamanoMax, anchoMax = 128) {
  for (let t = tamanoMax; t >= 1; t--) {
    if (calcularAnchoTexto(texto, t) <= anchoMax) {
      return t;
    }
  }
  return 1;
}

/**
 * Recorta un texto agregando "..." si excede el ancho
 * Usado para modo 'recortar'
 */
export function recortarTexto(texto, tamano, anchoMax = 128) {
  if (calcularAnchoTexto(texto, tamano) <= anchoMax) {
    return texto;
  }

  const sufijo = '...';
  const anchoSufijo = calcularAnchoTexto(sufijo, tamano);
  
  for (let len = texto.length - 1; len > 0; len--) {
    const candidata = texto.slice(0, len) + sufijo;
    if (calcularAnchoTexto(candidata, tamano) <= anchoMax) {
      return candidata;
    }
  }
  
  return sufijo;
}

/**
 * Calcula la posición X para alinear un texto
 */
export function calcularX(texto, tamano, alineacion, anchoMax = 128) {
  const w = calcularAnchoTexto(texto, tamano);
  let x = 0;
  
  if (alineacion === 'centro') {
    x = (anchoMax - w) / 2;
  } else if (alineacion === 'derecha') {
    x = anchoMax - w;
  }
  // 'izquierda': x = 0 (por defecto)
  
  return Math.max(0, Math.round(x));
}