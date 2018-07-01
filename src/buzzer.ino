// This function executes a delay based (blocking) beep
void beep(int timeMs){
  digitalWrite(buzzer, HIGH);
  delay(timeMs);
  digitalWrite(buzzer, LOW);
}

// This function executes non-blocking beep patterns
void doBuzzerStuff(){
  // === BUZZER STATUS ===
  // 0: Off
  // 1: Arming/Disarming
  // 2: Bomb armed
  // 3: About to explode
  // 4: Exploded
  // 5: Disarmed

  static int beepStep = 1;
  static long timerMillis = 0;
  static int previousStatus = 0;

  // When the status changes, reset the step
  if(buzzerStatus != previousStatus){
    previousStatus = buzzerStatus;
    beepStep = 1;
  }

  switch (buzzerStatus){
    case 0: // Off
      if(digitalRead(buzzer) == HIGH)
        digitalWrite(buzzer, LOW);
      break;

    case 1: // Arming/Disarming
      switch(beepStep){
        case 1:
          if(timerMillis == 0)
            timerMillis = millis();
          digitalWrite(buzzer, HIGH);
          if(millis() - timerMillis >= 100){
            beepStep = 2;
            timerMillis = millis();
          }
          break;
        
        case 2:
          digitalWrite(buzzer, LOW);
          if(millis() - timerMillis >= 900){
            beepStep = 1;
            timerMillis = 0;
          }
          break;
      }
      break;

    case 2: // Bomb armed
      switch(beepStep){
        case 1:
          if(timerMillis == 0)
            timerMillis = millis();
          digitalWrite(buzzer, HIGH);
          if(millis() - timerMillis >= 100){
            beepStep = 2;
            timerMillis = millis();
          }
          break;
        
        case 2:
          digitalWrite(buzzer, LOW);
          if(millis() - timerMillis >= 100){
            beepStep = 3;
            timerMillis = millis();
          }
          break;
          
        case 3:
          digitalWrite(buzzer, HIGH);
          if(millis() - timerMillis >= 100){
            beepStep = 4;
            timerMillis = millis();
          }
          break;
          
        case 4:
          digitalWrite(buzzer, LOW);
          if(millis() - timerMillis >= 700){
            beepStep = 1;
            timerMillis = 0;
          }
          break;
      }
      break;

    case 3: // About to explode
      switch(beepStep){
        case 1:
          if(timerMillis == 0)
            timerMillis = millis();
          digitalWrite(buzzer, HIGH);
          if(millis() - timerMillis >= 50){
            beepStep = 2;
            timerMillis = millis();
          }
          break;
        
        case 2:
          digitalWrite(buzzer, LOW);
          if(millis() - timerMillis >= 50){
            beepStep = 1;
            timerMillis = 0;
          }
          break;
      }
      break;

    case 4: // Exploded
      if(digitalRead(buzzer) == LOW)
        digitalWrite(buzzer, HIGH);
      break;

    case 5: // Disarmed
      switch(beepStep){
        case 1:
          if(timerMillis == 0)
            timerMillis = millis();
          digitalWrite(buzzer, HIGH);
          if(millis() - timerMillis >= 600){
            beepStep = 2;
            timerMillis = millis();
          }
          break;
        
        case 2:
          digitalWrite(buzzer, LOW);
          if(millis() - timerMillis >= 600){
            beepStep = 3;
            timerMillis = millis();
          }
          break;
          
        case 3:
          digitalWrite(buzzer, HIGH);
          if(millis() - timerMillis >= 600){
            beepStep = 4;
            timerMillis = millis();
          }
          break;

        case 4:
          digitalWrite(buzzer, LOW);
          if(millis() - timerMillis >= 600){
            beepStep = 5;
            timerMillis = millis();
          }
          break;

        case 5:
          digitalWrite(buzzer, HIGH);
          if(millis() - timerMillis >= 600){
            beepStep = 6;
            timerMillis = millis();
          }
          break;

        case 6:
          digitalWrite(buzzer, LOW);
          buzzerStatus = 0;
          break;
      }
      break;
  }
}
