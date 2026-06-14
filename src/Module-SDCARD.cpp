#include "Module-SDCARD.h"

bool sdCardAvailable = false;
//-----------------Initialisation de la carte SD----------------------------
void initSDCard()
{
  if (!SD.begin())
  {
    Serial.println("Card Mount Failed");
    sdCardAvailable = false;
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  sdCardAvailable = true;
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMBn", cardSize);
  Serial.println("");
  Serial.println("--------------------------------------------------");
  Serial.println("");
}
void loadConfiguration(){;


}