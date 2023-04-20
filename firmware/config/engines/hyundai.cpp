/*
 * @file hyundai.cpp
 *
 * @date Oct 11, 2021
 * @author Andrey Belomutskiy, (c) 2012-2021
 */

#include "pch.h"

#include "hyundai.h"
#include "proteus_meta.h"

void setHyundaiPb() {
	engineConfiguration->specs.cylindersCount = 4;
	engineConfiguration->specs.firingOrder = FO_1_3_4_2;
	engineConfiguration->specs.displacement = 1.6;
	strcpy(engineConfiguration->engineMake, ENGINE_MAKE_Hyundai);
	strcpy(engineConfiguration->engineCode, "Gamma");
}

void setProteusHyundaiPb() {
	engineConfiguration->triggerInputPins[0] = PROTEUS_VR_1;
	engineConfiguration->camInputs[0] = PROTEUS_DIGITAL_1;
	engineConfiguration->camInputs[1] = PROTEUS_DIGITAL_2;
}

static void commonGenesisCoupe() {
	strncpy(config->luaScript, R"(

-- code outside of functions is invoked once on start-up
-- listen to CAN ID 0x4f0
canRxAdd(0x4f0)

-- todo: add payload logic
packet4f1 = { 0xFA, 0x54, 0x00, 0x0, 0x00, 0x51, 0x00, 0x00 }

function onCanRx(bus, id, dlc, data)
	print('got CAN id=' .. id .. ' dlc='  .. dlc)
	-- todo: add filtering logic if needed

	rpm = getSensor("RPM")
	-- handle nil RPM, todo: change firmware to avoid nil RPM
	rpm = (rpm == nil and 0 or rpm)

	if rpm > 800 then
		-- at the moment we simply
		txCan(1, 0x4f1, 0, packet4f1)
	end


end

function onTick()
 -- empty function onTick we are not doing anything periodically so far
end


)", efi::size(config->luaScript));
}

void setGenesisCoupeBK1() {
	commonGenesisCoupe();
	// 2010-2012 315cc at 43.5psi
	// fuel system is fixed pressure 55psi
	engineConfiguration->injector.flow = 354.19; // https://www.google.com/search?q=315*sqrt%2855%2F43.5%29
	engineConfiguration->fuelReferencePressure = PSI2KPA(55);

}

void setGenesisCoupeBK2() {
	commonGenesisCoupe();
	// 2013+ 450cc at 43.5
	// fuel system is fixed pressure 85psi
	// flow rate P2 = flow rate P1 * sqrt(P2/P1)
	engineConfiguration->injector.flow = 629.03; // https://www.google.com/search?q=450*sqrt%2885%2F43.5%29
	engineConfiguration->fuelReferencePressure = PSI2KPA(85);

}
