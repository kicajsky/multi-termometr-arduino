#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 3, 4, 5, 6, 7); //info o podl¹czeniu nowego wyœwietlacza
#define LM35 A0 //LM35 do A0
OneWire oneWire(A1); //komunikacja 1-wire do A1 DLA 18B20
#define THERMISTORPIN A2
#define PT100 A3

DallasTemperature sensors(&oneWire); //Przekazania informacji do biblioteki
byte stopnie[8] = {  //konfigurajca znaku stopnie
	0b00100,
	0b01010,
	0b10001,
	0b01010,
	0b00100,
	0b00000,
	0b00000,
	0b00000
};

//zmienne dla DS18B20
float maxtemp0 = 0;  //current max temp
float mintemp0 = 100;  //current min temp
float maxtemp1 = 0;  //current max temp
float mintemp1 = 0;  //current min temp
//zmiene dla termistora
#define NUMSAMPLES 5
#define SERIESRESISTOR 100000   
int samples[NUMSAMPLES];
//zmienne dla LCD
//int jasnosc = 0;
int jasnosc1 = 255;
//int zmiana = 5;
//zmienne dla switcha
volatile int i = 0;
volatile int klik = 0;
unsigned int buttonPin = 2;	//pin posiada przerwania

void setup()
{
	Serial.begin(19200);
	pinMode(buttonPin, INPUT_PULLUP);	//Przycisk
	attachInterrupt(digitalPinToInterrupt(buttonPin), przycisk, FALLING);
	interrupts();	//rozpoczecie przerwan
	maxtemp1 = sensors.getTempCByIndex(0);// DLA 18B20
	mintemp1 = sensors.getTempCByIndex(0);
	sensors.begin(); //Inicjalizacja czujnikow

	analogWrite(9, jasnosc1);
	lcd.begin(16, 2);//deklaracja typu wyswietlacza
	//lcd.createChar(0, stopnie); //stwórz znak o nazwie stopnie i przypisz jej numer 0
	lcd.clear();
	//lcd.setCursor(15, 0);	//AUTOSCROLL
	//lcd.print("Multi Termometr");
	// scroll 16 positions (display length + string length) to the left
	// to move it back to center:
	/*for (int positionCounter = 0; positionCounter < 15; positionCounter++)
	{
		// scroll one position left:
		lcd.scrollDisplayLeft();
		// wait a bit:
		delay(150);
	}
	lcd.setCursor(0, 1);
	lcd.print("Multi Termometr");
	for (int positionCounter = 0; positionCounter < 16; positionCounter++)
	{
		// scroll one position right:
		lcd.scrollDisplayRight();
		// wait a bit:
		delay(150);
	}*/
	analogReference(EXTERNAL);
}


void przycisk()	//obsluga przerwan
{
	noInterrupts();
	delay(1);
	klik++;
	interrupts();
}

void loop()
{
	float temp0 = ((analogRead(LM35) * 5.0) / 1024.0) * 100;  //converting ADC reading to temp

	float temp1 = sensors.getTempCByIndex(0); //zapis temperatury do zmiennej dla 18B20
	sensors.requestTemperatures();	//Pobranie temperatury czujnika 18B20
	//przeliczanie dla termisotra
	uint8_t i;
	float average;

	// take N samples in a row, with a slight delay
	for (i = 0; i< NUMSAMPLES; i++) {
		samples[i] = analogRead(THERMISTORPIN);
		delay(10);
	}

	// average all the samples out
	average = 0;
	for (i = 0; i< NUMSAMPLES; i++) {
		average += samples[i];
	}
	average /= NUMSAMPLES;

	/*	PT100
	float temp3 = ((analogRead(PT100) * 5.0) / 1024.0); //zapis temperatury do zmiennej dla Pt100
	float Rpt = ((97.6 * temp3) / (4.85 - temp3));	//przeliczanie rezystancji z dzielnika napiecia
	float PT = (Rpt - 100) / 0.3885;	//przeliczanie rezystancji na temperature DLA PT100
	*/
	float diff = temp0 - temp1; //absolute difference
	float diff1 = temp1 - temp0;

	Serial.print("Average analog reading ");
	Serial.println(average);
	// convert the value to resistance
	average = 1023 / average - 1;
	average = SERIESRESISTOR / average;

	Serial.print("Thermistor resistance ");
	Serial.println(average);
	//Wyslanie przez UART temperatury MAX i MIN DLA LM35
	/*if (temp0 > maxtemp0) //Jesli aktualna temperatura jest wyzsza od maksymalnej
	{
		maxtemp0 = temp0; //Pobranie temperatury czujnika, ustawienie jako max
		Serial.print("MAX0: ");
		Serial.print(maxtemp0);
		Serial.println("*C, ");
	}
	if (temp0 < mintemp0)
	{
		mintemp0 = temp0;  //assign current min value
		Serial.print("MIN0: ");
		Serial.print(mintemp0);
		Serial.println("*C");
	}
	//Wyslanie przez UART temperatury MAX i MIN DLA DS18
	if (temp1 > maxtemp1)
	{
		maxtemp1 = temp1;
		Serial.print("MAX1: ");
		Serial.print(maxtemp1);
		Serial.println("*C");
	}
	if (temp1 < mintemp1)
	{
		mintemp1 = temp1;
		Serial.print("MIN1: ");
		Serial.print(mintemp1);
		Serial.println("*C");
	}
	*/

	if (klik == 0)
	{
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("LM35:  ");
		lcd.print(temp0);
		lcd.print("\xDF""C");
		lcd.setCursor(0, 1);
		lcd.print("18B20: ");
		lcd.print(temp1);
		lcd.print("\xDF""C");
	}
	if (klik == 1)
	{
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Termistor:");
		lcd.print(average);
		lcd.print("\xDF""C");
		lcd.setCursor(0, 1);
		lcd.print("PT100:  NONE");
		//lcd.print(PT);
		//lcd.print("\xDF""C");
	}
	if (klik == 2)
	{
		lcd.clear();
		lcd.print("LM35 vs DS18b20");
		lcd.setCursor(0, 1);
		lcd.print("Roznica:  ");
		lcd.print(abs(diff));
		lcd.print("\xDF""C");
	}
	if (klik == 3)
	{
		lcd.clear();
		lcd.print("DS18 vs termist.");
		lcd.setCursor(0, 1);
		lcd.print("Roznica:  ");
		lcd.print(abs(diff1));
		lcd.print("\xDF""C");
	}
	if (klik == 4)
	{
		lcd.clear();
		lcd.print("tadam..");
	}
	if (klik > 4)
	{
		klik = 0;
	}
}