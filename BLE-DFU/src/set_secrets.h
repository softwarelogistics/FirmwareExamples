#include <NuvIoT.h>

/* fill in network details below */

void setSysConfigSecrets(SysConfig *sysConfig) {
  sysConfig->DeviceId = "device001";
  sysConfig->WiFiSSID = "";
  sysConfig->WiFiPWD = "";
  sysConfig->SrvrType = "";
  sysConfig->SrvrHostName = "";
  sysConfig->SrvrUID = "";
  sysConfig->SrvrPWD = "";  
}
