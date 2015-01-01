/**
 * @file	idle_controller.cpp
 * @brief	Simple Idle Air Valve control algorithm
 *
 *			This algorithm is trying to get current RPM to the desired 'target' value
 *			by changing Idle Air Valve solenoid duty cycle. Depending on how far current RPM
 *			is from the target RPM, the incremental change would be smaller or bigger.
 *
 *
 * todo: DEFAULT_IDLE_DUTY should be a field on the IdleValveState, not a constant
 *
 * @date May 22, 2013
 * @author Andrey Belomutskiy, (c) 2012-2014
 */

#include "main.h"
#include "idle_controller.h"
#include "efilib.h"
#include "rpm_calculator.h"
#include "tps.h"

static int lastGoodValue = DEFAULT_IDLE_DUTY;

EXTERN_ENGINE
;

IdleValveState::IdleValveState() {
	value = DEFAULT_IDLE_DUTY;
	timeOfLastIdleChange = 0;
}

void IdleValveState::init(DECLARE_ENGINE_PARAMETER_F) {
	setIdleRpm(this, engineConfiguration->targetIdleRpm);
}

void setIdleRpm(IdleValveState *idle, int targetRpm) {
	idle->targetRpmRangeLeft = (int) (targetRpm * 0.93);
	idle->targetRpmRangeRight = (int) (targetRpm * 1.07);
}

/**
 * @brief	sets new idle valve duty cycle: checks the bounds and reports new value
 */
static percent_t setNewValue(IdleValveState *idle, int currentRpm, efitimems_t now, const char * msg, percent_t newValue) {
	newValue = maxF(newValue, MIN_IDLE);
	newValue = minF(newValue, MAX_IDLE);

	if (idle->value != newValue) {
		idleDebug(msg, currentRpm);
		idle->timeOfLastIdleChange = now;
	}

	idle->value = newValue;
	return newValue;
}

static percent_t changeValue(IdleValveState *idle, int currentRpm, int now, const char * msg, percent_t delta DECLARE_ENGINE_PARAMETER_S) {
	if (getTPS(PASS_ENGINE_PARAMETER_F) > 5.0) {
		// We are not supposed to be in idle mode. Don't touch anything
		idleDebug("TPS Lockout, TPS=", getTPS(PASS_ENGINE_PARAMETER_F));
		return idle->value;
	}
	percent_t newValue = idle->value + delta;
	return setNewValue(idle, currentRpm, now, msg, newValue);
}

/**
 * now - current time in milliseconds
 */
percent_t IdleValveState::getIdle(int currentRpm, efitimems_t now DECLARE_ENGINE_PARAMETER_S) {
	if (currentRpm == 0 || isCranking()) {
		return setNewValue(this, currentRpm, now, "cranking value: ", DEFAULT_IDLE_DUTY);
	}

	if (currentRpm < 0.7 * targetRpmRangeLeft) {
		return setNewValue(this, currentRpm, now, "RPMs are seriously low: ", lastGoodValue);
	}

	if (now - timeOfLastIdleChange < IDLE_PERIOD) {
		// too soon to adjust anything - exiting
		return value;
	}

	if (currentRpm > targetRpmRangeLeft && currentRpm < targetRpmRangeRight) {
		// current RPM is good enough
		// todo: need idle signal input
		//lastGoodValue = idle->value;
		return value;
	}

	if (currentRpm >= targetRpmRangeRight + 100)
		return changeValue(this, currentRpm, now, "idle control: rpm is too high: ", -IDLE_DECREASE_STEP PASS_ENGINE_PARAMETER);

	if (currentRpm >= targetRpmRangeRight)
		return changeValue(this, currentRpm, now, "idle control: rpm is a bit too high: ", -IDLE_DECREASE_SMALL_STEP PASS_ENGINE_PARAMETER);

	// we are here if RPM is low, let's see how low
	if (currentRpm < targetRpmRangeLeft - 100) {
		return changeValue(this, currentRpm, now, "idle control: RPMs are low: ", IDLE_INCREASE_STEP PASS_ENGINE_PARAMETER);
	}
	return changeValue(this, currentRpm, now, "idle control: RPMs are a bit low: ", IDLE_INCREASE_SMALL_STEP PASS_ENGINE_PARAMETER);
}
