#include "system.h"
#include <stdio.h>
#include <unistd.h>
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_timer_regs.h"
#include "sys/types.h"

// Structs
struct Time
{
	int milliseconds;
	int seconds;
	int minutes;
	int hours;
};

//****************************************************************************************************************
// Functions
void horloge(struct Time *currentTime)
{
	currentTime->seconds++;

	if (currentTime->seconds > 59)
	{
		currentTime->minutes++;
		currentTime->seconds = 0;
	}

	if (currentTime->minutes > 59)
	{
		currentTime->hours++;
		currentTime->minutes = 0;
	}

	if (currentTime->hours > 23)
	{
		currentTime->hours = 0;
	}
}

void millisecondsHorloge(struct Time *currentTime)
{
	currentTime->milliseconds++;

	if (currentTime->milliseconds > 999)
	{
		currentTime->seconds++;
		currentTime->milliseconds = 0;
	}

	if (currentTime->seconds > 59)
	{
		currentTime->minutes++;
		currentTime->seconds = 0;
	}

	if (currentTime->minutes > 59)
	{
		currentTime->hours++;
		currentTime->minutes = 0;
	}
	if (currentTime->hours > 23)
	{
		currentTime->hours = 0;
	}
}

void countDownHorloge(struct Time *currentTime)
{

	int stopDecrementing = 0;

	if (currentTime->minutes == 0 && currentTime->seconds == 0)
	{
		stopDecrementing = 1;
	}

	if (stopDecrementing == 0)
	{
		if (currentTime->seconds == 0)
		{
			currentTime->minutes--;
			currentTime->seconds = 59;
		}
		else
		{
			currentTime->seconds--;
		}
	}
}

int integerTo7seg(int n)
{
	// The LUT
	unsigned char lut[] = {
		0x3F, // 0
		0x06, // 1
		0x5B, // 2
		0x4F, // 3
		0x66, // 4
		0x6D, // 5
		0x7D, // 6
		0x07, // 7
		0x7F, // 8
		0x67, // 9
		0x40  // -
	};
	int i = 0;
	int shiftValue = 0;
	int seven_segment_value = 0;
	int digit = 0;

	if (n == 0)
	{
		seven_segment_value = lut[0];
		seven_segment_value = seven_segment_value | (lut[0] << shiftValue + 8);
	}

	while (n < 0)
	{
		seven_segment_value = seven_segment_value | (lut[10] << 24); // Pour positionner le signe negatif

		digit = abs(n) % 10;
		seven_segment_value = seven_segment_value | (lut[digit] << shiftValue);
		n = abs(n) / 10;
		shiftValue += 8;
		i++;
	}

	while (n > 0)
	{
		if (n < 10)
			seven_segment_value = seven_segment_value | (lut[0] << shiftValue + 8);
		digit = n % 10;
		seven_segment_value = seven_segment_value | (lut[digit] << shiftValue);
		n /= 10;
		shiftValue += 8;
		i++;
	}
	return seven_segment_value;
};

void write7seg(int seconds, int minutes, int hours)
{
	IOWR_ALTERA_AVALON_PIO_DATA(HEX1_HEX0_BASE, seconds);
	IOWR_ALTERA_AVALON_PIO_DATA(HEX3_HEX2_BASE, minutes);
	IOWR_ALTERA_AVALON_PIO_DATA(HEX5_HEX4_BASE, hours);
}

void melodie(int melodie[], int size, int *i)
{
	int indice = *i;

	IOWR_ALTERA_AVALON_TIMER_PERIODL(SIGNAL_GENERATOR_TIMER_BASE, (melodie[indice] & 0xFFFF));
	IOWR_ALTERA_AVALON_TIMER_PERIODH(SIGNAL_GENERATOR_TIMER_BASE, ((melodie[indice] >> 16) & 0xFFFF));
	IOWR_ALTERA_AVALON_TIMER_CONTROL(SIGNAL_GENERATOR_TIMER_BASE, 0x6);

	if (indice == size - 1)
		(*i) = 0;
	else
		(*i)++;
}

//****************************************************************************************************************
int main(void)
{
	struct Time currentTime, Alarm, Stopwatch, countDown;
	currentTime.seconds = 0, currentTime.minutes = 0, currentTime.hours = 0;
	Alarm.seconds = 0, Alarm.minutes = 0, Alarm.hours = 0;
	Stopwatch.seconds = 0, Stopwatch.minutes = 0, Stopwatch.hours = 0, Stopwatch.milliseconds = 0;
	countDown.seconds = 0, countDown.minutes = 0, countDown.hours = 0;

	int seconds7seg, minutes7seg, hours7seg;
	int seconds7segAlarm, minutes7segAlarm, hours7segAlarm;
	int milliseconds7segStopwatch, seconds7segStopwatch, minutes7segStopwatch, hours7segStopwatch;
	int seconds7segcountDown, minutes7segcountDown, hours7segcountDown;

	int currentTime_hours_format_12 = 0;
	int currentTime_hours_format_12_7seg = 0;

	int Alarm_hours_format_12 = 0;
	int Alarm_hours_format_12_7seg = 0;

	int SW_value; // SW[9..0]
	int SW0, SW1, SW2, SW3, SW4, SW5, SW6, SW7, SW8, SW9;

	int press; // KEY[1..0]
	int delay_count = 0;

	int signal_haut_parleur;

	int SIGNAL_GENERATOR_etat_timer = 0;
	int etat_timer = 0;

	int Alarm_beeps;

	int Stopwatch_should_count = 0, Stopwatch_reset = 0;

	int frequency = 0;

	int i = 0;
	int i_Countdown = 0;

	int countDown_value_change = 0, countDown_should_decrement = 0;
	int countDown_possible_values[5] = {1, 2, 5, 10, 30};
	int stopDecrementing = 0;
	int countDown_reset = 0;

	int CountDown_etat_timer;
	int Stopwatch_etat_timer;

	int C4 = 191570;
	int D4 = 170648;
	int E4 = 151975;
	int F4 = 143266;
	int G4 = 127877;
	int A4 = 113636;
	int B4;
	int C5 = 95602;
	int C3 = 381679;
	int G3 = 255102;
	int C2 = 769230;

	int frere_jacques[33] = {C4, D4, E4, C4, C4, D4, E4, C4, C4, E4, G4, C4, E4, G4, G4, A4, G4, F4, E4, C4, G4, A4, G4, F4, E4, C4, C4, C3, C4, C4, G3, C4, 50000000};
	int size_frere_jacques = sizeof(frere_jacques) / sizeof(frere_jacques[0]);
	int birthday[12] = {C4, C4, D4, C4, F4, E4, C4, C4, D4, C4, G4, F4};
	int size_birthday = sizeof(birthday) / sizeof(birthday[0]);
	int anthem[19] = {A4, A4, F4, F4, F4, E4, D4, E4, F4, F4, E4, D4, F4, F4, C2, E4, E4, D4, D4, C4};
	int size_anthem = sizeof(anthem) / sizeof(anthem[0]);

	IOWR_ALTERA_AVALON_TIMER_PERIODL(SYS_CLK_TIMER_BASE, (50000000 & 0xFFFF));
	IOWR_ALTERA_AVALON_TIMER_PERIODH(SYS_CLK_TIMER_BASE, ((50000000 >> 16) & 0xFFFF));
	IOWR_ALTERA_AVALON_TIMER_CONTROL(SYS_CLK_TIMER_BASE, 0x6);

	IOWR_ALTERA_AVALON_TIMER_PERIODL(STOPWATCH_TIMER_BASE, (50000 & 0xFFFF));
	IOWR_ALTERA_AVALON_TIMER_PERIODH(STOPWATCH_TIMER_BASE, ((50000 >> 16) & 0xFFFF));
	IOWR_ALTERA_AVALON_TIMER_CONTROL(STOPWATCH_TIMER_BASE, 0x6);

	IOWR_ALTERA_AVALON_TIMER_PERIODL(COUNTDOWN_TIMER_BASE, (50000000 & 0xFFFF));
	IOWR_ALTERA_AVALON_TIMER_PERIODH(COUNTDOWN_TIMER_BASE, ((50000000 >> 16) & 0xFFFF));
	IOWR_ALTERA_AVALON_TIMER_CONTROL(COUNTDOWN_TIMER_BASE, 0x6);

	while (1)
	{

		SW_value = IORD_ALTERA_AVALON_PIO_DATA(INTERRUPTEURS_BASE);
		SW0 = SW_value & 0x1;
		SW1 = (SW_value >> 1) & 0x1; // Recuperer que l'etat des deux switchs SW0, SW1
		SW2 = (SW_value >> 2) & 0x1;
		SW3 = (SW_value >> 3) & 0x1;
		SW4 = (SW_value >> 4) & 0x1;
		SW5 = (SW_value >> 5) & 0x1;
		SW6 = (SW_value >> 6) & 0x1;
		SW7 = (SW_value >> 7) & 0x1;
		SW8 = (SW_value >> 8) & 0x1;
		SW9 = (SW_value >> 9) & 0x1;

		// press = IORD_ALTERA_AVALON_PIO_DATA(BOUTONS_POUSSOIRS_BASE); // Detecte l'etat continuellement
		press = IORD_ALTERA_AVALON_PIO_EDGE_CAP(BOUTONS_POUSSOIRS_BASE); // Detecte que le front montant
		IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BOUTONS_POUSSOIRS_BASE, 0);

		etat_timer = IORD_ALTERA_AVALON_TIMER_STATUS(SYS_CLK_TIMER_BASE);
		SIGNAL_GENERATOR_etat_timer = IORD_ALTERA_AVALON_TIMER_STATUS(SIGNAL_GENERATOR_TIMER_BASE);
		CountDown_etat_timer = IORD_ALTERA_AVALON_TIMER_STATUS(COUNTDOWN_TIMER_BASE);
		Stopwatch_etat_timer = IORD_ALTERA_AVALON_TIMER_STATUS(STOPWATCH_TIMER_BASE);
		//---------------------------------------------------------------------------------------------------------------------------
		if (etat_timer == 3)
		{
			IOWR_ALTERA_AVALON_TIMER_STATUS(SYS_CLK_TIMER_BASE, 0);
			horloge(&currentTime);

			if (SW9 == 1)
				melodie(&frere_jacques, size_frere_jacques, &i);
			else if (SW8 == 1)
				melodie(&birthday, size_birthday, &i);
			else if (SW7 == 1)
				melodie(&anthem, size_anthem, &i);
		}
		//---------------------------------------------------------------------------------------------------------------------------
		if (countDown_should_decrement == 1)
		{
			if (CountDown_etat_timer == 3)
			{
				IOWR_ALTERA_AVALON_TIMER_STATUS(COUNTDOWN_TIMER_BASE, 0);
				countDownHorloge(&countDown);
			}
		}

		if (countDown.minutes == 0 && countDown.seconds == 0)
			countDown_should_decrement = 0;
		//---------------------------------------------------------------------------------------------------------------------------
		if (Stopwatch_should_count == 1)
		{
			if (Stopwatch_etat_timer == 3)
			{
				IOWR_ALTERA_AVALON_TIMER_STATUS(STOPWATCH_TIMER_BASE, 0);
				millisecondsHorloge(&Stopwatch);
			}
		}
		//---------------------------------------------------------------------------------------------------------------------------
		seconds7seg = integerTo7seg(currentTime.seconds);
		minutes7seg = integerTo7seg(currentTime.minutes);
		hours7seg = integerTo7seg(currentTime.hours);

		seconds7segAlarm = integerTo7seg(Alarm.seconds);
		minutes7segAlarm = integerTo7seg(Alarm.minutes);
		hours7segAlarm = integerTo7seg(Alarm.hours);

		milliseconds7segStopwatch = integerTo7seg(Stopwatch.milliseconds);
		seconds7segStopwatch = integerTo7seg(Stopwatch.seconds);
		minutes7segStopwatch = integerTo7seg(Stopwatch.minutes);
		hours7segStopwatch = integerTo7seg(Stopwatch.hours);

		seconds7segcountDown = integerTo7seg(countDown.seconds);
		minutes7segcountDown = integerTo7seg(countDown.minutes);
		//---------------------------------------------------------------------------------------------------------------------------
		// Alarme activee, LED0 allumee
		if (SW2 == 1)
		{
			// La led correspondante
			IOWR_ALTERA_AVALON_PIO_DATA(LEDR_BASE, 1);

			// Activer la sonnerie
			if (Alarm.seconds == currentTime.seconds && Alarm.minutes == currentTime.minutes && Alarm.hours == currentTime.hours)
			{
				Alarm_beeps = 1;
				IOWR_ALTERA_AVALON_PIO_DATA(LEDR_BASE, 1);
			}
		}
		// Alarme desactivee, LED0 eteinte
		else
		{
			IOWR_ALTERA_AVALON_PIO_DATA(LEDR_BASE, 0);
			Alarm_beeps = 0;
		}

		if (Alarm_beeps == 1)
		{
			if (SIGNAL_GENERATOR_etat_timer == 3)
			{
				IOWR_ALTERA_AVALON_TIMER_STATUS(SIGNAL_GENERATOR_TIMER_BASE, 0);
				IOWR_ALTERA_AVALON_PIO_DATA(SPEAKER_BASE, signal_haut_parleur);
				signal_haut_parleur = ~signal_haut_parleur;
			}
			if (press == 1)
				Alarm_beeps = 0;
		}

		//---------------------------------------------------------------------------------------------------------------------------
		// Mode affichage uniquement
		if (SW3 == 1 && SW6 == 0 && SW5 == 0)
		{
			// Affichage de l'heure actuelle (format d'affichage : 24 heures)
			if (SW1 == 0 && SW0 == 0)
			{
				write7seg(~seconds7seg, ~minutes7seg, ~hours7seg);
			}
			// Affichage de l'heure actuelle (format d'affichage : 12 heures)
			else if (SW1 == 1 && SW0 == 1)
			{
				// On agit pas directement sur les variables de la structure currentTime pour ne pas alterer le fonctionnement (notamment le format 24 heures mais pas que!)
				if (currentTime.hours >= 12 && currentTime.hours <= 23)
				{
					currentTime_hours_format_12 = currentTime.hours - 12;
				}
				else
				{
					currentTime_hours_format_12 = currentTime.hours;
				}

				currentTime_hours_format_12_7seg = integerTo7seg(currentTime_hours_format_12);

				write7seg(~seconds7seg, ~minutes7seg, ~currentTime_hours_format_12_7seg);
			}
			// Affichage de l'heure de l'alarme (format d'affichage : 24 heures)
			else if (SW1 == 0 && SW0 == 1)
			{
				write7seg(~seconds7segAlarm, ~minutes7segAlarm, ~hours7segAlarm);
			}
			// Affichage de l'heure de l'alarme (format d'affichage : 12 heures)
			else if (SW1 == 1 && SW0 == 0)
			{
				if (Alarm.hours >= 12 && Alarm.hours <= 23)
				{
					Alarm_hours_format_12 = Alarm.hours - 12;
				}
				else
				{
					Alarm_hours_format_12 = Alarm.hours;
				}

				Alarm_hours_format_12_7seg = integerTo7seg(Alarm_hours_format_12);

				write7seg(~seconds7segAlarm, ~minutes7segAlarm, ~Alarm_hours_format_12_7seg);
			}
		}
		//---------------------------------------------------------------------------------------------------------------------------
		// Mode affichage avec reglage incrementatif
		else if (SW4 == 0 && SW3 == 0 && SW6 == 0 && SW5 == 0)
		{
			// Reglage de l'heure actuelle (format d'affichage : 24 heures)
			if (SW1 == 0 && SW0 == 0)
			{
				if (press == 1)
				{
					if (currentTime.minutes == 59)
						currentTime.minutes = 0;
					else
						currentTime.minutes++;
				}
				if (press == 2)
				{
					if (currentTime.hours == 23)
						currentTime.hours = 0;
					else
						currentTime.hours++;
				}

				write7seg(~seconds7seg, ~minutes7seg, ~hours7seg);
			}
			// Reglage de l'heure actuelle (format d'affichage : 12 heures)
			else if (SW1 == 1 && SW0 == 1)
			{
				// On n'agit pas directement sur les variables de la structure currentTime pour ne pas alt�rer le fonctionnement (notamment le format 24 heures mais pas que!)
				if (press == 1)
				{
					if (currentTime.minutes == 59)
						currentTime.minutes = 0;
					else
						currentTime.minutes++;
				}
				if (press == 2)
				{
					if (currentTime.hours == 23)
						currentTime.hours = 0;
					else
						currentTime.hours++;
				}

				if (currentTime.hours >= 12 && currentTime.hours <= 23)
				{
					currentTime_hours_format_12 = currentTime.hours - 12;
				}
				else
				{
					currentTime_hours_format_12 = currentTime.hours;
				}

				currentTime_hours_format_12_7seg = integerTo7seg(currentTime_hours_format_12);

				write7seg(~seconds7seg, ~minutes7seg, ~currentTime_hours_format_12_7seg);
			}
			// Reglage de l'heure de l'alarme (format d'affichage : 24 heures)
			else if (SW1 == 0 && SW0 == 1)
			{
				if (press == 1)
				{
					if (Alarm.minutes == 59)
						Alarm.minutes = 0;
					else
						Alarm.minutes++;
				}
				if (press == 2)
				{
					if (Alarm.hours == 23)
						Alarm.hours = 0;
					else
						Alarm.hours++;
				}

				write7seg(~seconds7segAlarm, ~minutes7segAlarm, ~hours7segAlarm);
			}
			// Reglage de l'heure de l'alarme (format d'affichage : 12 heures)
			else if (SW1 == 1 && SW0 == 0)
			{
				if (press == 1)
				{
					if (Alarm.minutes == 59)
						Alarm.minutes = 0;
					else
						Alarm.minutes++;
				}
				if (press == 2)
				{
					if (Alarm.hours == 23)
						Alarm.hours = 0;
					else
						Alarm.hours++;
				}

				if (Alarm.hours >= 12 && Alarm.hours <= 23)
				{
					Alarm_hours_format_12 = Alarm.hours - 12;
				}
				else
				{
					Alarm_hours_format_12 = Alarm.hours;
				}

				Alarm_hours_format_12_7seg = integerTo7seg(Alarm_hours_format_12);

				write7seg(~seconds7segAlarm, ~minutes7segAlarm, ~Alarm_hours_format_12_7seg);
			}
		}
		//---------------------------------------------------------------------------------------------------------------------------
		// Mode affichage avec reglage decrementatif
		else if (SW4 == 1 && SW3 == 0 && SW6 == 0 && SW5 == 0)
		{
			// Reglage de l'heure actuelle (format d'affichage : 24 heures)
			if (SW1 == 0 && SW0 == 0)
			{
				if (press == 1)
				{
					if (currentTime.minutes == 0)
						currentTime.minutes = 59;
					else
						currentTime.minutes--;
				}
				if (press == 2)
				{
					if (currentTime.hours == 0)
						currentTime.hours = 23;
					else
						currentTime.hours--;
				}

				write7seg(~seconds7seg, ~minutes7seg, ~hours7seg);
			}
			// Reglage de l'heure actuelle (format d'affichage : 12 heures)
			else if (SW1 == 1 && SW0 == 1)
			{
				// On n'agit pas directement sur les variables de la structure currentTime pour ne pas alt�rer le fonctionnement (notamment le format 24 heures mais pas que!)
				if (press == 1)
				{
					if (currentTime.minutes == 0)
						currentTime.minutes = 59;
					else
						currentTime.minutes--;
				}
				if (press == 2)
				{
					if (currentTime.hours == 0)
						currentTime.hours = 23;
					else
						currentTime.hours--;
				}

				if (currentTime.hours >= 12 && currentTime.hours <= 23)
				{
					currentTime_hours_format_12 = currentTime.hours - 12;
				}
				else
				{
					currentTime_hours_format_12 = currentTime.hours;
				}

				currentTime_hours_format_12_7seg = integerTo7seg(currentTime_hours_format_12);

				write7seg(~seconds7seg, ~minutes7seg, ~currentTime_hours_format_12_7seg);
			}
			// Reglage de l'heure de l'alarme (format d'affichage : 24 heures)
			else if (SW1 == 0 && SW0 == 1)
			{
				if (press == 1)
				{
					if (Alarm.minutes == 0)
						Alarm.minutes = 59;
					else
						Alarm.minutes--;
				}
				if (press == 2)
				{
					if (Alarm.hours == 0)
						Alarm.hours = 23;
					else
						Alarm.hours++;
				}

				write7seg(~seconds7segAlarm, ~minutes7segAlarm, ~hours7segAlarm);
			}
			// Reglage de l'heure de l'alarme (format d'affichage : 12 heures)
			else if (SW1 == 1 && SW0 == 0)
			{
				if (press == 1)
				{
					if (Alarm.minutes == 0)
						Alarm.minutes = 59;
					else
						Alarm.minutes--;
				}
				if (press == 2)
				{
					if (Alarm.hours == 0)
						Alarm.hours = 23;
					else
						Alarm.hours--;
				}

				if (Alarm.hours >= 12 && Alarm.hours <= 23)
				{
					Alarm_hours_format_12 = Alarm.hours - 12;
				}
				else
				{
					Alarm_hours_format_12 = Alarm.hours;
				}

				Alarm_hours_format_12_7seg = integerTo7seg(Alarm_hours_format_12);

				write7seg(~seconds7segAlarm, ~minutes7segAlarm, ~Alarm_hours_format_12_7seg);
			}
		}
		//---------------------------------------------------------------------------------------------------------------------------
		// Reglage et affichage du compte a rebours
		if (SW5 == 1 && SW6 == 0 && SW3 == 0)
		{
			if (SW9 == 0)
			{
				if (press == 1)
					countDown_value_change = 1;

				if (countDown_value_change == 1)
				{
					if (i_Countdown == 5)
					{
						i_Countdown = 0;
					}
					countDown_value_change = 0;
					countDown.minutes = countDown_possible_values[i_Countdown];
					i_Countdown += 1;
				}

				if (press == 2)
					countDown_should_decrement = 1 - countDown_should_decrement;
			}

			else if (SW9 == 1)
			{
				if (press == 1)
					countDown_reset = 1;

				if (countDown_reset == 1)
				{
					countDown.minutes = countDown_possible_values[i_Countdown];
					countDown.seconds = 0;
					countDown_reset = 0;
				}
			}

			write7seg(~seconds7segcountDown, ~minutes7segcountDown, ~0x3F3F);
		}
		//---------------------------------------------------------------------------------------------------------------------------
		// Reglage et affichage du chronometre
		if (SW6 == 1 && SW5 == 0 && SW3 == 0)
		{

			if (press == 1)
				Stopwatch_should_count = 1 - Stopwatch_should_count;

			if (press == 2)
				Stopwatch_reset = 1;
			if (Stopwatch_reset == 1)
			{
				Stopwatch.hours = 0;
				Stopwatch.minutes = 0;
				Stopwatch.seconds = 0;
				Stopwatch.milliseconds = 0;

				Stopwatch_should_count = 0;
				Stopwatch_reset = 0;
			}
			if (Stopwatch.milliseconds > 99) 
				milliseconds7segStopwatch = milliseconds7segStopwatch >> 16;
			else
				milliseconds7segStopwatch = 0x3F3F;

			// J'affiche que les deux digits de la partie superieure des millisecondes
			// printf("%d\n",Stopwatch.milliseconds);
			write7seg(~milliseconds7segStopwatch, ~seconds7segStopwatch, ~minutes7segStopwatch);
		}
	}
}
