#ifndef STATICSERVE_H
#define STATICSERVE_H
#include <Arduino.h>
String getContentType(String filename);
bool handleFileRead(String path);
bool loadFromSPIFFS(String path, String dataType);
void handleNotFound();
#endif
