const int ARRAY_SIZE = 100; // Average for flame sensor, bigger more accurate
const int RELAY_PIN = 7;
int initialSensorTicks = 1000; // Initial ticks for the temperature sensor to get an average
int reAdjustTicks = 1000;
int UVtotalSumForFire = 1500;  // Total UV needed to comfirm for fire
int alarmTimer = 200;  // Time for the alarm to be on

int i = 0;

int fireValue = 0;
int tempValue = 0;

int avgFireValue = 0;
long avgFireValueSum = 0;
long tempTotalValue = 0;
long fireTotalValue = 0;
int recentAverageTempValue = 0;
int recentAverageFireValue = 0;
int fireInitialCounter = 0;
int tempInitialCounter = 0;
int adjustmentDiff = 0;
int variableAdjustment = 0;
int reAdjustmentCounter = 0;
int UVtotalValue = 0;
int alarmCounter = 0;

int recentTotalCount = 0;
int lastFireValue = 0;
int recentTotalFireValues[ARRAY_SIZE];
int diff = 0;
int UVvalue = 0;

bool start = true;
bool tempMarker = true;
bool fireMarker = true;
bool reAdjustmentMarker = false;
bool fire = false;

void setup() {
  Serial.begin(9600);

  pinMode(A0, INPUT); // IR sensor (fire sensor)
  pinMode(A1, INPUT); // Temperature sensor
  pinMode(A2, INPUT); // UV sensor

  pinMode(RELAY_PIN, OUTPUT); // Relay
}

void loop() {
  
  // Get the sensor values in the current cycle
  fireValue = analogRead(A0);
  tempValue = analogRead(A1);
  UVvalue = analogRead(A2);

  // Get the average temperature since the start of the program
  // until 1000 ticks
  if(tempMarker) {
    if (tempInitialCounter >= initialSensorTicks) {
      recentAverageTempValue = tempTotalValue / initialSensorTicks;
      tempMarker = false;
    } else {
      tempTotalValue += tempValue;
      tempInitialCounter += 1;
    }
  }

  // If the Detector is recently started
  if (start) {
    
    // Fill the first recent fire values in the array
    recentTotalFireValues[recentTotalCount] = fireValue;
    recentTotalCount += 1;

    // After it gets enough data set the start to false
    if (recentTotalCount >= ARRAY_SIZE) {
      start = false;
    }
  }

  // If the Detector gets enoguh data to start calculating the average
  if (!start) {

    // Update the recent fire values array with the most recent value,
    // and delete the oldest
    for (i = 1; i < ARRAY_SIZE; i++) {
      recentTotalFireValues[i-1] = recentTotalFireValues[i];
    }
    recentTotalFireValues[ARRAY_SIZE-1] = fireValue;

    // Calculate the average value from the recent fire values array
    avgFireValueSum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
      avgFireValueSum += recentTotalFireValues[i];
    }
    avgFireValue = avgFireValueSum / ARRAY_SIZE;
    
    // Calculate the difference between the average fire value and the 
    // room temeperature value
    diff = (avgFireValue - (tempValue + variableAdjustment));

    // If the difference is grader or less than 100 there is a
    // fire, and activate the relay
    if (diff > 100 || diff < -100) {
      UVtotalValue += UVvalue;
      if (UVtotalValue >= UVtotalSumForFire) {
        fire = true;
      }
    } else {
      UVtotalValue = 0;
    }

    // Get the average fire value since the start of the fire sensor
    if(fireMarker) {
      if (fireInitialCounter >= initialSensorTicks) {
        recentAverageFireValue = fireTotalValue / initialSensorTicks;
        fireMarker = false;
      } else {
        fireTotalValue += avgFireValue;
        fireInitialCounter += 1;
      }
    }

  }

  if (recentAverageFireValue != 0 && recentAverageTempValue != 0) {
    adjustmentDiff = recentAverageFireValue - recentAverageTempValue;

    variableAdjustment = adjustmentDiff;
    recentAverageFireValue = 0;
    recentAverageTempValue = 0;
    
    reAdjustmentCounter = 0;
    reAdjustmentMarker = true;
  } else {
    if (reAdjustmentMarker) {
      reAdjustmentCounter += 1;
  
      if (reAdjustmentCounter >= reAdjustTicks) {
        tempMarker = true;
        tempTotalValue = 0;
        tempInitialCounter = 0;
        
        fireMarker = true;
        fireTotalValue = 0;
        fireInitialCounter = 0;
  
        reAdjustmentMarker = false;
      }
    }
  }

  if (fire) {
    digitalWrite(RELAY_PIN, HIGH);
    alarmCounter -= 1;
  
    if (alarmCounter <= 0) {
      alarmCounter = 0;
      UVtotalValue = 0;
      
      fire = false;
    }
    
  } else {
    digitalWrite(RELAY_PIN, LOW);
    alarmCounter = alarmTimer;
  }

  Serial.print(fireValue); // print the raw flame sensor
  Serial.print("\t");
  Serial.print(avgFireValue); // print the average flame sensor
  Serial.print("\t");
  Serial.print(tempValue + variableAdjustment); // print the room temperature
  Serial.print("\t");
  Serial.print(UVvalue); // print the UV value
  Serial.print("\t");
  Serial.print(UVtotalValue);
  

  delay(100);

  Serial.println(); // line-feed
}
