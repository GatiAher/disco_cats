/*
   Button to Serial

   Send button selection over to laptop
*/

void setup() {
  Serial.begin(115200);

  /* Play seed files */
  Serial.print("0 e\n");
  delay(4000);
  Serial.print("0 f\n");
  delay(4000);
  Serial.print("0 h\n");
  delay(4000);
  Serial.print("0 i\n");

  /* Select interpolation - 2 seeds */
  Serial.print("0 g\n");
  delay(4000);
  Serial.print("0 d\n");
  delay(4000);
  Serial.print("1 g d\n");
  delay(50000);

  /* Play interpolation - 3 seeds */
  Serial.print("0 a\n");
  delay(4000);
  Serial.print("0 b\n");
  delay(4000);
  Serial.print("0 c\n");
  delay(4000);
  Serial.print("1 a b c\n");
  delay(50000);
}

void loop() {

}
