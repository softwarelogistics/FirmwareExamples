#include <Arduino.h>

#define TEMP_SNSR_BOARD_V3

#include <NuvIoT.h>
#include <HX711.h>



#define FIRMWARE_VERSION "1.0.0"
#define EXAMPLE_SKU "HX711 Example"

HX711 loadcell;


//const long LOADCELL_OFFSET = 50682624;
//const long LOADCELL_DIVIDER = 5895655;


const long LOADCELL_OFFSET = 0;
const long LOADCELL_DIVIDER = 1;

void setup() {

  initPins();


  loadcell.begin(26, 25);
  loadcell.set_scale(LOADCELL_DIVIDER);
  loadcell.set_offset(LOADCELL_OFFSET);

  configureConsole();
  
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);
  
  configureFileSystem();
}

void loop() {
  console.setVerboseLogging(true);
  loadcell.set_gain(64);
  console.print("Weight - a: ");
  console.println(String(loadcell.get_units(10), 2));
  loadcell.set_gain(32);
  console.print("Weight - b: ");
  console.println(String(loadcell.get_units(10), 2));


  delay(500);
}