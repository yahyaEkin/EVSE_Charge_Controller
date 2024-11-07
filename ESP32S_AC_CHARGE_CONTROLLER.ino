/*  EVSE Charger Yöntem 4 Deneme 2
    26.08.2023
    Pazartesi

    Bu yöntemde komparatörler ile oluşan sinyalin eşik değerlerini geçip geçmediği
    karşılaştırılmış, analıg bir voltaj okuması yerine üretilen sinyalin high ve low
    geçiş zamanları üzerinden durum geçişleri yapılmıştır.

*/
#define STATE_A_LED 25              // Kırmızı
#define STATE_B_LED 26              // Sarı
#define STATE_C_LED 27              // Yeşil
#define OSCILLATOR_PIN 4            //Control Pilot üretilen pin
#define VEHICLE_CONNECTED 32        //Komparatör 1 çıkışını okur
#define CHARGE_REQUEST 35           //Komparatör 2 çıkışını okur
#define SET_CURRENT_ADC 36
#define RELAY_PIN 16

int selected_current;

unsigned long vehicle_connected_low =0;
unsigned long vehicle_connected_high=0; 
unsigned long charge_request_low =0;
unsigned long charge_request_high=0; 

// State Geçişi değişkenleri --------
bool vehicle_is_connected = false;   //araç bağlı mı ?
bool charge_is_requested = false;    //Şarj istiyor mu ?
bool vehicle_is_connected_state_a = false; //araç ilk state A'da bağlı mı?

/* NOT: Yukarıdaki değişken şu sebeple gereklidir:
  Araç state A da analogWrite(OSCILLATOR_PIN, 255) ile DC 12V'luk bir sinyal 
  oluşturur. Bu sinyal kare dalga olmadığından dolayı komparatörlerden geçse 
  bile high_to_low yada low_to_high zamanı 0us çıkar çünkü DC bir sinyaldir,
  geçiş yapmaz. Bundan dolayı state A da bu pinin bu sefer digitalRead() olarak
  okunması gereklidir. Osiloskop ile görülen değer şu şekildedir:
  
  Araç Bağlı Değil -> Komparatör çıkışında 2.92V sabit bir sinyal var.
  Araç Baplı       -> Komparatör çıkışında 0V sabit bir sinyal var.

*/
// ----------------------------------

// State Geçişi için debounce uygulamaı--
#define DEBOUNCE_DELAY 200  // 200ms debounce süresi
unsigned long lastDebounceTime = 0;
unsigned long currentTime;
//--------------------------------------

// LCD Ekran ve Yazdırma ------------
#include <LiquidCrystal_I2C.h> 
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x3F for a 16 chars and 2 line display 

String lastVehicleStatus = "";
String lastChargeStatus = "";
int lastMaxCurrent = 0;
//-----------------------------------

// RFID Okuyucu ---------------------
#include <SPI.h>
#include <MFRC522.h>
 
#define SS_PIN 5
#define RST_PIN 14
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
int rfid_check = 0;
bool rfid_read_successful = false;

// State Machine --------------------
enum State {
  STATE_A,
  STATE_B,
  STATE_C
};

State currentState = STATE_A;
//-----------------------------------


// Debug Amaçlı Değişkenler ---------
bool state_A = false;
bool state_B = false;
bool state_c = false;

//-----------------------------------
// FONKSİYONLAR ------------------------------------------------------------------

int calculate_duty_cycle(float available_current){
  float duty_cycle;

  if (available_current>= 6 && available_current < 51){
    duty_cycle = available_current / 0.6;
  }
  else if (available_current >= 51 && available_current <= 80){
    duty_cycle = (available_current / 0.6) + 64;
  }
  else {
    Serial.println("Akım şarj için çok düşük! Akım < 6A.");
    Serial.println("Dijital Haberleşme Duty Cycle'ı: %5.");
    duty_cycle = 5;
  }

  //Yüzdelik duty cycle'ı analogWrite() için 0 ile 255 arasına oranlama
  int analogwrite_value = (int)(duty_cycle * 255) / 100;
  return analogwrite_value;
}

int set_available_current(int ADC_PIN)
{
  int available_current;
  available_current = map(analogRead(ADC_PIN), 0, 4096, 6, 51);
  return available_current;
}

void LCD_Print(int state, int max_current, int arac_durumu, int sarj_isteme) {
  String currentVehicleStatus = arac_durumu ? "Bagli" : "Bagli Degil";
  String currentChargeStatus = sarj_isteme ? "Istiyor" : "Istemiyor";

  lcd.setCursor(0, 0);
  lcd.print("Yahya Sarj Istasyon");

  lcd.setCursor(0,1);
  lcd.print("Arac: ");
  // Update vehicle connection status only if it has changed
  if (currentVehicleStatus != lastVehicleStatus) {
    lcd.setCursor(6, 1);
    lcd.print("              "); // Clear previous status
    lcd.setCursor(6, 1);
    lcd.print(currentVehicleStatus);
    lastVehicleStatus = currentVehicleStatus;
  }
  lcd.setCursor(0,2);
  lcd.print("Sarj: ");
  // Update charging request status only if it has changed
  if (currentChargeStatus != lastChargeStatus) {
    lcd.setCursor(6, 2);
    lcd.print("              "); // Clear previous status
    lcd.setCursor(6, 2);
    lcd.print(currentChargeStatus);
    lastChargeStatus = currentChargeStatus;
  }
  lcd.setCursor(0,3);
  lcd.print("Maksimum Akim: ");
  // Update max current only if it has changed
  if (max_current != lastMaxCurrent) {
    lcd.setCursor(15, 3);
    lcd.print("    "); // Clear previous value
    lcd.setCursor(15, 3);
    lcd.print(max_current);
    lcd.setCursor(17,3);
    lcd.print("A");
    lastMaxCurrent = max_current;
  }
}
/*
int selected_current(int current)
{
  int selected_current;
  if (current == 6)
    selected_current = 0;
  else if(current == 10)
    selected_current = 1;
  else if (current == 16)
    selected_current = 2;
  else if(current == 32)
    selected_current = 3;
  else
    Serial.println("INVALID CURRENT!");

  return selected_current;
}
*/

int RFID_READ()
{
  int rfid_check;
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "C3 33 84 28") //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println();
    rfid_check = 1;
    delay(3000);
  }
 
 else   {
    Serial.println(" Access denied");
    delay(3000);
    rfid_check = 0;
  }

  return rfid_check;

}
// FONKSİYONLAR BİTİŞ ------------------------------------------------------------------

void setup() 
{
  Serial.begin(9600);
  pinMode(STATE_A_LED, OUTPUT);
  pinMode(STATE_B_LED, OUTPUT);
  pinMode(STATE_C_LED, OUTPUT);
  pinMode(OSCILLATOR_PIN, OUTPUT);        // Control Pilot üretilen pin
  pinMode(CHARGE_REQUEST, INPUT);         // Komparatör 2 çıkışını okur
  pinMode(VEHICLE_CONNECTED, INPUT);      // Komparatör 1 çıkışını okur
  pinMode(RELAY_PIN, OUTPUT);

  lcd.init();
  lcd.clear();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("     Yahya ARGE    ");
  lcd.setCursor(0, 1);
  lcd.print("  AC Sarj Istasyonu ");
  lcd.setCursor(0,2);
  lcd.print("....Baslatiliyor....");

  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  delay(4);

  analogWrite(OSCILLATOR_PIN, 255);
  digitalWrite(RELAY_PIN, LOW);
  delay(3000);
  lcd.clear();
}

void loop() 
{
  
  

  selected_current = set_available_current(SET_CURRENT_ADC);
  Serial.printf("Selected Current: %d\n",selected_current);
  Serial.println();
  // Debug Seri Monitör Ekranı
  Serial.print("vehicle_is_connected: ");
  Serial.print(vehicle_is_connected);
  Serial.print(" | charge is requested: ");
  Serial.print(charge_is_requested);
  Serial.print(" | vehicle_is_connected_state_a: ");
  Serial.println(vehicle_is_connected_state_a);

  Serial.print("Şarj İsteği LOW to HIGH zamanı : ");
  Serial.print(charge_request_low);
  Serial.print("us | Şarj İsteği HIGH to LOW zamanı ");
  Serial.print(charge_request_high);
  Serial.print("us | Araç Bağlı LOW to HIGH zamanı: ");
  Serial.print(vehicle_connected_low);
  Serial.print("us | Araç Bağlı HIGH to LOW zamanı: ");
  Serial.print(vehicle_connected_high);
  Serial.println("us");
  delay(500);



  switch(currentState) 
  {
    case STATE_A:
      digitalWrite(STATE_A_LED, HIGH);
      digitalWrite(STATE_B_LED, LOW);
      digitalWrite(STATE_C_LED, LOW);
      Serial.println("STATE_A: Araç bağlı değil. Araç şuan güç istemiyor.");
      LCD_Print(1, selected_current, 0, 0);
      // Oscillatörden DC 12V üretelim.
      analogWrite(OSCILLATOR_PIN, 255);
      delay(100);
      digitalWrite(RELAY_PIN, LOW);
      
 
      if (digitalRead(VEHICLE_CONNECTED) == 0)
      {
        vehicle_is_connected_state_a = true;
      }
      else
      vehicle_is_connected_state_a = false;
  

      if (vehicle_is_connected_state_a == true)
      {
        currentState = STATE_B;
      }
      else
        currentState = STATE_A;

      rfid_read_successful = false;
    break;

    case STATE_B:
      digitalWrite(STATE_A_LED, LOW);
      digitalWrite(STATE_B_LED, HIGH);
      digitalWrite(STATE_C_LED, LOW);
      Serial.println("STATE_B: Araç bağlandı. Araç şuan güç istemiyor.");
      LCD_Print(1, selected_current, 1, 0);

      analogWrite(OSCILLATOR_PIN, calculate_duty_cycle(selected_current));
      digitalWrite(RELAY_PIN, LOW);

      vehicle_connected_low = pulseIn(VEHICLE_CONNECTED, LOW, 5000);
      vehicle_connected_high = pulseIn(VEHICLE_CONNECTED, HIGH, 5000);
      charge_request_low = pulseIn(CHARGE_REQUEST, LOW, 5000);
      charge_request_high = pulseIn(CHARGE_REQUEST, HIGH, 5000);

      if (vehicle_connected_low >= 0) 
      {
        delay(100);
        // Check again after delay
        vehicle_connected_low = pulseIn(VEHICLE_CONNECTED, LOW, 5000);
            if (vehicle_connected_low > 10) {
              vehicle_is_connected = true;
              
              // Check if charge is requested
              if (charge_request_low > 10) {
                charge_is_requested = true;
              } else {
                charge_is_requested = false;
              }
            } 
            else 
            {
              vehicle_is_connected = false;
            }
      } 
      else 
      {
        vehicle_is_connected = false;
        charge_is_requested = false;
      }


      // Durum kontrolü
      if (vehicle_is_connected == false) {
        currentState = STATE_A;
      } else if (charge_is_requested == true) {
        currentState = STATE_C;
      }

      rfid_read_successful = false;
        
    break;
    case STATE_C:
      digitalWrite(STATE_A_LED, LOW);
      digitalWrite(STATE_B_LED, LOW);
      digitalWrite(STATE_C_LED, HIGH);
      
      Serial.println("STATE_C: Araç bağlı. Araç güç istiyor.");

      LCD_Print(1, selected_current, 1, 1);
      analogWrite(OSCILLATOR_PIN, calculate_duty_cycle(selected_current));
      delay(100);

      

      vehicle_connected_low = pulseIn(VEHICLE_CONNECTED, LOW, 5000);
      vehicle_connected_high = pulseIn(VEHICLE_CONNECTED, HIGH, 5000);
      charge_request_low = pulseIn(CHARGE_REQUEST, LOW, 5000);
      charge_request_high = pulseIn(CHARGE_REQUEST, HIGH, 5000);

      if (vehicle_connected_low >= 0) 
      {
        delay(100);
        // Check again after delay
        vehicle_connected_low = pulseIn(VEHICLE_CONNECTED, LOW, 5000);
            if (vehicle_connected_low > 10) {
              vehicle_is_connected = true;
              
              // Check if charge is requested
              if (charge_request_low > 10) {
                charge_is_requested = true;
              } else {
                charge_is_requested = false;
              }
            } 
            else 
            {
              vehicle_is_connected = false;
            }
      } 
      else 
      {
        vehicle_is_connected = false;
        charge_is_requested = false;
      }
      // Non-blocking zamanlama için millis() kullanımı
      currentTime = millis();
      if (currentTime - lastDebounceTime > DEBOUNCE_DELAY) {
        
        // Durum kontrolü
        if (vehicle_is_connected == false) {
          currentState = STATE_A;
        } else if (charge_is_requested == false) {
          currentState = STATE_B;
        }
        
        lastDebounceTime = currentTime;  // Son debounce zamanını güncelle
      }

      /*
      if (!rfid_read_successful) 
      {
        rfid_check = RFID_READ();
        if (rfid_check == 1) {
            digitalWrite(RELAY_PIN, HIGH);
            Serial.println("RFID Okundu, güç veriliyor");
            rfid_read_successful = true; // Set the flag to indicate RFID was read successfully
        }
      }
      */
      digitalWrite(RELAY_PIN, HIGH);

      break;
  }

  

}
