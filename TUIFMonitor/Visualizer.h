void clearTerminal() {
  // https://stackoverflow.com/questions/10105666/clearing-the-terminal-screen
  Serial.write(27);       // ESC command
  Serial.print("[2J");    // clear screen command
  Serial.write(27);
  Serial.print("[H");     // cursor to home command
}

void departureLine(String line) {
  int lp = ((display.height() / 2) - 24) / 2;
  u8g2.setFont(u8g2_font_logisoso24_tf);

  for (int i = 0; i < line.length(); i++) {
    u8g2.setCursor(i * 13, (display.height() / 2) - lp + 3);
    u8g2.print(line.charAt(i));
    
    u8g2.setCursor(i * 13, display.height() - lp - 3);
    u8g2.print(line.charAt(i));
  }
}

void departureTime(String d0, String d1) {
  int lp = ((display.height() / 2) - 24) / 2;
  u8g2.setFont(u8g2_font_logisoso24_tf);
  
  if (d0.toInt() > 9) {
    u8g2.setCursor(display.width() - 17 - 15, (display.height() / 2) - lp + 3);
    u8g2.print(d0.charAt(0));
    u8g2.setCursor(display.width() - 17, (display.height() / 2) - lp + 3);
    u8g2.print(d0.charAt(1));
  } else {
    u8g2.setCursor(display.width() - 17, (display.height() / 2) - lp + 3);
    u8g2.print(d0);
  }

  if (d1.toInt() > 9) {
    u8g2.setCursor(display.width() - 17 - 15, display.height() - lp - 3);
    u8g2.print(d1.charAt(0));
    u8g2.setCursor(display.width() - 17, display.height() - lp - 3);
    u8g2.print(d1.charAt(1));
  } else {
    u8g2.setCursor(display.width() - 17, display.height() - lp - 3);
    u8g2.print(d1);
  }
}

void departureDirection(String name) {
  int dp = ((display.height() / 2) - 24) / 2;
  u8g2.setFont(u8g2_font_courB12_tf);
  
  if (name.length() > 13) {
    int sl = name.indexOf("/");
    if (sl == -1) {
      sl = name.indexOf(",");
    }
    if (sl > -1) {
      String p1 = name.substring(0, sl);
      p1.trim();
      p1 = p1.substring(0, 13);
      String p2 = name.substring(sl + 1);
      p2.trim();
      p2 = p2.substring(0, 13);

      u8g2.setCursor(44, (display.height() / 2) - dp - 12 + 3 - 1);
      u8g2.print(p1);
      u8g2.setCursor(44, (display.height() / 2) - dp - 00 + 3);
      u8g2.print(p2);

      u8g2.setCursor(44, display.height() - dp - 12 - 3 - 2);
      u8g2.print(p1);
      u8g2.setCursor(44, display.height() - dp - 00 - 3);
      u8g2.print(p2);
    } else {
      u8g2.setCursor(44, (display.height() / 2) - dp - 12 + 3);
      u8g2.print(name.substring(0, 13));

      u8g2.setCursor(44, display.height() - dp - 12 - 3);
      u8g2.print(name.substring(0, 13));
    }
  } else {
    u8g2.setCursor(44, (display.height() / 2) - dp - 12 + 3);
    u8g2.print(name);

    u8g2.setCursor(44, display.height() - dp - 12 - 3);
    u8g2.print(name);
  }
}

void showDepartures(String line, String direction, String d0, String d1) {
  display.firstPage();
  Serial.println("Display update started");
  do
  {
    display.fillScreen(GxEPD_WHITE);

    departureLine(line);
    
    departureDirection(direction);

    departureTime(d0, d1);
    
  }
  while (display.nextPage());
  Serial.println("Display update finished");
}

#define bike_width 40
#define bike_height 24
static const unsigned char bike_bits[] PROGMEM = {
  0x00, 0x3f, 0x03, 0xe0, 0x00, 0x00, 0x1e, 0x03, 0xe0, 0x00, 0x00, 0x06, 0x00, 0x20, 0x00, 0x00, 
  0x07, 0xff, 0xf0, 0x00, 0x00, 0x07, 0xff, 0xf0, 0x00, 0x00, 0x0f, 0x00, 0x78, 0x00, 0x00, 0x0d, 
  0x80, 0x78, 0x00, 0x07, 0xf9, 0x80, 0xdf, 0xe0, 0x1f, 0xf8, 0x81, 0x9f, 0xf8, 0x38, 0x3c, 0xc3, 
  0xbc, 0x1c, 0x30, 0x36, 0xc3, 0x66, 0x0c, 0x60, 0x63, 0x66, 0xc6, 0x06, 0xc0, 0x63, 0x6c, 0xc2, 
  0x03, 0xc0, 0xc1, 0x3c, 0x83, 0x03, 0xc1, 0xe3, 0xf9, 0x83, 0x83, 0xc1, 0xff, 0xf1, 0x83, 0x83, 
  0xc1, 0xc1, 0x81, 0x83, 0x83, 0xc0, 0x01, 0x00, 0x80, 0x03, 0x40, 0x03, 0x00, 0xc0, 0x02, 0x60, 
  0x03, 0x00, 0xc0, 0x06, 0x30, 0x06, 0x00, 0x60, 0x0c, 0x38, 0x1c, 0x00, 0x38, 0x1c, 0x0f, 0xf8, 
  0x00, 0x1f, 0xf0, 0x03, 0xe0, 0x00, 0x07, 0xc0 };

void showBikes(String name, String count) {
  display.firstPage();
  Serial.println("Display update started");
  do
  {    
    display.fillScreen(GxEPD_WHITE);

    int lp = ((display.height() / 2) - 20) / 2;
    u8g2.setFont(u8g2_font_logisoso18_tf);
  
    u8g2.setCursor(10, (display.height() / 2) - lp + 3);
    u8g2.print(name);

    u8g2.setCursor(60, display.height() - lp - 3);
    u8g2.print(count);
    u8g2.print(" verfügbar");

    display.drawBitmap(10, display.height() - lp - bike_height ,bike_bits, bike_width,bike_height, GxEPD_BLACK);
    
  }
  while (display.nextPage());
  Serial.println("Display update finished");
}

void showState() {

  Items ci = items[currentItem];
  
  if (ci.type == citybike) {
    // City Bike

    String dsk = "cb/" + cb[ci.index].key;
    String bikeCount;
    for (int i = 0; i < departurecount; i++) {
      if (strcmp(dsk.c_str(), departures[i].key.c_str()) == 0) {
        bikeCount = String(departures[i].time0);
      }
    }
    
    String stationName = cb[ci.index].name;

    clearTerminal();
    
    Serial.println("City Bike Update:");
    Serial.println("There are " + bikeCount + " free bikes at Station " + stationName + ".");

    showBikes(stationName, bikeCount);
    
  }

  if (ci.type == wienerlinien) {
    // Wiener Linien
    
    WL wc = wl[ci.index];

    String line = wc.line;
    int activeDirection = wc.activeDirection;

    String dsk = "wl/" + String(wc.keys[activeDirection]) + "/" + line;
    String stationDirection = wc.direction[activeDirection];
    String stationName = wc.name[activeDirection];

    Serial.println(dsk);
    
    String departure0;
    String departure1;
    for (int i = 0; i < departurecount; i++) {
      if (strcmp(dsk.c_str(), departures[i].key.c_str()) == 0) {
        departure0 = String(departures[i].time0);
        departure1 = String(departures[i].time1);
      }
    }

    clearTerminal();

    Serial.println("Wiener Linien Update:");
    Serial.println("The next vehicle for line " + line + " at " + stationName + " towards " + stationDirection + " arrives in " + departure0 + " minutes, the one afterwards in " + departure1 + " minutes.");

    showDepartures(line, stationDirection, departure0, departure1);
    
  }
  
}
