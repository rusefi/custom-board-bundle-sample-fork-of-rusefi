#pragma once

#include "speed_density_base.h"

class SpeedDensityAirmass : public SpeedDensityBase {
public:
	explicit SpeedDensityAirmass(const ValueProvider3D& veTable, const ValueProvider3D& mapEstimationTable)
		: SpeedDensityBase(veTable)
		, m_mapEstimationTable(&mapEstimationTable)
	{}

	AirmassResult getAirmass(int rpm) override;
	AirmassResult getAirmass(float rpm, float map);
	float getAirflow(float rpm, float map);

	float getMap(int rpm) const;

private:
	const ValueProvider3D* const m_mapEstimationTable;
};
