#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#include <time.h>
#include <iostream>
#include "net_com.h"
#include <curses.h>
#include <wiringPi.h>
//#include "StepperMotor.h"

#define PORT 7 // Ethernet port
#define MAXLINE 1024
#define BUFLEN 512 // Max length of buffer+
#define BUTTON 26  // Button port -> muss noch angepasst werden


using namespace std;


// Data package send in one frame
struct sensor_data
{
	uint32_t counter; 
	uint8_t id;
	uint32_t timestamp;
	float sensor1;
	float sensor2;
	float sensor3;
	float sensor4;
	float sensor5;
	int32_t sensor6;
	int32_t sensor7;
	float temp1;
	float temp2;
	float temp3;
	float temp4;
	float temp5;
	int32_t temp6;
	int32_t temp7;
};

/* void set_StepperMotor(int step, float temp_A, float temp_S)
{
	// RPi GPIO | WiringPi
	// -------------------
	// GPIO 17  |    0
	// GPIO 18  |    1
	// GPIO 27  |    2
	// GPIO 22  |    3
	sm.setGPIOutputs(0, 1, 2, 3);

	// NOTE: Before starting, the current position of the
	// stepper motor corresponds to 0 degrees

	// Rotate of 90 degrees clockwise at 100% of speed
	sm.run(1, step * 90, 100); // direction, angle, speed
}*/

// Sleep Funktion für Linux System - angegeben in Nanosek.
/* void mysleep_ms(int milisec)
{
	struct timespec res;
	res.tv_sec = milisec/1000;
	res.tv_nsec = (milisec*1000000) % 1000000000;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &res, NULL);
}*/

float setSchiebewinkel(float Schiebewinkel)
{
	printf("Bisheriger Schiebewinkel: %f \n", Schiebewinkel);
	printf("Neuen Schiebewinkel eingeben: ");
	scanf("%f", &Schiebewinkel);
	return Schiebewinkel;
}

// Read Sensordata and writes data in CSV file
void readValues(Net_com *net, int counter, float temp_A, float temp_S)
{
	struct sensor_data rx_data;
	int temp_timestamp; // Hilfsvariable zur Berechnung der Latenz zw. zwei Datenpaketen
	char buffer[50];	// buffer to store file name

	FILE *file_temp;							// create file pointer
	sprintf(buffer, "%d_Messung.csv", counter); // create file name with counter included

	// checks if file name is already used
	if (access(buffer, F_OK) == 0)
	{
		printf("File mit diesem Namen existiert bereits.\n");
		exit(1);
	}

	file_temp = (fopen(buffer, "w+"));

	// checks if file was created successfully
	if (file_temp == NULL)
	{
		printf("File konnte nicht erstellt werden.\n");
		exit(1);
	}
	else
	{
		// Header for CSV file: Anstell- & Schiebewinkel; column header
		fprintf(file_temp, "Anstellwinkel: %f - Schiebewinkel %f\n", temp_A, temp_S);
		fprintf(file_temp, "Counter; Timestamp; ID; Latency; Pressure 1; Pressure 2; Pressure 3; Pressure 4; Pressure 5; Pressure 6; Pressure 7; Temperature 1; Temperature 2; Temperature 3; Temperature 4; Temperature 5; Temperature 6; Temperature 7\n");
		printf("Datei wurde erfolgreich erstellt. \n");
	}

	printf("Übertragung gestartet.");
	fflush(stdout);
	// sleep(30); // Wartet 30 Sek. damit sich Luftstrom stabilisieren kann

	for (int i = 0; i < 50; i++) // liest ca. 180 Datensätze pro Minute ein mit einer Sleep-Dauer von 20ms
	{
		int rec_values = net->net_com_receive(&rx_data, sizeof(struct sensor_data));

		if (rec_values > 0) // if server receives data r > 0
		{
			// write data in file
			temp_timestamp = rx_data.timestamp - temp_timestamp; // caluclates latency = difference between data packages
			fprintf(file_temp, "\n %i; %i; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i \n", rx_data.counter, rx_data.timestamp, rx_data.id, temp_timestamp, rx_data.sensor1, rx_data.sensor2, \
			rx_data.sensor3, rx_data.sensor4, rx_data.sensor5, rx_data.sensor6, rx_data.sensor7, rx_data.temp1, rx_data.temp2, rx_data.temp3, rx_data.temp4, rx_data.temp5, rx_data.temp6, rx_data.temp7);

			// print in console
			// printf("\n %d; %d; %d; %.2f; %.2f; %.2f \n", rx_data.timestamp, rx_data.id, temp_timestamp, rx_data.sensor1, rx_data.sensor2, rx_data.sensor3);
			temp_timestamp = rx_data.timestamp; // reset temp_timestamp to timestemp of recent data package

			// Pause programm
			fflush(stdout); // flushed Outputstream bevor System schläft - notwendig vor allem wenn Daten auf Konsole ausgegeben werden
			usleep(20);		// in Millisekunden -> kleineres Intervall wählen??
		}
	}

	fclose(file_temp);
	printf("Daten wurden erforlgreich gespeichert.");
}

int main(void)
{
	Net_com net(7, "192.168.0.5", "192.168.0.3"); // Port, Server address, Cient address - net = Datenübertragung

	net.net_com_connect();

	// wiringPi initialization
	// wiringPiSetup();

	float Schiebewinkel = 0;	// von -18° bis 18°
	float Anstellwinkel = -6.0; // von -6° bis 15°
	int counter = 0;			// Anzahl der Messungen
	int step = 1;				// Traversor steps

	while (true)
	{
		cout << "Wähle:" << endl;
		cout << "1: Veränderung des Schiebewinkels." << endl;
		cout << "2: Anstellwinkel um 1° verschieben." << endl;
		cout << "3: Messung starten." << endl; // kann manuell aufgerufen werden oder integriert in 2

		char input;
		scanf(" %c", &input);

		switch (input)
		{
		case '1':
			Schiebewinkel = setSchiebewinkel(Schiebewinkel);
			printf("Gesetzter Schiebewinkel: %f", Schiebewinkel);
			break;
		case '2':
			// set_StepperMotor(step, Anstellwinkel, Schiebewinkel);
			break;
		case '3':
			for(Anstellwinkel; Anstellwinkel < 16; Anstellwinkel++){
				counter++;
				printf("%d Messung. \n", counter);
				readValues(&net, counter, Anstellwinkel, Schiebewinkel);
			}
			
			break;
		default:
			printf("Ungültige Eingabe.");
			break;
		}
	}

	return 0;
}
