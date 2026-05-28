#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JSON_SIZE 100 // 100 Zeichen zum testen - nicht berechnet
#define JSON_STR_TIMESTAMP "\"timestamp\":"
#define JSON_STR_STATION_ID "\"station_id\":"
#define JSON_STR_TEMPERATURE_C "\"temperature_c\""
#define JSON_STR_HUMIDITY_PCT "\"humidity_pct\""

typedef struct {
    char timestamp[20]; // 20 zeichen mit \0 
    char station_id[3];
    float temperature_c;
    float humidity_pct;
} SensorData;

// Bsp. Sensordaten
const SensorData example_data01 = {"2026-05-28 09:09:30", "S02", 24.4, 74.5}; // daten clean
const SensorData example_data02 = {"", "S02", 24.4, 74.5}; // timestamp = leerer String -> funktioniert -> aktuell ersetzen durch "empty"
const SensorData example_data03 = {"20260-05-28 09:09:30", "S02", 24.4, 74.5}; // timestamp Zeichen zu viel -> funktioniert -> invalid format
const SensorData example_data04 = {"2026-05-2809:09:31", "S02", 24.4, 74.5}; // fehlendes Leerzeichen wird nicht als Fehler erkannt             !!!
const SensorData example_data05 = {"2026-05-28 09:09:31", "S2", 24.4, 74.5}; // station_id Zeichen !=3 -> funktioniert -> invalid format
const SensorData example_data06 = {"2026-05-28 09:09:31", "S06", 24.4, 74.5}; // station_id - nicht vorhandene Station

SensorData example_data;

// String für JSON Output
char out_buffer[JSON_SIZE];

// Test-Funktion für das Validieren des timestamps -> Format und Wertebereich einzelner Komponenten werden geprüft (kein Vergleich zum aktuellen Datum!)
int valid_timestamp(const char* timestamp) {
  
  int y, m, d, H, M, S;

  if (sscanf(timestamp, "%4d-%2d-%2d %2d:%2d:%2d", &y, &m, &d, &H, &M, &S) != 6) {
    return -1;
  }
  if (m < 1 || m > 12 || d < 1 || d > 31 || H < 0 || H > 23 || M < 0 || M > 59 || S < 0 || S > 59) {
    return -1;
  }
  return 0;
}

int main() {

  // Zuweisung eines Beispiel-Datensatzes zur verwendeten struct-Variablen
  // (um nicht  mehrfach die Bezeichnung zu Ändern, wenn andere Beispieldaten verwendet werden)
  example_data = example_data06;

   // strncat für Anfügen von maximal n Zeichen zu buffer - n = definierte Länge des out-buffers (JSON_SIZE) - tatsächliche Länge (strlen(out_buffer) -1 für \0
  strncat(out_buffer, "{", JSON_SIZE - strlen(out_buffer) - 1);

  char tmp_string[JSON_SIZE] = "Fehler";

  // Sensordaten in JSON-String einfügen
  // Wenn einer der SensorData Einträge leer ist, diesen mit NULL ersetzen, sonst den Wert verwenden
  // Werte auf Typ und Format prüfen


  // timestamp

  // Prüfen, ob String leer ist
  if (strcmp(example_data.timestamp, "") == 0) { // 0 -> Strings sind gleich ()> 0 erstes ungleiches Zeichen in example ist größer / > 0 erstes ungleiches Zeichen in exampel ist kleiner)

    
    // tmp_string auf "timestamp: "empty" ändern
    //     -> würde verwerfen nicht mehr Sinn machen, weil man mit Messwerten ohne Datum nichts anfangen kann?

    // snprintf ersetzt die definierte Länge (sizeof tmp_string) des strings tmp_string mit dem neuen String
    snprintf(tmp_string,sizeof tmp_string, "%s\"empty\"\n", JSON_STR_TIMESTAMP);

    // tmp_string an den out_buffer hängen
    strncat(out_buffer, tmp_string, JSON_SIZE - strlen(out_buffer) - 1);
  
  } else if (valid_timestamp(example_data.timestamp) == -1) {
    printf("timestamp has invalid format\n");
    return -1;

  } else {

    // tmp_string auf "timestamp": "tatsächlicher Wert" setzen
    // snprintf ersetzt die definierte Länge (sizeof tmp_string) des strings tmp_string mit dem neuen String
    snprintf(tmp_string,sizeof tmp_string, "%s\"%s\",", JSON_STR_TIMESTAMP, example_data.timestamp);
    strncat(out_buffer, tmp_string, JSON_SIZE - strlen(out_buffer) -1);
  }


  // station_id

  // Prüfen, ob String leer ist
  if (strcmp(example_data.station_id, "") == 0) { 

    snprintf(tmp_string,sizeof tmp_string, "%s\"empty\"\n", JSON_STR_STATION_ID);
    strncat(out_buffer, tmp_string, JSON_SIZE - strlen(out_buffer) - 1);
  
    // Prüfen, ob String die richtige Länge hat
  } else if (strlen(example_data.station_id) != 3) {
    printf("station_id has invalid format\n");
    return -1;

  } else {

    snprintf(tmp_string,sizeof tmp_string, "%s\"%s\",", JSON_STR_STATION_ID, example_data.station_id);
    strncat(out_buffer, tmp_string, JSON_SIZE - strlen(out_buffer) -1);
  }


  printf("out_buffer:\n%s\n", out_buffer);

  return 0;

}