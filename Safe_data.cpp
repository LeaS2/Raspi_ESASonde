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
//#include <wiringPi.h>

#define PORT 7 // Ethernet port
#define MAXLINE 1024
#define BUFLEN 512 // Max length of buffer
#define pulse 0
#define direction 1
#define enable 2
#define AMOUNT_OFFSETS 1000

using namespace std;

// Data package send in one frame
struct sensor_data
{
	uint32_t counter; // counter variable zur Überprüfung, ob Datenpakete vollständig & in richtiger Reihenfolge ankommen
	uint8_t id;
	uint32_t timestamp; // Systemzeit des Microcontrollers
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

double offset_p1 = 0;
double offset_p2 = 0;
double offset_p3 = 0;
double offset_p4 = 0;
double offset_p5 = 0;

float setSchiebewinkel(float Schiebewinkel)
{
	printf("Bisheriger Schiebewinkel: %f \n", Schiebewinkel);
	printf("Neuen Schiebewinkel eingeben: ");
	scanf("%f", &Schiebewinkel);
	return Schiebewinkel;
}

void calculateOffsets(Net_com *net)
{
	struct sensor_data rx_data;

	for (int i = 0; i < AMOUNT_OFFSETS; i++)
	{
		int rec_values = 0;
		do
		{
			rec_values = net->net_com_receive(&rx_data, sizeof(struct sensor_data));
			usleep(10);
		} while (rec_values == 0);

		// print in console
		printf("\n %i; %i; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i \n", rx_data.counter, rx_data.timestamp, rx_data.id, latency, rx_data.sensor1, rx_data.sensor2,
			   rx_data.sensor3, rx_data.sensor4, rx_data.sensor5, rx_data.sensor6, rx_data.sensor7, rx_data.temp1, rx_data.temp2, rx_data.temp3, rx_data.temp4, rx_data.temp5, rx_data.temp6, rx_data.temp7);

		latency = rx_data.timestamp; // reset temp_timestamp to timestemp of recent data package
		offset_p1 = +rx_data.sensor1;
		offset_p2 = +rx_data.sensor2;
		offset_p3 = +rx_data.sensor3;
		offset_p4 = +rx_data.sensor4;
		offset_p5 = +rx_data.sensor5;
		// Pause programm
		fflush(stdout); // flushed Outputstream bevor System schläft - notwendig vor allem wenn Daten auf Konsole ausgegeben werden
	}
	offset_p1 = / AMOUNT_OFFSETS;
	offset_p2 = / AMOUNT_OFFSETS;
	offset_p3 = / AMOUNT_OFFSETS;
	offset_p4 = / AMOUNT_OFFSETS;
	offset_p5 = / AMOUNT_OFFSETS;
	printf("Offsets ready!\n");
}

// Read Sensordata and writes data in CSV file
void readValues(Net_com *net, int counter, float temp_A, float temp_S)
{
	struct sensor_data rx_data;
	int latency;	 // Hilfsvariable zur Berechnung der Latenz zw. zwei Datenpaketen
	char buffer[50]; // buffer to store file name

	// Date & Time
	time_t timer;
	char buffer_date[26];
	char buffer_time[26];
	struct tm *tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	strftime(buffer_date, 26, "Date: %d.%m.%Y", tm_info);
	strftime(buffer_time, 26, "Time: %H:%M:%S", tm_info);

	FILE *file_temp;								 // create file pointer
	FILE *file_kalib;								 // create file pointer
	sprintf(buffer, "%d_Messung.csv", counter);		 // create file name with counter included
	sprintf(buffer, "%d_MessungKalib.csv", counter); // create file name with counter included

	// checks if file name is already used
	if (access(buffer, F_OK) == 0)
	{
		printf("File mit diesem Namen existiert bereits.\n");
		exit(1);
	}

	file_temp = (fopen(buffer, "w+"));
	file_kalib = (fopen(buffer, "w+"));

	// checks if file was created successfully
	if (file_temp == NULL || file_kalib == NULL)
	{
		printf("File konnte nicht erstellt werden.\n");
		exit(1);
	}
	else
	{
		// Header for CSV file: Anstell- & Schiebewinkel; column header //
		fprintf(file_temp, "Date: %s; Time: %s \n", buffer_date, buffer_time);
		fprintf(file_kalib, "Date: %s; Time: %s \n", buffer_date, buffer_time);
		fprintf(file_temp, "Anstellwinkel: %f - Schiebewinkel %f\n", temp_A, temp_S);
		fprintf(file_kalib, "Anstellwinkel: %f - Schiebewinkel %f\n", temp_A, temp_S);
		fprintf(file_temp, "Counter; Timestamp; ID; Latency; Pressure 1; Pressure 2; Pressure 3; Pressure 4; Pressure 5; Pressure 6; Pressure 7; Temperature 1; Temperature 2; Temperature 3; Temperature 4; Temperature 5; Temperature 6; Temperature 7\n");
		fprintf(file_kalib, "Counter; Timestamp; ID; Latency; Pressure 1; Pressure 2; Pressure 3; Pressure 4; Pressure 5; Pressure 6; Pressure 7; Temperature 1; Temperature 2; Temperature 3; Temperature 4; Temperature 5; Temperature 6; Temperature 7\n");
		printf("Datei wurde erfolgreich erstellt. \n");
	}

	printf("Übertragung gestartet.");

	for (int i = 0; i < 200; i++)
	{
		int rec_values = net->net_com_receive(&rx_data, sizeof(struct sensor_data));

		int rec_values = 0;
		do
		{
			rec_values = net->net_com_receive(&rx_data, sizeof(struct sensor_data));
			usleep(10);
		} while (rec_values == 0);

		// write data in file
		latency = rx_data.timestamp - latency; // caluclates latency = difference between data packages
		fprintf(file_temp, "\n %i; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i \n", rx_data.counter, rx_data.timestamp, rx_data.id, latency, rx_data.sensor1, rx_data.sensor2,
				rx_data.sensor3, rx_data.sensor4, rx_data.sensor5, rx_data.sensor6, rx_data.sensor7, rx_data.temp1, rx_data.temp2, rx_data.temp3, rx_data.temp4, rx_data.temp5, rx_data.temp6, rx_data.temp7);

		rx_data.sensor1 = -offset_p1;
		rx_data.sensor2 = -offset_p2;
		rx_data.sensor3 = -offset_p3;
		rx_data.sensor4 = -offset_p4;
		rx_data.sensor5 = -offset_p5;

		fprintf(file_kalib, "\n %i; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i \n", rx_data.counter, rx_data.timestamp, rx_data.id, latency, rx_data.sensor1, rx_data.sensor2,
				rx_data.sensor3, rx_data.sensor4, rx_data.sensor5, rx_data.sensor6, rx_data.sensor7, rx_data.temp1, rx_data.temp2, rx_data.temp3, rx_data.temp4, rx_data.temp5, rx_data.temp6, rx_data.temp7);

		// print in console
		printf("\n %i; %i; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i \n", rx_data.counter, rx_data.timestamp, rx_data.id, latency, rx_data.sensor1, rx_data.sensor2,
			   rx_data.sensor3, rx_data.sensor4, rx_data.sensor5, rx_data.sensor6, rx_data.sensor7, rx_data.temp1, rx_data.temp2, rx_data.temp3, rx_data.temp4, rx_data.temp5, rx_data.temp6, rx_data.temp7);
		latency = rx_data.timestamp; // reset temp_timestamp to timestemp of recent data package

		// Pause programm
		fflush(stdout); // flushed Outputstream bevor System schläft - notwendig vor allem wenn Daten auf Konsole ausgegeben werden
		usleep(20);		// Milisekunden - Datenpakete werden nur alle 20ms verschickt
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

	// StepperMotor sm;

	float Anstellwinkel = 0; // von -18° bis 18°
	int counter = 0;		 // Anzahl der Messungen
	uint step = 1;			 // Traversor steps in degree

	while (true)
	{
		// Menü
		cout << "Wähle:" << endl;
		cout << "1: Nullpunkt setzten." << endl;
		cout << "2: Neuen Schiebewinkel eingeben und Messung starten." << endl;
		cout << "3: Programm beenden." << endl;

		// Terminates programm if 2 is choosen
		char input;
		scanf(" %c", &input);
		if (input != '1' || '2' || '3')
		{
			printf("Falsche Eingabe.\n");
			continue;
		}
		else if (input == '3')
		{
			break;
		}

		// Sets Schiebewinkel
		printf("Bisheriger Anstellwinkel: %.2f\n", Anstellwinkel);
		printf("Neuen Anstellwinkel eingeben: ");
		scanf("%f", &Anstellwinkel);
		printf("Gesetzter Anstellwinkel: %.2f\n", Anstellwinkel);

		// reads sensor data for all Anstellwinkel for the set Schiebewinkel
		for (int Schiebewinkel = -18; Schiebewinkel < 19; Schiebewinkel++) // von -6° bis 15°
		{
			counter++;
			printf("%i Messung. \n", counter);
			readValues(&net, counter, Anstellwinkel, Schiebewinkel);
			// sm.run(1, 1); // Startposition -6° -> Sonde dreht sich im Uhrzeigersinn
			fflush(stdout);
			// sleep(30); // Wartet 30 Sek. damit sich Luftstrom stabilisieren kann -> wahrscheinlich unnötig, Luftstrom bleibt gleich
		}
	}
	return 0;
}
