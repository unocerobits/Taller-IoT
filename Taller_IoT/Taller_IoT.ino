/*
    Medición de Ambiental Web
    Taller IoT Demo v0.1

    PASOS:
     1. Configurar WiFi
     2. Visualizar IP de acceso a traves del terminal
     3. Acceder mediante la IP
     4. Acceso a las variables mediante http://IP/temp o http://IP/humi

    Basado en DHTServer de Adafruit, ESP8266Webserver, DHTexample y BlinkWithoutDelay

    UNOCEROBITS 2018

*/


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#define DHTTYPE DHT11
#define DHTPIN  14

const char* ssid     = "Taller_IoT";
const char* password = "taller_iot";

ESP8266WebServer server(80);

// Inicializa el sensor DHT
// NOTA: Para trabajar con un chip Arduino de 16 MHz más rápido que ATmega328p, como un ESP8266,
// necesita aumentar el umbral para recuentos de ciclos considerados como 1 o 0.
// Puedes hacer esto pasando un tercer parámetro para este umbral. Es un poco
// complejo encontrar el valor correcto, pero en general, cuanto más rápida sea la CPU,
// más alto el valor. El valor predeterminado para un AVR de 16 mhz es un valor de 6. Para un
// Arduino Due, que funciona a 84mhz, funciona con un valor de 30.
// Esto es para el procesador ESP8266 en ESP-01
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266

float humidity, temp_c, temp_f, heat_i;  // Valores leídos del sensor
float temp_t = 17; // inicializacion de la primera temperatura (estimada) para la tendencia
String webString = "";    // Variable tipo texto para la pantalla
String tendencia = "";    // Variable para mostrar la tendencia termica
//Generalmente, se debe usar "unsigned long" para las variables de tiempo
unsigned long previousMillis = 0;        // Variable que guardará el ultimo tiempo
const long interval = 2000;              // Tiempo entre lecturas del sensor

void handle_root() {
  webString = "DEMO PLATAFORMA AMBIENTAL\n------------------------------------------------\n\n";
  webString += "Bienvenido a la demostracion basica IoT \n";
  webString += "Para acceder a las variables ambientales utilice los siguientes\n";
  webString += "comandos en la barra de navegacion, justo despues de la IP:\n\n";
  webString += "\t /temp --> Para ver la temperatura\n";
  webString += "\t /humi --> Para ver la humedad\n\n";
  server.send(200, "text/plain", webString);
  delay(100);
}

void setup(void)
{

  // Puede abrir la ventana Serial Monitor para ver qué está haciendo el código
  Serial.begin(115200);  // Conexión en serie de ESP-01 a través del cable de la consola de 3.3v
  dht.begin();           // Inicializar el sensro de temperatura

  // Conectando a la WiFi
  WiFi.mode(WIFI_STA);    // STA = Station, AP = Punto de Acceso
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rConectando...");

  // Esperando conexion
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Servicio de medición ambiental web");
  Serial.print("Conectado a ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_root);

  server.on("/temp", []() { // si llamada al servidor web para la Temperatura
    gettemperature();       // leyendo el sensor
    webString = "TEMPERATURA\n------------------------------------------------\n";
    webString += "La temperatura actual es: " + String((int)temp_c) + " *C\n"; // conversion a enteros para evitar trabajar con flotantes
    webString += "La temperatura anterior es: " + String((int)temp_t) + " *C\n";
    webString += "En grados Fahrenheit es: " + String((int)temp_f) + " *F\n\n";
    webString += "El indice termico es: " + String(heat_i) + "\n\n";
    webString += "La tendencia termica: " + tendencia;
    server.send(200, "text/plain", webString);            // envio al navegador del solicitante
    temp_t = temp_c;  // Refresco de la variable de temperatura temporal.
  });

  server.on("/humi", []() { // subdirectorio para Humedad
    gettemperature();           // leyendo el sensor
    webString = "HUMEDAD\n------------------------------------------------\n";
    webString += "La humedad en la zona: " + String((int)humidity) + "%";
    server.send(200, "text/plain", webString);               // envio al navegador del solicitante
  });

  server.begin();
  Serial.println("Servidor HTTP iniciado");
  
}

void loop(void)
{
  server.handleClient();
}

void gettemperature() {
  // Espere al menos 2 segundos  entre mediciones.
  // si la diferencia entre la hora actual y la última vez que se leyó el sensor
  // es más grande que el intervalo establecido, lee el sensor
  // Funciona mejor que aplicar Delay()
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // guarda la última vez que se leyó el sensor
    previousMillis = currentMillis;

    // ¡La temperatura de lectura para la humedad tarda alrededor de 250 milisegundos!
    // Las lecturas del sensor también pueden tener hasta 2 segundos de antigüedad (es un sensor muy lento)
    humidity = dht.readHumidity();      // Lectura de Humedad (porciento)
    temp_c = dht.readTemperature();     // Lectura de Temperatura en Celsius si (true) en Fahrenheit
    temp_f = dht.convertCtoF(temp_c); // Calculo de la conversion a Fahrenheit
    Serial.println("Temp. actual: " + String(temp_c));
    Serial.println("Temp. anterior: " + String(temp_t));
    if (temp_c < temp_t)
      tendencia = "descendente";
    else if ((int)temp_c == temp_t)
      tendencia = "constante";
    else
      tendencia = "ascendente";
    heat_i = dht.computeHeatIndex(temp_c, humidity, false); // Calulo del indice termico de confort
    // Verifica si alguna lectura falló y saliera antes (para intentar de nuevo).
    if (isnan(humidity) || isnan(temp_c)) {
      Serial.println("Lectura del sensor DHT errónea");
      return;
    }
  }
}
