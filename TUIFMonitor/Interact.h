void changeItem(ESPRotary& r) {
  
  switch (r.getDirection()) {
    case 1:
      //Serial.print("Left Turn: ");
      rotary_step--;
      if (rotary_step < 0) rotary_step = itemcount - 1;
      break;
    case 255:
      //Serial.print("Right Turn: ");
      rotary_step++;
      if (rotary_step > itemcount - 1) rotary_step = 0;
      break;
  }

  currentItem = rotary_step;

  updateScreen = true;
  
}

void switchDirection(Button2& btn) {

  Items ci = items[currentItem];
  if (ci.type == wienerlinien) {
    wl[ci.index].activeDirection ^= 1;
  }
  
  updateScreen = true;
 
}
