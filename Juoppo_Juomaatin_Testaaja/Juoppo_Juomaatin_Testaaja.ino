/*
 * JUOPPO
 * Juomaatin Ultra Optimoitu Prosessi Pilotointi Ohjain 
 * 
 * Mahdollistaa binäärikäskyjen lähettämisen Juomaatille.
 * 
 */

const int io1 = 2, 
          io2 = 3, 
          io4 = 4,
          exe = 5,
          busy= 6;

const int output[]{io1,io2,io4};

int input;

void setup() {
  Serial.begin(9600);
  Serial.println("---JUOPPO---");
  pinMode(io1, OUTPUT);
  pinMode(io2, OUTPUT);
  pinMode(io4, OUTPUT);
  pinMode(exe, OUTPUT);
  pinMode(busy, INPUT);
  digitalWrite(exe, 1);
  
}

void loop() {
  digitalWrite(exe, 1);
  while(digitalRead(busy)){delay(100);Serial.println(Serial.available());};
  if (Serial.available() > 0){
    input = Serial.parseInt();
    if(0 < input && input < 8){
    Serial.print("Input: ");
    Serial.println(input);
    digitalWrite(output[0] , bitRead(input, 0));
    digitalWrite(output[1] , bitRead(input, 1));
    digitalWrite(output[2] , bitRead(input, 2));
    
    delay(200);
    digitalWrite(exe, 0);
    delay(200);
    }
  

    
  }

}
