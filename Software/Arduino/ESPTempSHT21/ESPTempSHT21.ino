//#define DEBUG
 
#ifdef DEBUG
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINTDEC(x)  Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)   Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x) 
#endif


#define DEF_SHT21
//#define DEF_DHT

 
 
//----------------------------------------------------------------------------//
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

struct SConnection
{
  const char* ssid;
  const char* password;
};

#define MAX_CONNECTION 3
SConnection m_sConnections[MAX_CONNECTION] = {{"ssid1", "password1"}, {"ssid2", "password2"}, {"ssid3", "password3"}};

const char* m_host = "www.neco.cz";
const int m_httpPort = 80;

unsigned long m_ulSernsor = 4;

#define SLEEP_S 300
#define SLEEP_LONG_S 3600


//----------------------------------------------------------------------------//
float m_fTemp = 0;
float m_fHum = 0;
float m_fVcc = 0.0;

//----------------------------------------------------------------------------//
#ifdef DEF_DHT
#include <PietteTech_DHT.h>
#define DHTTYPE  DHT22       // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   2           // Digital pin for communications
void dht_wrapper(); // must be declared before the lib initialization
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);
void dht_wrapper()
{
  DHT.isrCallback();
}
#endif

#ifdef DEF_SHT21
#include <SHT21.h>  // include SHT21 library
//#include <SparkFunHTU21D.h>  // include SHT21 library

SHT21 m_sht21; 
//HTU21D m_sht21;
#endif

//----------------------------------------------------------------------------//
void GoSleep(int uiSleepS = SLEEP_S)
{
	DEBUG_PRINT("Sleep to: ");
	DEBUG_PRINTDEC(uiSleepS);
	DEBUG_PRINTLN("s");
	ESP.deepSleep(uiSleepS * 1000000UL, WAKE_RF_DEFAULT);
 delay(50000);
}

//----------------------------------------------------------------------------//
float GetVcc()
{
	unsigned int raw = 0;
	pinMode(A0, INPUT);
	raw = analogRead(A0);
	m_fVcc = raw / 1023.0;
	m_fVcc = m_fVcc * 4.275;

  DEBUG_PRINT("Vcc = ");
  DEBUG_PRINTDEC(m_fVcc);
  DEBUG_PRINTLN("V");
  
  return m_fVcc;
}

#ifdef DEF_DHT
//----------------------------------------------------------------------------//
bool DHTRead()
{
  int i = 0;
  delay(800);

  int result = DHT.acquireAndWait(100);  
  do
  {
    delay(100);
    result = DHT.acquireAndWait(100);
  	switch (result)
  	{
  		case DHTLIB_OK:
  			DEBUG_PRINTLN("DHT OK");
  			break;
  		case DHTLIB_ERROR_CHECKSUM:
  			DEBUG_PRINTLN("DHT Error - Checksum error");
  			break;
  		case DHTLIB_ERROR_ISR_TIMEOUT:
  			DEBUG_PRINTLN("DHT Error - ISR time out error");
  			break;
  		case DHTLIB_ERROR_RESPONSE_TIMEOUT:
  			DEBUG_PRINTLN("DHT Error - Response time out error");
  			break;
  		case DHTLIB_ERROR_DATA_TIMEOUT:
  			DEBUG_PRINTLN("DHT Error - tData time out error");
  			break;
  		case DHTLIB_ERROR_ACQUIRING:
  			DEBUG_PRINTLN("DHT Error - Acquiring");
  			break;
  		case DHTLIB_ERROR_DELTA:
  			DEBUG_PRINTLN("DHT Error - Delta time to small");
  			break;
  		case DHTLIB_ERROR_NOTSTARTED:
  			DEBUG_PRINTLN("DHT Error - Not started");
  			break;
  		default:
  			DEBUG_PRINTLN("DHT Unknown error");
  			break;
  	}
    i++;
  }
  while (result != DHTLIB_OK && i < 20);
	return result == DHTLIB_OK;
}
#endif

//----------------------------------------------------------------------------//
bool RefreshValues()
{
  bool bRet = false;
	m_fVcc = GetVcc();
  m_fTemp = -99;
  m_fHum = -99;

#ifdef DEF_DHT
	if (DHTRead())
	{
		m_fTemp = DHT.getCelsius();
		m_fHum = DHT.getHumidity();
    bRet = true;
	}
#endif

#ifdef DEF_SHT21
  DEBUG_PRINTLN("read DEF_SHT21");
  m_fTemp = m_sht21.getTemperature();  // get temp from SHT 
  m_fHum = m_sht21.getHumidity(); // get temp from SHT

  //m_fHum = m_sht21.readHumidity();
  //m_fTemp = m_sht21.readTemperature();
  DEBUG_PRINTLN("end read DEF_SHT21");

  bRet = true;
#endif
 
  return bRet;
}

//----------------------------------------------------------------------------//
void Send2Web()
{
	WiFiClient client;
	if (!client.connect(m_host, m_httpPort))
	{
		DEBUG_PRINT("Connection to ");
		DEBUG_PRINT(m_host);
		DEBUG_PRINTLN(" failed");
		return;
	}
	RefreshValues();
	String sGet = "http://";
	sGet += m_host;
	sGet +="/temp_add.php?sens=";
	sGet += m_ulSernsor;
	sGet += "&temp=";
	sGet += String(m_fTemp, 2);
	sGet += "&hum=";
	sGet += String((int)(m_fHum + 0.5));
	sGet += "&vcc=";
	sGet += String(m_fVcc, 2);
  sGet += "&dBm=";
  sGet += String(WiFi.RSSI());
	DEBUG_PRINT("HTTP GET: ");
	DEBUG_PRINTLN(sGet);
	
	HTTPClient http;
	http.begin(sGet); //HTTP
	int httpCode = http.GET();
  if (httpCode)
  {
  
  }
	http.end();
}

//----------------------------------------------------------------------------//
bool Connection()
{
	DEBUG_PRINTLN("Connecting");
	int i = 0;
	while (WiFi.status() != WL_CONNECTED && i < 100)	// 20s timeout
	{
		delay(200);
		DEBUG_PRINT(".");
		i++;
	}
  int iConnection = 0;
	while (WiFi.status() != WL_CONNECTED && iConnection < MAX_CONNECTION)
	{ 
    ESP.eraseConfig();
    DEBUG_PRINTLN(""); 
    DEBUG_PRINT("WiFi.begin to "); 
    DEBUG_PRINTLN(m_sConnections[iConnection].ssid); 
    //WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.begin(m_sConnections[iConnection].ssid, m_sConnections[iConnection].password);
		i = 0;
		while (WiFi.status() != WL_CONNECTED && i < 100)// 20s timeout
		{
			delay(200);
			DEBUG_PRINT("-");
			i++;
		}
   iConnection++;
	}
	DEBUG_PRINTLN("");
	if (WiFi.status() == WL_CONNECTED)
	{ 
		DEBUG_PRINT("WiFi connected: ");  
		DEBUG_PRINT(WiFi.localIP());
    DEBUG_PRINT(" (");
    DEBUG_PRINTDEC(WiFi.RSSI());
    DEBUG_PRINTLN("dBm)");
		return true;
	}
	else
	{ 
		DEBUG_PRINT("WiFi failed: ");  
		DEBUG_PRINTLN(WiFi.status());
		return false;
	}
}

//----------------------------------------------------------------------------//
void setup()
{
#ifdef DEF_DHT
  DHT.reset();
#endif

#ifdef DEF_SHT21
  Wire.begin(4, 5);
  //m_sht21.begin();
  
#endif

	Serial.begin(115200);
	delay(10);
	DEBUG_PRINTLN("");

  if (GetVcc() < 2.8)
    GoSleep(SLEEP_LONG_S);

	if (Connection() == false)
		GoSleep(SLEEP_LONG_S);
	
	Send2Web();
	GoSleep();
}

//----------------------------------------------------------------------------//
void loop()
{
  DEBUG_PRINTLN("Looooooooooooooooop neeeeeeeeee");
  GoSleep();
}
