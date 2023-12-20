/* NODE Repeater V1.2
   WENDA YUSUP
   PT.MAKERINDO PRIMA SOLUSI*/

/*------------Source Framework-----------*/
#include <Arduino.h>

/*------------Source Timing-----------*/
unsigned long awal = millis();

/*------------Source LoRa-----------*/
#include <SPI.h>
#include <LoRa.h>
#define SS 18
#define RST 14
#define DIO0 26
#define SCK 5
#define MISO 19
#define MOSI 27
//-----RSSI
int rssi = 0;
//-----sendMessage
String SenderNode = "";
String outgoing;
byte msgCount = 0;
String incoming = "";
//-----CodeMesssage
byte Noderepeater = 0xB0;
byte NodeNetral = 0x00;
byte NodepetikA = 0xA0;

bool node1 = true;
bool node2 = false;
/*------------Source Mac Address-----------*/
#include <esp_system.h>
#include <regex.h>
String macAddress = "";
const int maxindec = 5;
/*------------List Mac-----------*/
const int MAX_ENTRIES = 5;
String entries[MAX_ENTRIES];
int entrycount = 0;

/*------------Source EBYTE-----------*/
HardwareSerial ebyteSerial(2);

void setup()
{
  /*------------PEMBACAAN SERIAL MAIN-----------*/
  Serial.begin(115200);
  /*------------PEMBACAAN SERIAL MAIN-----------*/
  ebyteSerial.begin(9600, SERIAL_8N1, 32, 33);

  /*------------PEMBACAAN MAC ADDRESS-----------*/
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  // Menyimpan MAC address dalam variabel global
  for (int i = 0; i < 6; ++i)
  {
    macAddress += String(mac[i], HEX);
    if (i < 5)
      macAddress += ":";
  }

  /*------------PEMBACAAN LoRa-----------*/
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  LoRa.begin(915E6);
  LoRa.setSpreadingFactor(11);
  LoRa.setCodingRate4(2);
  LoRa.setSignalBandwidth(500E3);
}

bool isMACAddressValidA(const String &mac)
{
  const char *validFormatPatternA = "^([0-9A-Fa-f]{1,2}[:-]){5}([0-9A-Fa-f]{1,2})$";

  regex_t regex;
  regcomp(&regex, validFormatPatternA, REG_EXTENDED);

  int result = regexec(&regex, mac.c_str(), 0, NULL, 0);

  regfree(&regex);

  return result == 0;
}

bool isMACAddressValidB(const String &mac)
{
  const char *validFormatPatternB = "^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$";
  regex_t regex;

  // Compile pola regex
  if (regcomp(&regex, validFormatPatternB, REG_EXTENDED) != 0)
  {
    Serial.println("Error compiling regex");
    return false;
  }

  // Mengecek apakah MAC address sesuai dengan pola regex
  bool isValid = (regexec(&regex, mac.c_str(), 0, NULL, 0) == 0);

  // Bebaskan sumber daya yang digunakan oleh pola regex
  regfree(&regex);

  return isValid;
}
//------------Proc SendMessage-----------//
void sendMessage(String outgoing, byte Noderepeater, byte othernode)
{
  LoRa.beginPacket();            // start packet
  LoRa.write(othernode);         // add destination address
  LoRa.write(Noderepeater);      // add sender address
  LoRa.write(msgCount);          // add message ID
  LoRa.write(outgoing.length()); // add payload length
  LoRa.print(outgoing);          // add payload
  LoRa.endPacket();              // finish packet and send it
  msgCount++;                    // increment message ID
}

void onReceive(int packetSize)
{
  if (packetSize == 0)
    return;
  int recipient = LoRa.read();
  byte sender = LoRa.read();
  byte incomingMsgId = LoRa.read();
  byte incomingLength = LoRa.read();

  while (LoRa.available())
  {
    incoming = LoRa.readStringUntil('\n');
  }
  if (incomingLength != incoming.length())
  {
    Serial.println("error: message length does not match length");
    ;
    return;
  }

  if (recipient != NodepetikA && recipient != Noderepeater)
  {
    // Serial.println("This message is not for me().");
    ;
    return;
  }

  

  if (isMACAddressValidA(incoming) || isMACAddressValidB(incoming))
  {
    Serial.println("MAC Address valid ! ");
    if (entrycount < MAX_ENTRIES)
    {
      incoming.trim();

      if (incoming.length() == 0)
      {
        Serial.println("MAC Address Kosong");
      }
      else
      {
        bool isDuplicate = false;
        int foundindex = -1;

        for (int i = 0; i < entrycount; ++i)

        {
          if (entries[i].equals(incoming))
          {
            isDuplicate = true;
            foundindex = i;
            break;
          }
        }

        if (isDuplicate)
        {
          Serial.print("Mac Address " + incoming + " sudah ada di indeks ke - ");
          Serial.println(foundindex + 1);
          // Pesan yang dikirim untuk validasi bahhwa mac diterima
          String macvalidation = String() + incoming + " " + (foundindex + 1);
          delay(100);
          sendMessage(macvalidation, Noderepeater, NodepetikA);
        }
        else
        {
          entries[entrycount++] = incoming;
          Serial.print(String() + "Mac Address " + incoming + " terdaftar di indeks ke- ");
          Serial.println(entrycount);
          Serial.println("");
          if (entrycount < MAX_ENTRIES)
          {
            Serial.println("Menunggu Mac Address Baru...");
          }
        }
      }
    }
    else
    {
      Serial.println("Daftar Mac Address masuk :");
      for (int i = 0; i < entrycount; ++i)
      {
        Serial.print("Mac Address ke ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(entries[i]);
      }
      delay(1000);
      entrycount = 0;
    }
  }
  else
  {
  }

  if (incoming.startsWith("MPGMBG0823001"))
  {

    if (node1)
    {
      Serial.println(incoming);
      ebyteSerial.println(incoming);
      delay(120);
      String success = "success 1";
      sendMessage(success, Noderepeater, NodepetikA);
      node1 = false;
      node2 = true;
    }
  }
  else  {
    node1 = false;
    node2 = true;
  }
  {
    
  }

  if (incoming.startsWith("MPGMBG0823002"))
  {
    if (node2)
    {
      Serial.println(incoming);
      ebyteSerial.println(incoming);
      delay(200);
      String success = "success 2";
      sendMessage(success, Noderepeater, NodepetikA);
      node1 = true;
      node2 = false;
    }
  }
  else 
  {
    node1 = true;
    node2 = false;
  }
}

void loop()
{

  onReceive(LoRa.parsePacket());
}
