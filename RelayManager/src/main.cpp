#define RELAY_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>

#define EXAMPLE_SKU "RELAY_MANAGER"
#define FIRMWARE_VERSION "1.0.0"

void setup() {
  configureI2C();
  configureConsole();
  initPins();

  sysConfig.setDefaults();
  relayManager.setup(&ioConfig);

  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);

  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "rly001", 010);  
}

long nextTransition = 0;
int relayIndex = 0;

/* Note - On the relay board, Rev 002, K5 is also used as DO on the JTag adapter 
          keep this in mind while doing development, when programming via Serial
          or once this is in production using K5 is not a problem.
  */
void loop() {
  console.setVerboseLogging(true);
  if(configPins.HasRelays)
  {    
    if(nextTransition < millis())
    {
      relayManager.debugPrint();

      if(relayIndex == 0) {
        relayManager.setRelay(configPins.NumberRelays - 1, false);
      }
      else {
          relayManager.setRelay(relayIndex - 1, false);
      }

      relayManager.setRelay(relayIndex, true);

      nextTransition = millis() + 2000;
      relayIndex++;
      if(relayIndex == configPins.NumberRelays)
      {
        relayIndex = 0;
      }
    }
  }
  else 
  {
    console.println("Sorry, the board does not have a relay configuration, please check board definition.");
  }  
}