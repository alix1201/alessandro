#define WIFI_SSID "Oppo Reno2" // nome del router
#define WIFI_PASSWORD "27f4859ad10f" // password del router
#define DHTPIN 2     //definisco la porta del sensore DHT11
#define DHTTYPE DHT11   //definisco la tipologia del sensore
#define FIREBASE_HOST1 "compitinogiugno2020.firebaseio.com" //definisco le credenziali di accesso ai database da cui ricevere e inviare i dati
#define FIREBASE_AUTH1 "e0WafHIXO9fJieRHlzvJOUuqklsejdyyHlN3KUvz" 
#define FIREBASE_HOST2 "database-dati-ricevuti.firebaseio.com"
#define FIREBASE_AUTH2 "BJ5YjQ675xVIzXW14otzO6bKP8p3PByeKv5Rg7Dq"
#define SOGLIA 25 //la soglia di tolleranza del pannello
#define LED 13 //porta del led 
#include "FirebaseESP8266.h" //libreria per Firebase
#include <ESP8266WiFi.h> //librerua per il WiFi della scheda
#include <DHT.h> //libreria del sensore
const char *ssidAP = "Ibrido"; // definisco access point
const char *passwordAP = "password";
const char *sole ="sun";  //variabile utile in fase di computazione



FirebaseData firebaseData; //creo l'oggetto FirebaseData

DHT dht(DHTPIN,DHTTYPE); //creo l'oggetto dht 

WiFiServer server(80); //apro la porta del server (di solito la porta 80)

void setup() {
Serial.println("Benvenuto nella stazione di controllo del pannello solare DBPEW8941");
Serial.begin(9600); //apro la porta seriale a 9600 baud rate
dht.begin(); //inizializzo il sensore
pinMode(LED,OUTPUT); //definisco il led come output
  WiFi.mode(WIFI_STA);  // definisco il tipo di connessione(station mode)
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //fornisco le credenziali di accesso
  delay(2000);
  Serial.print("Mi sto connettendo a:  "+String(WIFI_SSID));
  while (WiFi.status() != WL_CONNECTED) {  //finchè non è connesso stampa tanti puntini
    Serial.print(".");
    delay(500);
  }
  Serial.println();
    server.begin(); //inizializzo Server

  Serial.print("Connesso!");
Serial.print("Utilizza questa URL per monitorare in tempo reale tutti i valori letti dal sensore: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP()); //definizione dell'IP locale
  Serial.println("/");
  Serial.println("");
   Serial.print("Configurazione access point..."); //configuro access point
  Serial.println(WiFi.softAP(ssidAP, passwordAP) ? "OK" : "Fallito!");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP); //mio Ip
  uint8_t macAddr[6];
  WiFi.softAPmacAddress(macAddr); //MAC address
  Serial.printf("MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);


  
  
  
  
  delay(100);



  }
    

void loop() {





  
  Firebase.begin(FIREBASE_HOST1, FIREBASE_AUTH1); //Inizializzo il Firebase esterno


 Serial.println("");
delay (1000);
float temp_interna=dht.readTemperature(); //il sensore rileva la temperatura interna al sensore e la stampa su monitor seriale

  if (isnan(temp_interna)) {                                                // Controlla se il sensore DHT11 sta effettivamente funzionando e, se non dovesse funzionare, tiene il LED acceso
    Serial.println("Fallita la lettura dal sensore DHT11!");
    digitalWrite(LED, HIGH);
    delay(1000);
    return;
  }
Serial.println("La temperatura interna è pari a: ");                       
Serial.print (temp_interna); Serial.print ("°C");

if (Firebase.getFloat(firebaseData, "/main/temp")) { //prelevo la temperatura esterna dal database esterno

    firebaseData.floatData();



}

 float temp_esterna=firebaseData.floatData()-273.15; //assegno il valore preso dal database esterno a una variabile float e la converto da Kelvin in gradi centigradi
float media_temperatura=(temp_esterna+temp_interna)/2;    // creo la temperatura media
if (Firebase.getFloat(firebaseData, "/weather/main")) { //prelevo come stringa le attuali condizioni esterne

    firebaseData.stringData();



}

 String condizioni=firebaseData.stringData();
Serial.println("\nLa temperatura esterna è pari a: ");Serial.print (temp_esterna);Serial.print ("°C");
delay(100);
Serial.println("\n\nLe attuali condizioni metereologiche sono:  ");Serial.print (condizioni);
delay(100);
Serial.println("\n\nLa media tra la temperatura esterna e interna al pannello è:  "); Serial.print(media_temperatura);Serial.print ("°C");
delay(5000);
/*In questa parte verifichiamo le condizioni di lampeggio del led tramite  due cicli if*/

if((media_temperatura>SOGLIA)&&(condizioni==sole)) {            
digitalWrite(LED, HIGH); //metti il pin allo stato alto (Led acceso)
}
if (media_temperatura>=SOGLIA){
 digitalWrite(LED, HIGH); // metti il pin  allo stato alto (Led acceso)
  delay(100); // mantieni questo stato per 100 ms
  digitalWrite(LED, LOW); // metti il pin  allo stato basso (Led spento)
  delay(100);
}

else {
  digitalWrite(LED, LOW);   //metti il pin allo stato basso
}
 delay(10000);
//Inizializzo il Firebase verso cui mandare i dati rilevati

Firebase.begin(FIREBASE_HOST2, FIREBASE_AUTH2);
FirebaseJson updateData; // definisco l'oggetto updateData
updateData.set("temperatura_interna", "temp_interna"); //faccio l'update della temperatura interna al pannello
updateData.set("temperatura_esterna","temp_esterna"); //faccio l'update della temperatura esterna al pannello
updateData.set("media_temperatura", "media_temperatura"); //faccio l'update della temperatura media
updateData.set("media_temperatura", "condizioni"); //faccio l'update delle condizioni rilevate 

if(Firebase.updateNode(firebaseData, "/", updateData)){   //verifico se l'update è avvenuto correttamente e stampo un messaggio di errore se ciò non avviene

    Serial.println("\n\n\n\n\n\nCorrect update data");    
  }
  else{ Serial.println(firebaseData.errorReason()); }

{
 /* In questa parte specifico le caratteristiche del webserver*/
 WiFiClient client = server.available(); 
  if (client) // Controllo se un client sta inviando richieste al server
 while (client.available()) 
    {
      if (client.connected()) 
      {
    //definisco le caratteristiche della pagina WEB in HTML    
client.println("HTTP/1.1 200 OK");
client.println("Content-Type: text/html");
client.println("Connection: close");  //la connessione verrà chiusa in automatico
client.println("Refresh: 5");  // ricarico la pagina ogni 5 secondi
client.println();  // obbligatorio
client.println("<!DOCTYPE html>"); //definisco la tipologia 
client.println("<html>");
client.println("<head>\n<meta charset='UTF-8'>"); //specifica la codifica dei caratteri  per HTML
client.println("<title>Stazione di controllo</title>"); //titolo della pagina (ciò che viene mostrato sul browser)
client.println("</head>\n<body ");       
client.print("style=");  client.print("");client.print ("background-color:#FFF380>");  //scelgo il colore dello sfondo
client.println("<H2>Benvenuto nella stazione di controllo del pannello solare DBPEW8941</H2>");
client.println("<H3>Sono mostrate le temperature e le condizioni ambientali monitorate</H3>");
client.println("<pre>"); //lascio il testo preformattato e quindi vado a capo in automatico
client.println("Temperatura esterna:  "); client.print(temp_esterna); client.print(" °C <br>"); //mostro la temperatura esterna al pannello
client.println("Temperatura interna:  "); client.print(temp_interna); client.print(" °C <br>"); //mostro la temperatura interna al pannello
client.println("Temperatura media:  ");client.print(media_temperatura); client.print(" °C <br>"); //mostro la temperatura media
client.println("Attuali condizioni:  ");client.print(condizioni); client.println("<br>");//mostro le attuali condizioni meteo
client.println(" Ecco la nostra attrezzatura di lavoro! ");
client.println("</pre>");
client.println("<img src=\"https://github.com/alix1201/alessandro/blob/master/IMG20200531214449.jpg?raw=true\" /HEIGHT=300 WIDTH=300>"); //inserisco lo sfondo 

client.print("</body>\n</html>"); //chiudo i tag
break;
}


}
}
}
                       
