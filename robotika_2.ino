#include<LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(7,8,9,10,11,12);

//                 R,G,Z,M
int lightPins[] = {5,4,3,2};    // Dalinasi su mygtukais pinus tai bus smagumo :)

int roundsToWin = 30;           // Darom 30 raundu bet pasiliekam galimybe turet ir daugiau ju
int buttonSequence[40];

volatile int currButton = -1;            // Kuris mygtukas buvo paspaustas

int currRound = 0;              // Dabartinis raundas

int subRound = -1;              // Po-raundas, (kelintas mygtukas mygtuku eileje)

unsigned long totalGameTime = 0;
long gameTime = 0;              // po-raundzio vykimo laikas
long timeLimit = 2000;

int mode = 0;                   // dabartinis ekranas
bool gameStarted = false;       //ar vyksta zaidimas

int gameStatus = 0;             // -1 - pralaimetas zaidimas, 0 - vyksta zaidimas, 1 - laimetas zaidimas

bool updateScreen = true;       // Ar reikia atnaujinti ekrano vaizda

volatile unsigned long debounceTimes[] = {0,0,0,0}; // kiekvieno mygtuko paspaudimo pertraukos laikas
unsigned long debounceDelay = 200;

unsigned long flashTime = 500;  // Kiek laiko uzdegt lempute

struct HighScore {
  char name[10];
  byte rounds;                  // Laikom tarp baito nes max raundu tik 40
  int time;                     // Saugom visa zaidimo laika (gal)
};

HighScore scores[10];           // Laikymo vieta geriausiems rezultatams

char nameToEnter[] = "AAAAAAAAAA";
int position = -1;

int currScore = -1;            // Rezultatas i kuri dabar ziurim
bool moreScores = true;        // Ar yra dar rezultatu (highscores)




void setup() {
  // Pirmiausiai sutvarkom timerio pertraukima po to darom likusi setup
  cli();                        // Sustabdom pertraukimus
  TCCR1A = 0;                   // Kadangi naudosim timer1 tai nustatom TCCR1A i 0
  TCCR1B = 0;                   // Tas pats su TCCR1B
  TCNT1  = 0;                   // Duodam pradine nuline reiksme skaiciuokliui (counteriui)
  
  // Nustatom palyginimo registra del 100Hz pertraukimu (kas 10ms)
  OCR1A = 19999;                // = 16000000 / (8 * 100) - 1
  // Ijungiam timerio nunulinima kai registras pasiekia nurodyta reiksme
  TCCR1B |= (1 << WGM12);
  // Nustatom CS12 CS11 ir CS10 bitus del 8 padidintojo
  TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
  // Ijungiam timerio palyginimo pertraukima
  TIMSK1 |= (1 << OCIE1A);
  sei();                        // Leidziam pertraukimus

  Serial.begin(9600);           // Printinimui

  LightDown(2); LightDown(3);   // Ijungiam pertraukimus zaliai ir melynai


  lcd.begin(16,2);              // Ijungiam ekrana

  getScores();                  // Istraukiam issaugotus rezultatus, jei jie geri
}


ISR(TIMER1_COMPA_vect){
   // Vyksta kas 10 ms, tai 100Hz
   // Paskaityti: https://www.instructables.com/Arduino-Timer-Interrupts/
   // Skaiciuotuvas: https://www.8bit-era.cz/arduino-timer-interrupts-calculator.html

   if(gameStarted){
    gameTime++;
    if(gameStatus == 0){
      totalGameTime++;
    }
   }
}



void loop() {
  switch(mode){
    case 0:                   // Pradinis ekranas
      mainScreen();
      break;
    case 1:                   // Zaidimas
      game();
      break;
    case 2:                   // Vardo ivedimas
      nameEntry();
      break;
    case 3:                   // Rezultatu ekranas
      scoreScreen();
      break;
  }
}

void mainScreen(){            // Pradinis ekranas
  if(updateScreen){           // Jei reikia atnaujint ekrana
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("New game?  green");
    lcd.setCursor(0, 1);
    lcd.print("Highscores? blue");
    updateScreen = false;
  }

  if(currButton == 2){        // Reaguojam i zalia mygtuka
    mode = 1;
    currButton = -1;
    updateScreen = true;
    return;
  }
  if(currButton == 3){        // Reaguojam i melyna mygtuka
    mode = 3;
    currButton = -1;
    updateScreen = true;
    return;
  }
}

void game(){
  if(!gameStarted){           // Jei zaidimas nepradetas
    resetGame();              // pirmiausiai paruosiam kintamuosius
    lcd.clear();
    updateScreen = true;
  } else if(gameStatus == 0){ // Jei zaidimas vyksta
    if(updateScreen){          // Jei reikia atnaujinti ekrana
      lcd.clear();            // Isvalom ekrana
      lcd.setCursor(0,0);     // Parasom pradzioje ekrano dabartini raunda
      lcd.print("Round: ");   // Parasom dabartini raunda
      lcd.print(currRound);
      updateScreen = false;
    }
    if(currButton == -1){     // Jei mygtukas nepaspaustas
      checkButton();
    }
    if(currButton != -1){     // Jei buvo mygtukas paspaustas tikrinam ar teisingas
      bool correct = correctButton();
      if (!correct){
        gameStatus = -1;
        updateScreen = true;
        return;
      }
    }
    if(subRound == -1){       // Jei raundas neprasidejo arba pasibaige
      if(currRound >= roundsToWin){ // Ar laimejom
        gameStatus = 1;
        updateScreen = true;
        return;
      }
      else{
        playButtonSequence();
        updateScreen = true;
        gameTime = 0;
        subRound = 0;
        return;
      }
    } else if(gameTime > timeLimit){
      gameStatus = -2;
      updateScreen = true;
      return;
    }
  } else{                     // Jei zaidimas pasibaige
    if(updateScreen){         // Atspausdinam reikalus
      lcd.clear();
      lcd.setCursor(0, 0);
      if(gameStatus < 0){
        lcd.print("You Lost!");
      } else lcd.print("You Won!");
      lcd.setCursor(0, 1);
      if(isNewHighscore()){
        lcd.print("Save: g exit: b");
      } else{
        lcd.print("exit: blue");
      }
      updateScreen = false;
    }
    if(currButton == 2 && isNewHighscore()){    // Jei naujas rezultatas tada pereinam i vardo ivedimo ekrana 
      gameStarted = false;
      currButton = -1;
      mode = 2;
      position = -1;
      updateScreen = true;
      return;
    }
    if(currButton == 3){
      currButton = -1;
      gameStarted = false;
      mode = 0;
      updateScreen = true;
      return;
    }
  }
}

void resetGame(){             // Isvalom zaidimo kintamuosius, ir sukuriam nauja mygtuku eile
  randomSeed(analogRead(A5));
  for(int i =0; i <= roundsToWin; i++){
    buttonSequence[i] = random(0,4);
  }
  gameStarted = true;
  gameStatus = 0;
  totalGameTime = 0;
  currRound = 0;
  subRound = -1;
  gameTime = 0;
  flashAll();
  updateScreen = true;
}

void nameEntry(){           // Vardo ivedimas
  if(updateScreen){
    if (position == -1){    // Jei pradinis ivedimo ekranas
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Green:chngLetter");
      lcd.setCursor(0, 1);
      lcd.print("Blue:moveLeft");
    } else{                 // Kitais atvejais rodom varda
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("enter name:");
      lcd.setCursor(0, 1);
      lcd.print(nameToEnter);
      updateScreen = false;

    }
    updateScreen = false;
  }
  if(currButton == 2){
    updateScreen = true;
    currButton = -1;
    if(position == -1){   // Jei esam pradiniam vardo ivedimo ekrane
      position++;
      return;
    }
    nameToEnter[position]++;
    if(nameToEnter[position] > 122){
      nameToEnter[position] = 32;
    } 
  }
  if(currButton == 3){   // Jei paspaude melyna
    position++;
    if(position >= 10){     // jei perejom visas raides issaugom rezultata nunulinam varda ir griztam i pradini ekrana
      saveScore();
      writeScores();
      strncpy(nameToEnter, "AAAAAAAAAA", 10);
      position=-1;
      currButton = -1;
      updateScreen = true;
      totalGameTime = 0;
      currRound = 0;
      mode = 0;
    }
    currButton = -1;
  }
}


void scoreScreen(){
  if(updateScreen){
    if(currScore == -1){
      lcd.clear();
      currScore = 0;
      return;
    }
    if(scores[currScore].name[0] != '\0' && scores[currScore].name[0] != 0xff){ // Jei ne tuscias rezultatas
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(scores[currScore].name);
      lcd.setCursor(0,1);
      lcd.print("R:   T:");
      lcd.setCursor(2,1);
      lcd.print(scores[currScore].rounds);
      lcd.setCursor(7,1);
      lcd.print(scores[currScore].time / 100);        // Sekundziu dalis
      lcd.print(".");
      lcd.print(scores[currScore].time % 100);    // milisekundziu dalis
      lcd.print("s");
      updateScreen = false;
    } else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("No scores!");
      lcd.setCursor(0,1);
      lcd.print("G/B to exit");
      moreScores= false;
      updateScreen = false;
    }
  }
  if(currButton == 2){
    currButton = -1;
    updateScreen = true;
    if(!moreScores){    // jei nebera daugiau rezultatu, tada iseinam
      mode = 0;
      currScore = -1;
      moreScores = true;
      return;
    }
    currScore++;
    if(currScore >= 10){    // jei pasiekem gala, iseinam
      mode = 0;
      moreScores = true;
      currScore = -1;
      return;
    }
  }
  if(currButton == 3){    // iseinam
      mode = 0;
      moreScores = true;
      currScore = -1;
      updateScreen = true;
      currButton = -1;
      return;
  }

}


bool isNewHighscore(){ // Ar paskutinis rezultatas sarase yra tuscias arba ar praejom daugiau raundu
  return (scores[9].name[0] == '\0' || scores[9].name[0] == 0xff || scores[9].rounds < currRound);
}

void saveScore(){
  if (!isNewHighscore()) return;  // Jei rezultatas nera pakankamai geras

  // Pridedam nauja rezultata į gala sarašo
  strncpy(scores[9].name, nameToEnter, sizeof(scores[9].name) - 1);
  scores[9].name[sizeof(scores[9].name) - 1] = '\0';
  scores[9].rounds = currRound;
  scores[9].time = totalGameTime;

  // Po to išrušiuojam
  sortScores();
}

void sortScores(){
  for(int i = 0; i < 9; i++){
    for(int j = i + 1; j < 10; j++){
      if(scores[j].rounds > scores[i].rounds || (scores[j].rounds == scores[i].rounds && scores[j].time < scores[i].time)){
        HighScore temp = scores[i];
        scores[i] = scores[j];
        scores[j] = temp;
      }
    }
  }
}

void getScores(){    // Imam rezultatus is EEPROM, jei crc teisingas
  if(!checkCrc()){
    return;
  }
  int address = 2;
  for(int i = 0; i < 10; i++){
    HighScore temp;
    EEPROM.get(address, temp);
    strncpy(scores[i].name, temp.name, 10);
    scores[i].rounds = temp.rounds;
    scores[i].time = temp.time;
    address+= sizeof(HighScore);
  }
}
void writeScores(){                   // Issaugom rezultatus i atminti, ir ju crc
  int address = 2;
  for(int i = 0; i < 10; i++){
    EEPROM.put(address, scores[i]);
    address+= sizeof(HighScore);
  }
  EEPROM.put(0, calculateScoreCrc());
}

uint_fast16_t calculateScoreCrc(){     // Apskaiciuojam CRC issaugotos rezultatu lenteles
  const uint_fast16_t crc_table[16] = {
    0x0000, 0xcc01, 0xd801, 0x1400, 0xf001, 0x3c00, 0x2800, 0xe401,
    0xa001, 0x6c00, 0x7800, 0xb401, 0x5000, 0x9c01, 0x8801, 0x4400
  };
  uint_fast16_t crc = 0;
  int length = 10*sizeof(HighScore) + 2;  // 10 highscores plus pirmi du baitai del crc
  for(int index = 2; index < length;index++){
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
  }
  return crc & 0xffff;
}
bool checkCrc(){    // Patikrinam ar issaugotas crc atitinka apskaiciuotam crc
  uint_fast16_t crc;
  EEPROM.get(0, crc);
  return crc == calculateScoreCrc();
}
void playButtonSequence(){    // Svieciam lempuciu seka pagal dabartini raunda
  delay(flashTime);
  for(int i = 0; i <= currRound; i++){
    flashLED(buttonSequence[i]);
    delay(flashTime);
  }
  currButton = -1;                                          // Del atsargos reset'iname reiksme
  for (int i = 0; i < 4; i++) debounceTimes[i] = millis();  // Ir mygtuku paspaudimo laika perstatome kad butu po grojimo
}


void LightUp(int idx){        // Ijungti lempute
  if(idx >= 2){                // Jei pin'ai su pertraukimais pirmiausiai reikia juos nuimti
    detachInterrupt(digitalPinToInterrupt(lightPins[idx]));
  }
  pinMode(lightPins[idx], OUTPUT);
  digitalWrite(lightPins[idx],HIGH);
}


void LightDown(int idx){      // Isjungti lempute
  digitalWrite(lightPins[idx], LOW);
  pinMode(lightPins[idx], INPUT_PULLUP);
  (void)digitalRead(lightPins[idx]);

  if(idx == 2){               // Grazinam pertraukimus zaliai
    EIFR = (1 << INTF1);      // Panaikinam interrupta jei del lempuciu buvo uzregistruotas
    attachInterrupt(digitalPinToInterrupt(lightPins[idx]), handleGreen,RISING);  }
  if(idx == 3){               // Grazinam pertraukimus melynai
    EIFR = (1 << INTF0);      // Panaikinam interrupta jei del lempuciu buvo uzregistruotas
    attachInterrupt(digitalPinToInterrupt(lightPins[idx]), handleBlue,RISING);
  }
}

void flashLED(int idx){       // Ijungiam ir isjungiam konkrecia lempute
  LightUp(idx);
  delay(flashTime);
  LightDown(idx);
}

void flashAll(){              // Ijungiam ir isjungiam visas lemputes
  for(int i = 0; i < 4; i++){
    LightUp(i);
  }
  delay(flashTime);
  for(int i = 0; i < 4; i++){
    LightDown(i);
  }
  delay(flashTime);
}

void checkButton(){             // Mygtuku tikrinimas
  if(currButton == -1){         // Jei jau nustatytas paspaustas mygtukas netikrinam
    for(int i = 0; i < 3; i++){
      if(digitalRead(lightPins[i]) == HIGH){    // Tikrinam ar ijungta
        unsigned long pressTime = millis();    // Jei taip tikrinam ar praejo pakankamai laiko
        if(pressTime - debounceTimes[i] > debounceDelay){    
          debounceTimes[i] = pressTime;
          currButton = i;       // Jei taip grazinam kad paspaude
          return;
        }
        debounceTimes[i] = pressTime;
      }
    }
  }
}

bool correctButton(){                           // Funkcija patikrinti ar teisingas mygtukas
  if(currButton != -1){                         // Jei nepaspaustas nieko nedarom
    if(currButton == buttonSequence[subRound]){ // Jei teisingas paspaustas
      currButton = -1;                          // reset'iname mygtuka
      if(subRound != currRound){                // Jei nepasiekem galo dabartinio raundo
        subRound++;                             // Padidinam poraundi
      } else{                                   // Jei pasiekem gala raundo
        subRound = -1;                          // Reset'iname poraundi ir padidinam raunda
        currRound++;
        updateScreen = true;
      }
      gameTime = 0;
      return true;
    } else {
        currButton = -1;
        return false;
      }
  }
}

void handleGreen(){         // Zalio mygtuko pertraukimas
  unsigned long intTime = millis();
  if (intTime - debounceTimes[2] > debounceDelay) {
    currButton = 2;
  }
  debounceTimes[2] = intTime;
}
void handleBlue(){          // Melyno mygtuko pertraukimas
  unsigned long intTime = millis();
  if (intTime - debounceTimes[3] > debounceDelay) {
    currButton = 3;
  }
  debounceTimes[3] = intTime;
}