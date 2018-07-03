// This function executes a delay based (blocking) beep
void beep(int timeMs){
  digitalWrite(buzzer, HIGH);
  delay(timeMs);
  digitalWrite(buzzer, LOW);
}

void beepArmDisarm(){
  leds[0] = CRGB::Yellow;
  leds[1] = CRGB::Yellow;
  FastLED.show();

  for(int i = 0; i <= 40; i++){
    beep(20);
    delay(20);
  }

  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  FastLED.show();
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
      leds[0] = CRGB::Green;
      leds[1] = CRGB::Green;
      FastLED.show();

      if(digitalRead(buzzer) == HIGH){
        digitalWrite(buzzer, LOW);
      }
      break;

    case 1: // Arming/Disarming
      switch(beepStep){
        case 1:
          if(timerMillis == 0)
            timerMillis = millis();
          digitalWrite(buzzer, HIGH);

          leds[0] = CRGB::Yellow;
          leds[1] = CRGB::Yellow;
          FastLED.show();

          if(millis() - timerMillis >= 100){
            beepStep = 2;
            timerMillis = millis();
          }
          break;
        
        case 2:
          digitalWrite(buzzer, LOW);

          leds[0] = CRGB::Black;
          leds[1] = CRGB::Black;
          FastLED.show();

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

          leds[0] = CRGB::Red;
          leds[1] = CRGB::Black;
          FastLED.show();

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
          leds[0] = CRGB::Black;
          leds[1] = CRGB::Red;
          FastLED.show();
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

          leds[0] = CRGB::Red;
          leds[1] = CRGB::Black;
          FastLED.show();

          digitalWrite(buzzer, HIGH);
          if(millis() - timerMillis >= 250){
            beepStep = 2;
            timerMillis = millis();
          }
          break;
        
        case 2:
          digitalWrite(buzzer, LOW);

          leds[0] = CRGB::Black;
          leds[1] = CRGB::Red;
          FastLED.show();

          if(millis() - timerMillis >= 250){
            beepStep = 1;
            timerMillis = 0;
          }
          break;
      }
      break;

    case 4: // Exploded
      switch(beepStep){
        case 1:
          if(timerMillis == 0)
            timerMillis = millis();

          leds[0] = CRGB::Red;
          leds[1] = CRGB::Yellow;
          FastLED.show();

          digitalWrite(buzzer, HIGH);

          if(millis() - timerMillis >= 150){
            beepStep = 2;
            timerMillis = millis();
          }
          break;
        
        case 2:
          leds[0] = CRGB::Yellow;
          leds[1] = CRGB::Red;
          FastLED.show();

          if(millis() - timerMillis >= 150){
            beepStep = 1;
            timerMillis = 0;
          }
          break;
      }
      break;

    case 5: // Disarmed
      switch(beepStep){
        case 1:
          if(timerMillis == 0)
            timerMillis = millis();
          
          digitalWrite(buzzer, HIGH);

          leds[0] = CRGB::Green;
          leds[1] = CRGB::Green;
          FastLED.show();

          if(millis() - timerMillis >= 600){
            beepStep = 2;
            timerMillis = millis();
          }
          break;
        
        case 2:
          digitalWrite(buzzer, LOW);

          leds[0] = CRGB::Black;
          leds[1] = CRGB::Black;
          FastLED.show();

          if(millis() - timerMillis >= 600){
            beepStep = 3;
            timerMillis = millis();
          }
          break;
          
        case 3:
          digitalWrite(buzzer, HIGH);

          leds[0] = CRGB::Green;
          leds[1] = CRGB::Green;
          FastLED.show();

          if(millis() - timerMillis >= 600){
            beepStep = 4;
            timerMillis = millis();
          }
          break;

        case 4:
          digitalWrite(buzzer, LOW);

          leds[0] = CRGB::Black;
          leds[1] = CRGB::Black;
          FastLED.show();

          if(millis() - timerMillis >= 600){
            beepStep = 5;
            timerMillis = millis();
          }
          break;

        case 5:
          digitalWrite(buzzer, HIGH);

          leds[0] = CRGB::Green;
          leds[1] = CRGB::Green;
          FastLED.show();

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
