const int switchPin = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(switchPin, INPUT);
}

void readSerialData()
{
  String readData = Serial.readString();     // read the incoming data as string
  if(readData.endsWith(String("end")))
  {
    Serial.print("Match Found");
  }
  else
  {
    Serial.print("No Match");
  }
}

void loop() {
  if(Serial.available())
  {
    readSerialData();
  }
}
