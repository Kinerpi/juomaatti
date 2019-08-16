#include <Encoder.h>
#include <Servo.h>

// Tähän listataan kaikki liitettävät ja niiden pinnit.
const int ENCODER_PIN1 = 3;
const int ENCODER_PIN2 = 2;

const int RAJA_OIKEA = 6;
const int RAJA_VASEN = 7;

const int MOOTTORI_OIKEA = 8;
const int MOOTTORI_VASEN = 9;

const int KELKKA_SERVO = 10;
const int TOLPPA1_SERVO = 11; // EI KYTKETTY
const int TOLPPA2_SERVO = 12;
const int TOLPPA3_SERVO = 46;

const int INPUT_PIN_STOP = 21; //HÄTÄSEIS
const int INPUT_PIN1 = 22;
const int INPUT_PIN2 = 23;
const int INPUT_PIN3 = 24;
const int INPUT_PIN_EXECUTE = 25;
const int OUTPUT_KIIRE = 26;
const int OHJELMANVALINTA = 27;
const int OUTPUT_PIN_ERROR = 28;

int buttonstate = 1;
int lastButtonstate = 1;

//
unsigned long aikaVanha;
const long aikaMax = 15000;


/*
 * Kelkassa olevan servon ylä ja ala raja.
 * HOX!! Nämä täytyy muuttaa mikäli servo
 * lyö tyhjää tai irroitetaan rattaistaan.
 */
const int KELKANNOSTO = 55;
const int KELKANLASKU = 72;


// Tolppien positiot oikeasta laidasta.
const long TOLPPA1 = 8100;
const long TOLPPA2 = 35000;
const long TOLPPA3 = 61000;
// toleranssi jota käytetään kelkan ajoa ohjaavassa funktiossa.
const long toleranssi = 1000;

// Testi kohtaa käytetään testi sekvenssin aikana kelkan servon testiin.
const long KELKKA_TESTI = 20000;

//Lantrinki pumppujen käyttöaika. Säädä sopivaksi.
const long PUMPPAUS_AIKA = 2000;

/*
 * Karusellien pysähdyspaikat taulukoituna.
 * Taulukossa ensimmäinen pysty ja vaaka on tyhjiä,
 * jotta indeksointi alkamaan 1:sestä
*/

const int TOLPPA_ASENNOT[4][5] {
  //       0      90     180     270
  {0  ,  0  ,    0  ,    0  ,    0}, //Tyhjä tarkoituksella
  {0  ,  0  ,   20  ,   40  ,   60}, //Tolppa 1
  {0  ,  1  ,   19  ,   40  ,   58}, //Tolppa 2
  {0  ,  0  ,   16  ,   35  ,   55}, //Tolppa 3
};

const int TOLPPA_SERVOT[4] {
  0 ,
  TOLPPA1_SERVO ,
  TOLPPA2_SERVO ,
  TOLPPA3_SERVO
};

const long TOLPPA_SIJAINNIT[4] {
  0,
  TOLPPA1,
  TOLPPA2,
  TOLPPA3
};
/*
 * Tässä on taulukoituna lantrinkien lähdöt.
 * Ensimmäisenä on pumppu ja toisena on sen servo.
 * Ensimmäinen rivi jätetty tyhjäksi tarkoituksella.
 */
const int HANAT[7][2] {
  //{pumppu, servo}
  { 0,  0},
  {32, 33}, // pumppu 1, servo 1
  {34, 35}, // pumppu 2, servo 2
  {36, 37}, // pumppu 3, servo 3
  {38, 39}, // pumppu 4, servo 4
  {40, 41}, // pumppu 5, servo 5
  {42, 43}  // pumppu 6, servo 6
};


Servo AKTIIVINEN_SERVO;

Encoder sijainti(ENCODER_PIN1, ENCODER_PIN2);
long oldPosition;


/********************************************************************************
  ______ _   _ _   _ _____ _____ _____ _____ _   _  _____
  |  ___| | | | \ | /  __ \_   _|_   _|  _  | \ | |/  ___|
  | |_  | | | |  \| | /  \/ | |   | | | | | |  \| |\ `--.
  |  _| | | | | . ` | |     | |   | | | | | | . ` | `--. \
  | |   | |_| | |\  | \__/\ | |  _| |_\ \_/ / |\  |/\__/ /
  \_|    \___/\_| \_/\____/ \_/  \___/ \___/\_| \_/\____/

*********************************************************************************/
/*
*Lukee ulkoisen ohjauksen bitit.
*/
int lueBin() {
  int input = 0;
  bitWrite(input, 0, digitalRead(INPUT_PIN1));
  bitWrite(input, 1, digitalRead(INPUT_PIN2));
  bitWrite(input, 2, digitalRead(INPUT_PIN3));
  return input;
}
/*<
    Valitaan karusellin asento / lantrinki hana.
    Pyöräytetään karuselli kohdalleen ja nostetaan kelkkaa
    tai avataan pumppu servot ja pumpataan lantrinkia eli ajetaan "pumppaa" funktio.
*/
void juomanValinta(int valTieto) {
  Serial.print("Odotetaan toista valintaa: ");
  digitalWrite(OUTPUT_KIIRE, LOW);

  lastButtonstate = buttonstate;
  bool toinenVaihe = true;
  while (toinenVaihe) {

    buttonstate = digitalRead(INPUT_PIN_EXECUTE);
    if (buttonstate != lastButtonstate) {
      lastButtonstate = buttonstate;
      delay(50);

      if (buttonstate == LOW) {
        toinenVaihe = false;
        int binTieto = lueBin();
        digitalWrite(OUTPUT_KIIRE, HIGH);
        Serial.println(binTieto);

        if (valTieto < 4) {
          if (1 > binTieto || binTieto > 4) {return;}
		  
          ajaPaikkaan(TOLPPA_SIJAINNIT[valTieto]);
          AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[valTieto][binTieto]);
          AKTIIVINEN_SERVO.attach(TOLPPA_SERVOT[valTieto]);
          delay(10000);
          AKTIIVINEN_SERVO.detach();

          AKTIIVINEN_SERVO.write(55);
          AKTIIVINEN_SERVO.attach(KELKKA_SERVO);
          delay(5000);
          AKTIIVINEN_SERVO.detach();
          delay(1000);

          AKTIIVINEN_SERVO.write(71);
          AKTIIVINEN_SERVO.attach(KELKKA_SERVO);
          delay(5000);
          AKTIIVINEN_SERVO.detach();
          
          Serial.println("Karuselli: DONE");
        } else {
          if (1 > binTieto || binTieto > 6) {return;}
          if (binTieto < 4) {
            Serial.println("Ajetaan oikeaan laitaan.");
            ajaPaikkaan(0);
          } else {
            Serial.println("Ajetaan vasempaan laitaan.");
            ajaPaikkaan(72000);
          }
          pumppaa(HANAT[binTieto]);

        }

      }

    }
  }
}

/*
   Ajaa kelkan oikeaan paikkaan. annetaan paikka.
   annetaan haluttu positio ajaPaikkaan funkitasdsdgbisBGSDBG

*/
void ajaPaikkaan(long p) {
  aikaVanha = millis();

  long S = sijainti.read();
  long yla = S + toleranssi;
  long ala = S - toleranssi;

  if ( ala <= p && p <= yla ) {
    Serial.println("Ei kannata, Close enought.");
    return;
  }
  if (S > p) {
    while (digitalRead(RAJA_OIKEA) != HIGH && S > p) {
      digitalWrite(MOOTTORI_OIKEA, HIGH);
      S = sijainti.read();
      {if ((millis() - aikaVanha) > aikaMax){emergencyStop();}}
    }
    digitalWrite(MOOTTORI_OIKEA, LOW);

    delay(500);

    return;

  } else {
    while (digitalRead(RAJA_VASEN) != HIGH && S < p) {
      digitalWrite(MOOTTORI_VASEN, HIGH);
      S = sijainti.read();
      {if (millis() - aikaVanha > aikaMax){emergencyStop();}}
    }
    digitalWrite(MOOTTORI_VASEN, LOW);

    delay(500);

    return;
  }

}

/*
   Hätäseis:
   - sammuttaa moottorin
   - pysäyttää pumput
   - sulkee putket
*/
void emergencyStop() {
  digitalWrite(MOOTTORI_OIKEA, LOW);
  digitalWrite(MOOTTORI_VASEN, LOW);
  //digitalWrite(HANAT[1][0], LOW);
  //digitalWrite(HANAT[2][0], LOW);
  //digitalWrite(HANAT[3][0], LOW);
  digitalWrite(HANAT[4][0], LOW);
  digitalWrite(HANAT[5][0], LOW);
  digitalWrite(HANAT[6][0], LOW);
  AKTIIVINEN_SERVO.write(180);
  digitalWrite(OUTPUT_PIN_ERROR, HIGH);
  while (true) {}

}

/*
   Lähettää kelkan tämänhetkisen sijainnin
   sarjaportin kautta.

   0 = Oikea raja (laitteiston takaa katsottuna)
*/
void printSijainti() {
  long newPosition = sijainti.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
  }
}

/*
   Avaa valitun pumpun letkun ja
   pumppaa pumppua.
   Ottaa arrayn jossa
   [0] = pumpun pin
   [1] = servon pin

*/
void pumppaa(const int pumppu[]) {
  Serial.print("Pumppaa pumppaa");
  AKTIIVINEN_SERVO.detach();
  delay(1000);
  Serial.print(".");
  AKTIIVINEN_SERVO.write(0);
  AKTIIVINEN_SERVO.attach(pumppu[1]);
  delay(2000);
  Serial.print(".");
  digitalWrite(pumppu[0], HIGH);
  delay(PUMPPAUS_AIKA);
  digitalWrite(pumppu[0], LOW);

  AKTIIVINEN_SERVO.write(180);
  Serial.print(".");
  delay(5000);
  Serial.println(".");
  AKTIIVINEN_SERVO.detach();
  Serial.println("Pumppaus valmis!");
}

/********************************************************************************
  _____ _____ _____ _   _______
  /  ___|  ___|_   _| | | | ___ \
  \ `--.| |__   | | | | | | |_/ /
  `--. \  __|  | | | | | |  __/
  /\__/ / |___  | | | |_| | |
  \____/\____/  \_/  \___/\_|

*********************************************************************************/


void setup() {
  

  Serial.begin(9600);
  Serial.println("JUOMAATTI 2: JUDGEMENT DAY");
  pinMode(MOOTTORI_OIKEA, OUTPUT);
  pinMode(MOOTTORI_VASEN, OUTPUT);
  pinMode(RAJA_OIKEA, INPUT);
  pinMode(RAJA_VASEN, INPUT);

  pinMode(INPUT_PIN1, INPUT_PULLUP);
  pinMode(INPUT_PIN2, INPUT_PULLUP);
  pinMode(INPUT_PIN3, INPUT_PULLUP);
  pinMode(INPUT_PIN_EXECUTE, INPUT_PULLUP);
  pinMode(INPUT_PIN_STOP, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INPUT_PIN_STOP), emergencyStop, LOW);
  pinMode(OHJELMANVALINTA, INPUT_PULLUP);

  // OUTPUT_KIIRE kertoo ulkoisellle logiikalle onko arduino valmis ottamaan käskyjä.
  pinMode(OUTPUT_KIIRE, OUTPUT);
  digitalWrite(OUTPUT_KIIRE, HIGH);
  
  pinMode(OUTPUT_PIN_ERROR, OUTPUT);

  //pinMode(HANAT[1][0], OUTPUT);
  //pinMode(HANAT[2][0], OUTPUT);
  //pinMode(HANAT[3][0], OUTPUT);
  pinMode(HANAT[4][0], OUTPUT);
  pinMode(HANAT[5][0], OUTPUT);
  pinMode(HANAT[6][0], OUTPUT);

  buttonstate = digitalRead(INPUT_PIN_EXECUTE);
  lastButtonstate = buttonstate;


  /*
     Ajetaan kelkka oikeaan rajaan ja asetetaan sijainti
     laskuri nollaksi.

     Lisäksi ajetaan kelkan hissi ala-asentoon.
  */
  
  Serial.println("Kalibroidaan järjestelmää...");
  AKTIIVINEN_SERVO.write(KELKANLASKU);
  AKTIIVINEN_SERVO.attach(KELKKA_SERVO);
  delay(5000);
  AKTIIVINEN_SERVO.detach();
  delay(2000);
  
  aikaVanha  = millis();
  if (digitalRead(RAJA_OIKEA) == LOW) {
    digitalWrite(MOOTTORI_OIKEA, HIGH);
  }
  while (digitalRead(RAJA_OIKEA) == LOW) {if (millis() - aikaVanha > aikaMax){emergencyStop();}}
  digitalWrite(MOOTTORI_OIKEA, LOW);
  delay(1000);

  sijainti.write(0);
  delay(1000);





  if(digitalRead(OHJELMANVALINTA) == LOW){Serial.println("Testi ohjelma alkaa");testi();}

  Serial.print("Odottaa valintaa:");
  digitalWrite(OUTPUT_KIIRE, LOW);
}


/********************************************************************************
  _     _____  ___________
  | |   |  _  ||  _  | ___ \
  | |   | | | || | | | |_/ /
  | |   | | | || | | |  __/
  | |___\ \_/ /\ \_/ / |
  \_____/\___/  \___/\_|

*********************************************************************************/

void loop() {
  /*
     Koodissa on kaksi erillistä osaa.
     Ensimmäinen osio on normaali ajo jolloin vaaditaan ulkoiset inputit.
     Toisessa osiossa on testi koodi jolla voidaan testata laitteiden toimivuus.
     Testiohjelma voidaan ajaa käyttämällä laitteen oikeassa kyljessä olevaa nokkakytkintä.
  */

  buttonstate = digitalRead(INPUT_PIN_EXECUTE);

  if (buttonstate != lastButtonstate) {

    if (buttonstate == LOW) {
      int binTieto = lueBin();
      digitalWrite(OUTPUT_KIIRE, HIGH);
      switch (binTieto) {

        case 1:
          Serial.println(" 1");

          juomanValinta(binTieto);

          break;

        case 2:
          Serial.println(" 2");

          juomanValinta(binTieto);

          break;

        case 3:
          Serial.println(" 3");

          juomanValinta(binTieto);

          break;

        case 4:
          Serial.println(" HANAT");

          juomanValinta(binTieto);

          break;

        case 6:
          delay(100);
          digitalWrite(OUTPUT_KIIRE, HIGH);
          ajaPaikkaan(KELKKA_TESTI);
          Serial.println(" Done");
          Serial.println("Hölkyn kölkyn!!");
          break;

        default:
          Serial.println(" Olet tyhäm");
      }
      digitalWrite(OUTPUT_KIIRE, LOW);
      Serial.print("Odottaa valintaa:");
    }
  }
  lastButtonstate = buttonstate;
  delay(100);
}
  /********************************************************************************
    _____  _____  _____  _____  _____
    |_   _||  ___|/  ___||_   _||_   _|
    | |  | |__  \ `--.   | |    | |
    | |  |  __|  `--. \  | |    | |
    | |  | |___ /\__/ /  | |   _| |_
    \_/  \____/ \____/   \_/   \___/

  *********************************************************************************/

  /*
        Tämä testiohjelma on tarkoitettu juomaatissa olevien laitteiden toiminnan testaamiseen.
        tämä ei vaadi ulkoisia inputteja.

        1. Kelkka ajaa vasempaan laitaan.
        2. Kelkka ajaa oikeaan laitaan pysähtyen jokaisen karusellin kohdalle.
        3. Kelkka ajetaan ensimmäisen ja toisen tolpan väliin ja ajetaan ylös ja alas.
        4. Karuselli tanssi.
        5. Käytetään pumppuja. ÄLÄ AJA JOS PUMPPUJEN LETKUISSA ON NESTETTÄ >-<
  */


  void testi(){
    // 1.
  /* delay(1000);

    aikaVanha = millis();
    digitalWrite(MOOTTORI_VASEN, HIGH);
    while(digitalRead(RAJA_VASEN) == LOW){{if (millis() - aikaVanha > aikaMax){emergencyStop();}}}
    digitalWrite(MOOTTORI_VASEN, LOW);

    delay(2000);

    // 2.
    Serial.println("Kelkka pysähtyy tolpalle 3.");
    aikaVanha = millis();
    digitalWrite(MOOTTORI_OIKEA, HIGH);
    while(sijainti.read() > TOLPPA3){{if (millis() - aikaVanha > aikaMax){emergencyStop();}}}
    digitalWrite(MOOTTORI_OIKEA, LOW);
    delay(1000);

    Serial.println("Kelkka pysähtyy tolpalle 2");
    aikaVanha = millis();
    digitalWrite(MOOTTORI_OIKEA, HIGH);
    while(sijainti.read() > TOLPPA2){{if (millis() - aikaVanha > aikaMax){emergencyStop();}}}
    digitalWrite(MOOTTORI_OIKEA, LOW);
    delay(1000);

    Serial.println("Kelkka pysähtyy tolpalle 1");
    aikaVanha = millis();
    digitalWrite(MOOTTORI_OIKEA, HIGH);
    while(sijainti.read() > TOLPPA1){{if (millis() - aikaVanha > aikaMax){emergencyStop();}}}
    digitalWrite(MOOTTORI_OIKEA, LOW);
    delay(1000);

    aikaVanha = millis();
    digitalWrite(MOOTTORI_OIKEA, HIGH);
    while(digitalRead(RAJA_OIKEA) == LOW){{if (millis() - aikaVanha > aikaMax){emergencyStop();}}}
    digitalWrite(MOOTTORI_OIKEA, LOW);

    delay(2000);

    // 3.

    aikaVanha = millis();
    digitalWrite(MOOTTORI_VASEN, HIGH);
    while(sijainti.read() < KELKKA_TESTI){{if (millis() - aikaVanha > aikaMax){emergencyStop();}}}
    digitalWrite(MOOTTORI_VASEN, LOW);
    delay(1000);
    Serial.println("Kelkan nostimen testaus.");

    AKTIIVINEN_SERVO.write(55);
    AKTIIVINEN_SERVO.attach(KELKKA_SERVO);
    delay(5000);
    AKTIIVINEN_SERVO.detach();
    delay(1000);

    AKTIIVINEN_SERVO.write(71);
    AKTIIVINEN_SERVO.attach(KELKKA_SERVO);
    delay(5000);
    AKTIIVINEN_SERVO.detach();

    delay(2000);

    // 4.

    delay(1000);  */
/*
    // ____________________---Karuselli 1.---_____________________________
    Serial.println("Karuselli pyörähtää 4 eri positiota.");
    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[1][1]);
    AKTIIVINEN_SERVO.attach(TOLPPA1_SERVO);
    delay(5000);


    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[1][2]);
    AKTIIVINEN_SERVO.attach(TOLPPA1_SERVO);
    delay(5000);
    AKTIIVINEN_SERVO.detach();


    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[1][3]);
    AKTIIVINEN_SERVO.attach(TOLPPA1_SERVO);
    delay(5000);
    AKTIIVINEN_SERVO.detach();


    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[1][4]);
    AKTIIVINEN_SERVO.attach(TOLPPA1_SERVO);
    delay(5000);
    AKTIIVINEN_SERVO.detach();
    delay(1200);

    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[1][1]);
    AKTIIVINEN_SERVO.attach(TOLPPA1_SERVO);
    delay(8200);
    AKTIIVINEN_SERVO.detach();
    delay(1200);
*/
    /// ____________________---Karuselli 2.---_____________________________
    Serial.println("Karuselli 2 pyörähtää 4 eri positiota.");
    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[2][1]);
    AKTIIVINEN_SERVO.attach(TOLPPA2_SERVO);
    delay(8200);

    delay(500);
    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[2][2]);
    AKTIIVINEN_SERVO.attach(TOLPPA2_SERVO);
    delay(3000);
    AKTIIVINEN_SERVO.detach();

    delay(500);
    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[2][3]);
    AKTIIVINEN_SERVO.attach(TOLPPA2_SERVO);
    delay(3000);
    AKTIIVINEN_SERVO.detach();

    delay(500);
    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[2][4]);
    AKTIIVINEN_SERVO.attach(TOLPPA2_SERVO);
    delay(3000);
    AKTIIVINEN_SERVO.detach();
    delay(1200);

    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[2][1]);
    AKTIIVINEN_SERVO.attach(TOLPPA2_SERVO);
    delay(8200);
    AKTIIVINEN_SERVO.detach();
    delay(1200);
    // ____________________---Karuselli 3.---_____________________________
    Serial.println("Karuselli 3 pyörähtää 4 eri positiota.");
    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[3][1]);
    AKTIIVINEN_SERVO.attach(TOLPPA3_SERVO);
    delay(8200);

    delay(500);
    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[3][2]);
    AKTIIVINEN_SERVO.attach(TOLPPA3_SERVO);
    delay(3000);
    AKTIIVINEN_SERVO.detach();

    delay(500);
    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[3][3]);
    AKTIIVINEN_SERVO.attach(TOLPPA3_SERVO);
    delay(3000);
    AKTIIVINEN_SERVO.detach();

    delay(500);
    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[3][4]);
    AKTIIVINEN_SERVO.attach(TOLPPA3_SERVO);
    delay(3000);
    AKTIIVINEN_SERVO.detach();
    delay(1200);

    AKTIIVINEN_SERVO.write(TOLPPA_ASENNOT[3][1]);
    AKTIIVINEN_SERVO.attach(TOLPPA3_SERVO);
    delay(8200);
    AKTIIVINEN_SERVO.detach();
    delay(1200);
    /*
    // 5.
    pumppaa(PUMPPU4);
    pumppaa(PUMPPU5);
    pumppaa(PUMPPU6);
    */
    Serial.println("Testi ohjelma on valmis.");
    
    while(1);
}
