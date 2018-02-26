/**
 * @file	servo.cpp
 *
 * http://rusefi.com/wiki/index.php?title=Hardware:Servo_motor
 *
 * @date Jan 3, 2015
 * @author Andrey Belomutskiy, (c) 2012-2018
 */

#include "servo.h"
#include "pin_repository.h"
#include "engine.h"

// todo: remove this hack once we have next incompatible configuration change
#define TEMP_FOR_COMPATIBILITY GPIOA_0

EXTERN_ENGINE;

THD_WORKING_AREA(seThreadStack, UTILITY_THREAD_STACK_SIZE);

OutputPin pin;

static int countServos() {
	int result = 0;

	for (int i = 0; i < SERVO_COUNT; i++) {
		if (engineConfiguration->servoOutputPins[i] != GPIO_UNASSIGNED &&
				engineConfiguration->servoOutputPins[i] != TEMP_FOR_COMPATIBILITY) {
			result++;
		}
	}
	return result;

}

static msg_t seThread(void *arg) {
	(void)arg;
	chRegSetThreadName("servo");
	while (true) {
		int count = countServos();

		chThdSleepMilliseconds(19 - 2 * count);
	}
	return 0;
}

void initServo(void) {


	brain_pin_e p = engineConfiguration->servoOutputPins[0];
	if (p != TEMP_FOR_COMPATIBILITY) {
		pin.initPin("servo", p);
	}

	chThdCreateStatic(seThreadStack, sizeof(seThreadStack), NORMALPRIO, (tfunc_t) seThread, NULL);
}


