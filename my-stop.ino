#include <Bridge.h>
#include <Process.h>
#include <HttpClient.h>
#include <Time.h>

// Define api details in a header file
// #define API_URL ""
// #define STOP ""
#include "ApiConstants.h"

String url = String(API_URL) + "?stopCode1=" + String(STOP) + "&ReturnList=LineName,DestinationText,EstimatedTime";
Process p;
String busArrivals[10][2];
int numberOfBuses = 10;

void setup() {
    pinMode(13, OUTPUT);
    Bridge.begin();
    Serial.begin(9600);
    while (!Serial); // wait for a serial connection
}

void loop() {
    boolean busDetailStart = false;
    String busDetail;
    int busSequence = 0;

    // Flash the onboard LED whilst retrieving data
    digitalWrite(13, HIGH);

    unsigned long currentTime = getCurrentTime();

    // Call the api to get the bus times
    HttpClient client;
    client.get(url);

    // Read incoming bytes
    while (client.available()) {
        char c = client.read();

        if (c == '[') {
            busDetailStart = true;
        }

        if (busDetailStart && c != '[')
            if (c == ']') {
                busDetailStart = false;
                String busNumber = getSubstring(busDetail, ',', 1);

                // Ignore header row
                if (busNumber != "1.0") {
                    busSequence++;

                    if (busSequence <= numberOfBuses) {
                        String destination = getSubstring(busDetail, ',', 2);
                        // Rogue forward slash in api response
                        destination.replace('/',' ');

                        String estimatedTime = getSubstring(busDetail, ',', 3);
                        // Remove last 3 digits to convert from ms to secs
                        String t = estimatedTime.substring(0, estimatedTime.length() - 3);
                        char busArrivalTime [20];
                        t.toCharArray(busArrivalTime, 20);
                        int secondsWait = atol(busArrivalTime) - currentTime;

                        // Store in an array for subsequent sorting on wait time
                        busArrivals[busSequence][0] = busNumber + " " + destination + ";" +  displayTime(t);
                        busArrivals[busSequence][1] = String(secondsWait/60);
                    }
                }

                busDetail = "";
        } else {
            busDetail += c;
        }
    }

    sortBusArrivals();
    displayBusArrivals();

    Serial.println("");
    Serial.flush();
    digitalWrite(13, LOW);

    // Query again in 30 seconds
    delay(30000);
}

// Sort bus arrival array on wait time
void sortBusArrivals() {
    for (int x = 0; x < numberOfBuses; x++) {
        for (int y = 0; y < numberOfBuses - 1; y++) {
            if ((busArrivals[y][1].toInt()) > (busArrivals[y + 1][1].toInt())) {
                String busDetails = busArrivals[y + 1][0];
                String waitTime = busArrivals[y + 1][1];

                busArrivals[y + 1][0] = busArrivals[y][0];
                busArrivals[y][0] = busDetails;

                busArrivals[y + 1][1] = busArrivals[y][1];
                busArrivals[y][1] = waitTime;
            }
        }
    }
}

void displayBusArrivals() {
    int busSequence = 0;

    for (int i = 0; i < numberOfBuses; i++) {
        if (busArrivals[i][0].length() > 0) {
            busSequence++;
            Serial.print(busSequence);
            Serial.print(". ");

            String busDetail = busArrivals[i][0];
            Serial.print(getSubstring(busDetail, ';', 0));
            Serial.print(" - ");

            String numberOfMinutes = busArrivals[i][1];
            switch(numberOfMinutes.toInt()) {
                case 0:
                    Serial.print("now");
                    break;
                case 1:
                    Serial.print("1 minute");
                    break;
                default:
                    Serial.print(numberOfMinutes + " minutes");
            }

            Serial.print(" - ETA: ");
            Serial.print(getSubstring(busDetail, ';', 1));
            Serial.println();
        }
    }
}

// Display time as hh:mm:ss
String displayTime(String val) {
    String t1 = "";
    time_t t = val.toInt();

    int h = hour(t);
    if (h < 10)
        t1 = "0";
    t1 = t1 + String(h) + ":";

    int m = minute(t);
    if (m < 10)
        t1 = t1 +  "0";
    t1 = t1 +  String(m) + ":";

    int s = second(t);
    if (s < 10)
        t1 = t1 +  "0";
    t1 = t1 +  String(s);

    return t1;
}

// Extract substring from a string with a given delimiter
String getSubstring(String s, char parser, int index) {
    String rs = "\0";
    int parserIndex = index;
    int parserCnt = 0;
    int rFromIndex = 0, rToIndex = -1;

    while (index >= parserCnt) {
        rFromIndex = rToIndex + 1;
        rToIndex = s.indexOf(parser,rFromIndex);

        if (index == parserCnt) {
            if (rToIndex == 0) {
                return "\0";
            }
            String s2 = s.substring(rFromIndex,rToIndex);
            s2.replace('"',' ');
            s2.trim();
            return s2;
        } else {
            parserCnt++;
        }
    }
    return rs;
}

// Get the time using the Bridge librarys Process class
unsigned long getCurrentTime() {
    Process p1;
    char epochCharArray[12] = "";

    // Get and display time in readable format
    p1.begin("date");
    p1.addParameter("+%T");
    p1.run();
    while (p1.available() > 0) {
        Serial.print("current time: ");
        Serial.println(p1.readString());
    }

    // Get and return UNIX timestamp for calculation of wait
    p1.begin("date");
    p1.addParameter("+%s");
    p1.run();
    while (p1.available() > 0) {
        p1.readString().toCharArray(epochCharArray, 12);
    }

    // Return long with timestamp
    return atol(epochCharArray);
}
