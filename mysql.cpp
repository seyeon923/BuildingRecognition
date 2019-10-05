#include "mysql.hpp"
#include <iostream>
#include <cstdarg>

const char hostname[] = "172.16.162.143";
const int port = 33060;
const char user[] = "xtwicett";
const char password[] = "rlatpdus";
const char schema[] = "campus_gis";

using namespace mysqlx;
using namespace std;

const char* BuildingColumnStr[] = { "BUILDING_ID", "BUILDING_NAME", "BUILDING_EN_NAME",
			"REP_LATITUDE", "REP_LONGITUDE", "REP_ALTITUDE" };
const char* SurfaceColumnStr[] = { "SURFACE_SEQ", "TOPLEFT_LATITUDE", "TOPLEFT_LONGITUDE", "TOPLEFT_ALTITUDE",
			"BOTLEFT_LATITUDE", "BOTLEFT_LONGITUDE", "BOTLEFT_ALTITUDE",
			"BOTRIGHT_LATITUDE", "BOTRIGHT_LONGITUDE", "BOTRIGHT_ALTITUDE",
			"TOPRIGHT_LATITUDE", "TOPRIGHT_LONGITUDE", "TOPRIGHT_ALTITUDE", "BUILDING_ID" };
const char* MarkerColumnStr[] = { "MARKER_SEQ", "LATITUDE", "LONGITUDE", "ALTITUDE",
			"SURFACE_SEQ", "BUILDING_ID", "MARKER_NAME" };
const char* TableStr[] = { "BUILDING", "SURFACE", "MARKER" };

using Building = GIS_DB::Building;
using Surface = GIS_DB::Surface;
using Marker = GIS_DB::Marker;

DB::DB(std::string hostname, int port, std::string user, std::string password, std::string schema) {
	pSession = new Session(hostname, port, user, password);
	pSchema = new Schema(*pSession, schema);
}

DB::~DB() {
	delete pSession;
	delete pSchema;
}

Table DB::getTable(std::string table) const {
	Table t = pSchema->getTable(table);
	return t;
}

void GIS_DB::Building::varInit() {
	buildingId[0] = 0;
	buildingName = wstring(L"");
	latitude = longitude = altitude = -1;
	buildingEnName = std::string("");
}

GIS_DB::Building::Building(std::string buildingId) {
	varInit();
	setBuildingId(buildingId);
}

void GIS_DB::Building::_setVar(Column col, const char* pchar) {
	switch (col) {
	case Column::BUILDING_ID:
		setBuildingId(pchar);
		break;
	case Column::BUILDING_EN_NAME:
		setBuildingEnName(std::string(pchar));
		break;
	default:
		wcout << "type const char* value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Building::_setVar(Column col, const wchar_t* pwchar) {
	if (col == Column::BUILDING_NAME)
		setBuildingName(wstring(pwchar));
	else {
		wcout << "type const char* value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Building::_setVar(Column col, double decimal) {
	switch (col) {
	case Column::LATITUDE:
		setLatitude(decimal);
		break;
	case Column::LONGITUDE:
		setLongitude(decimal);
		break;
	case Column::ALTITUDE:
		setAltitude(decimal);
		break;
	default:
		wcout << "type double value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Building::_setVar(Column col, std::string str) {
	switch (col) {
	case Column::BUILDING_ID:
		setBuildingId(str);
		break;
	case Column::BUILDING_EN_NAME:
		setBuildingEnName(str);
		break;
	default:
		wcout << "type std::string value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Building::_setVar(Column col, wstring wstr) {
	if (col == Column::BUILDING_NAME)
		setBuildingName(wstr);
	else {
		wcout << "type wcstr value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Building::_setVar(Column col, const Value& value) {
	switch (col) {
	case Column::BUILDING_ID:
		if (!value.isNull())
			setBuildingId(std::string(value));
		break;
	case Column::BUILDING_NAME:
		if (!value.isNull())
			setBuildingName(wstring(value));
		break;
	case Column::BUILDING_EN_NAME:
		if (!value.isNull())
			setBuildingEnName(std::string(value));
		break;
	case Column::LATITUDE:
		if (!value.isNull()) {
			setLatitude(double(value));
		}
		break;
	case Column::LONGITUDE:
		if(!value.isNull())
			setLongitude(double(value));
		break;
	case Column::ALTITUDE:
		if(!value.isNull())
			setAltitude(double(value));
		break;
	default:
		wcout << "Unknown Column!" << endl;
		exit(-1);
	}
}

void GIS_DB::Building::setBuildingId(std::string buildingId) {
	int i = 0;
	for (i = 0; i < 3 && i < buildingId.size(); i++)
		this->buildingId[i] = buildingId[i];
	this->buildingId[i] = 0;
}

void GIS_DB::Building::setBuildingName(std::wstring buildingName) {
	this->buildingName = buildingName;
}

void GIS_DB::Building::setBuildingEnName(std::string buildingEnName) {
	this->buildingEnName = buildingEnName;
}

void GIS_DB::Building::setLatitude(double lati) {
	this->latitude = lati;
}

void GIS_DB::Building::setLongitude(double longi) {
	this->longitude = longi;
}

void GIS_DB::Building::setAltitude(double alti) {
	this->altitude = alti;
}

void GIS_DB::Building::setGPSLoc(double lati, double longi, double alti) {
	setLatitude(lati);
	setLongitude(longi);
	setAltitude(alti);
}

const char* GIS_DB::Building::ColumnToStr(Column col) {
	return BuildingColumnStr[(int)col];
}

void GIS_DB::Building::print() {
	wcout << fixed;
	wcout.precision(6);
	wcout << "BUILDING_NAME: " << buildingName << endl;
	wcout << "BUILDING_ID: " << buildingId << endl;
	wcout << "BUILDING_EN_NAME: " << buildingEnName.c_str() << endl;
	wcout << "GPS: " << latitude << ", " <<
		longitude << ", " << altitude << endl << endl;
	wcout.unsetf(ios::fixed);
}

void GIS_DB::Surface::varInit() {
	surfaceSeq = -1;
	topLeftLatitude = topLeftLongitude = topLeftAltitude =
		botLeftLatitude = botLeftLongitude = botLeftAltitude = 
		botRightLatitude = botRightLongitude = botRightAltitude = 
		topRightLatitude = topRightLongitude = topRightAltitude = -1;
	buildingId[0] = 0;
}

GIS_DB::Surface::Surface(int surfaceSeq, std::string buildingId) : surfaceSeq(surfaceSeq) {
	varInit();
	setBuildingId(buildingId);
}

const char* GIS_DB::Surface::ColumnToStr(Column col) {
	return SurfaceColumnStr[(int)col];
}

void GIS_DB::Surface::_setVar(Column col, int integer) {
	switch (col) {
	case Column::SURFACE_SEQ:
		setSurfaceSeq(integer);
		break;
	case Column::BUILDING_ID:
		wcout << "type int value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	default:
		_setVar(col, (double)integer);
	}
}

void GIS_DB::Surface::_setVar(Column col, double deci) {
	switch (col) {
	case Column::BOTLEFT_LATITUDE:
		setBotLeftLati(deci); break;
	case Column::BOTLEFT_LONGITUDE:
		setBotLeftLongi(deci); break;
	case Column::BOTLEFT_ALTITUDE:
		setBotLeftAlti(deci); break;
	case Column::BOTRIGHT_LATITUDE:
		setBotRightLati(deci); break;
	case Column::BOTRIGHT_LONGITUDE:
		setBotRightLongi(deci); break;
	case Column::BOTRIGHT_ALTITUDE:
		setBotRightAlti(deci); break;
	case Column::TOPLEFT_LATITUDE:
		setTopLeftLati(deci); break;
	case Column::TOPLEFT_LONGITUDE:
		setTopLeftLongi(deci); break;
	case Column::TOPLEFT_ALTITUDE:
		setTopLeftAlti(deci); break;
	case Column::TOPRIGHT_LATITUDE:
		setTopRightLati(deci); break;
	case Column::TOPRIGHT_LONGITUDE:
		setTopRightLongi(deci); break;
	case Column::TOPRIGHT_ALTITUDE:
		setTopRightAlti(deci); break;
	default:
		wcout << "type double value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Surface::_setVar(Column col, const char* pchar) {
	if (col == Column::BUILDING_ID)
		setBuildingId(std::string(pchar));
	else {
		wcout << "type const char* value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Surface::_setVar(Column col, std::string str) {
	if (col == Column::BUILDING_ID)
		setBuildingId(str);
	else {
		wcout << "type std::string value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Surface::_setVar(Column col, GPS gps) {
	switch(col) {
	case Column::TOPRIGHT_GPS:
		setTopRightGPS(gps);
		break;
	case Column::TOPLEFT_GPS:
		setTopLeftGPS(gps);
		break;
	case Column::BOTRIGHT_GPS:
		setBotRightGPS(gps);
		break;
	case Column::BOTLEFT_GPS:
		setBotLeftGPS(gps);
		break;
	default:
		wcout << "type GPS object can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Surface::_setVar(Column col, const Value& val) {
	if (!val.isNull())
		switch (col) {
		case Column::SURFACE_SEQ:
			setSurfaceSeq((int)val);
			break;
		case Column::BUILDING_ID:
			setBuildingId(std::string(val));
			break;
		case Column::TOPLEFT_GPS:
		case Column::TOPRIGHT_GPS:
		case Column::BOTLEFT_GPS:
		case Column::BOTRIGHT_GPS:
			wcout << "wrong specified colum " << endl;
			exit(1);
		default:
			_setVar(col, (double)val);
		}
}

void GIS_DB::Surface::setBuildingId(std::string str) {
	int i = 0;
	for (i = 0; i < 3 && i < str.size(); i++) {
		buildingId[i] = str[i];
	}
	buildingId[i] = 0;
}

void GIS_DB::Surface::print() {
	wcout << fixed;
	wcout.precision(6);
	wcout << "SURFACE_SEQ: " << surfaceSeq << endl;
	wcout << "TOPRIGHT_GPS: " << topRightLatitude << ", " << topRightLongitude <<
		", " << topRightAltitude << endl;
	wcout << "TOPLEFT_GPS: " << topLeftLatitude << ", " <<
		topLeftLongitude << ", " <<
		topLeftAltitude << endl;
	wcout << "BOTRIGHT_GPS: " << botRightLatitude << ", " <<
		botRightLongitude << ", " <<
		botRightAltitude << endl;
	wcout << "BOTLEFT_GPS: " << botLeftLatitude << ", " <<
		botLeftLongitude << ", " <<
		botLeftAltitude << endl;
	wcout << "ID: " << buildingId << endl << endl;
	wcout.unsetf(ios::fixed);
}

void GIS_DB::Marker::varInit() {
	markerSeq = surfaceSeq = -1;
	latitude = longitude = altitude = -1;
	buildingId[0] = 0;
	markerName = wstring(L"");
}

GIS_DB::Marker::Marker(int markerSeq, int surfaceSeq, std::string buildingId) :
	markerSeq(markerSeq), surfaceSeq(surfaceSeq) {
	varInit();
	setBuildingId(buildingId);
}

const char* GIS_DB::Marker::ColumnToStr(Column col) {
	return MarkerColumnStr[(int)col];
}

void GIS_DB::Marker::setBuildingId(std::string str) {
	int i = 0;
	for (i = 0; i < 3 && i < str.size(); i++)
		buildingId[i] = str[i];
	buildingId[i] = 0;
}

const char* GIS_DB::TableToStr(GIS_DB::Table t) {
	return TableStr[(int)t];
}

void GIS_DB::Marker::_setVar(Column col, int val) {
	switch (col) {
	case Column::MARKER_SEQ:
		setMarkerSeq(val);
		break;
	case Column::SURFACE_SEQ:
		setSurfaceSeq(val);
		break;
	case Column::LATITUDE:
	case Column::LONGITUDE:
	case Column::ALTITUDE:
		_setVar(col, (double)val);
		break;
	default:
		wcout << "type int value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Marker::_setVar(Column col, double val) {
	switch (col) {
	case Column::LATITUDE:
		setLatitude(val);
		break;
	case Column::LONGITUDE:
		setLongitude(val);
		break;
	case Column::ALTITUDE:
		setAltitude(val);
		break;
	default:
		wcout << "type double value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Marker::_setVar(Column col, std::string str) {
	if (col == Column::BUILDING_ID)
		setBuildingId(str);
	else {
		wcout << "type std::string value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Marker::_setVar(Column col, wstring wstr) {
	if (col == Column::MARKER_NAME)
		setMarkerName(wstr);
	else {
		wcout << "type wstring value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Marker::_setVar(Column col, GPS gps) {
	if (col == Column::GPS)
		setGPS(gps);
	else {
		wcout << "type GPS object can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Marker::_setVar(Column col, const Value& val) {
	if(!val.isNull())
		switch (col) {
		case Column::MARKER_SEQ:
		case Column::SURFACE_SEQ:
			_setVar(col, (int)val);
			break;
		case Column::LATITUDE:
		case Column::LONGITUDE:
		case Column::ALTITUDE:
			_setVar(col, (double)val);
			break;
		case Column::BUILDING_ID:
			setBuildingId(std::string(val));
			break;
		case Column::MARKER_NAME:
			setMarkerName(wstring(val));
			break;
		default:
			wcout << "type mysqlx::Value value can't set to " << ColumnToStr(col) << endl;
			exit(1);
		}
}

void GIS_DB::Marker::_setVar(Column col, const char* pchar) {
	if (col == Column::BUILDING_ID)
		setBuildingId(std::string(pchar));
	else {
		wcout << "type const char* value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Marker::_setVar(Column col, const wchar_t* pwchar) {
	if (col == Column::MARKER_NAME)
		setMarkerName(wstring(pwchar));
	else {
		wcout << "type const wchar_t* value can't set to " << ColumnToStr(col) << endl;
		exit(1);
	}
}

void GIS_DB::Marker::print() {
	wcout << fixed;
	wcout.precision(6);
	wcout << "MARKER_NAME: " << markerName << endl;
	wcout << "GPS: " << latitude << ", " << longitude << ", " << altitude << endl;
	wcout << "MARKER_SEQ: " << markerSeq << endl;
	wcout << "SURFACE_SEQ: " << surfaceSeq << endl;
	wcout << "BUILDING_ID: " << buildingId << endl << endl;
	wcout.unsetf(ios::fixed);
}

void GIS_DB::RowToBuilding(const Row& row, Building& building, const vector<BuildingCol>& selectedCols) {
	building.varInit();
	for(int i = 0; i < selectedCols.size(); i++)
		building.setVar(selectedCols[i], row[i]);
}

void GIS_DB::RowToSurface(const mysqlx::Row& row, Surface& surface, const vector<Surface::Column>& selectedCols) {
	surface.varInit();
	for (int i = 0; i < selectedCols.size(); i++)
		surface.setVar(selectedCols[i], row[i]);
}

void GIS_DB::RowToMarker(const mysqlx::Row& row, Marker& marker, const vector<Marker::Column>& selectedCols) {
	marker.varInit();
	for (int i = 0; i < selectedCols.size(); i++)
		marker.setVar(selectedCols[i], row[i]);
}

void GIS_DB::selectFromBuilding(vector<Building*>& buildings, mysqlx::string whereCond, int limit) {
	mysqlx::Table tBuilding = getTable(TableToStr(Table::BUILDING));
	RowResult rowResult;
	mysqlx::TableSelect tSelect = tBuilding.select(
		"BUILDING_ID", "BUILDING_NAME", "REP_LATITUDE",
		"REP_LONGITUDE", "REP_ALTITUDE", "BUILDING_EN_NAME");
	if (limit < 0) {
		if (whereCond.empty())
			rowResult = tSelect.execute();
		else
			rowResult = tSelect.where(whereCond).execute();
	}
	else {
		if (whereCond.empty())
			rowResult = tSelect.limit(limit).execute();
		else
			rowResult = tSelect.where(whereCond).limit(limit).execute();
	}

	for (int i = 0; rowResult.count() > 0; i++) {
		Row row = rowResult.fetchOne();
		Building* pBuilding = new Building();
		RowToBuilding(row, *pBuilding, buildingAllSelectList);
		buildings.push_back(pBuilding);
	}
}

void GIS_DB::selectFromSurface(vector<Surface*>& surfaces, const mysqlx::string whereCond, int limit) {
	mysqlx::Table tSurface = getTable(TableToStr(Table::SURFACE));
	RowResult rowResult;
	mysqlx::TableSelect tSelect = tSurface.select(
		"SURFACE_SEQ", "TOPLEFT_LATITUDE", "TOPLEFT_LONGITUDE", "TOPLEFT_ALTITUDE",
		"BOTLEFT_LATITUDE", "BOTLEFT_LONGITUDE", "BOTLEFT_ALTITUDE",
		"BOTRIGHT_LATITUDE", "BOTRIGHT_LONGITUDE", "BOTRIGHT_ALTITUDE",
		"TOPRIGHT_LATITUDE", "TOPRIGHT_LONGITUDE", "TOPRIGHT_ALTITUDE", "BUILDING_ID");
	if (limit < 0) {
		if (whereCond.empty())
			rowResult = tSelect.execute();
		else
			rowResult = tSelect.where(whereCond).execute();
	}
	else {
		if (whereCond.empty())
			rowResult = tSelect.limit(limit).execute();
		else
			rowResult = tSelect.where(whereCond).limit(limit).execute();
	}

	for (int i = 0; rowResult.count() > 0; i++) {
		Row row = rowResult.fetchOne();
		Surface* pSurface = new Surface();
		RowToSurface(row, *pSurface, SurfaceAllSelectList);
		surfaces.push_back(pSurface);
	}
}

void mysqlTest() {
	GIS_DB::Building b1(std::string("000"));
	b1.print();

	GIS_DB::Building b2(std::string("10"));
	b2.print();

	GIS_DB::Building b3(BuildingCol::BUILDING_NAME, L"음악관",
		BuildingCol::BUILDING_ID, "11",
		BuildingCol::BUILDING_EN_NAME, "Music Building",
		BuildingCol::LATITUDE, 37,
		BuildingCol::LONGITUDE, 12,
		BuildingCol::ALTITUDE, 0);
	b3.print();

	b1.setVar(BuildingCol::BUILDING_NAME, L"테스트건물",
		BuildingCol::BUILDING_ID, "-1",
		BuildingCol::BUILDING_EN_NAME, "Test Building",
		BuildingCol::LATITUDE, 37.1234,
		BuildingCol::LONGITUDE, 127.5678,
		BuildingCol::ALTITUDE, 100);
	b1.print();

	GIS_DB::Surface s1(SurfaceCol::SURFACE_SEQ, -1,
		SurfaceCol::TOPRIGHT_GPS, GPS(1, 2, 3),
		SurfaceCol::TOPLEFT_GPS, GPS(4, 5, 6),
		SurfaceCol::BOTRIGHT_GPS, GPS(7, 8, 9),
		SurfaceCol::BOTLEFT_GPS, GPS(10, 11, 12),
		SurfaceCol::BUILDING_ID, "2");
	s1.print();

	s1.setVar(SurfaceCol::TOPRIGHT_LATITUDE, -1,
		SurfaceCol::TOPLEFT_LATITUDE, -1,
		SurfaceCol::BOTRIGHT_LATITUDE, -1,
		SurfaceCol::BOTLEFT_LATITUDE, -1);
	s1.print();

	GIS_DB::Marker m1(MarkerCol::MARKER_SEQ, 123,
		MarkerCol::GPS, GPS(10, 20, 30),
		MarkerCol::SURFACE_SEQ, 456,
		MarkerCol::BUILDING_ID, "25",
		MarkerCol::MARKER_NAME, L"테스트마커");
	m1.print();

	wcout << "MySQL Connecting..." << endl;
	GIS_DB db(hostname, port, user, password, schema);
	wcout << "Connected" << endl;

	vector<Building*> buildings;
	db.selectFromBuilding(buildings, "BUILDING_ID='19' OR BUILDING_ID='5'");
	for (int i = 0; i < buildings.size(); i++) {
		buildings[i]->print();
	}
	clearPointerVec(buildings);

	vector<Surface*> surfaces;
	db.selectFromSurface(surfaces, "BUILDING_ID='19'");
	for (int i = 0; i < surfaces.size(); i++)
		surfaces[i]->print();
	clearPointerVec(surfaces);

	db.selectFromBuilding(buildings, "");
	for (int i = 0; i < buildings.size(); i++)
		buildings[i]->print();
	clearPointerVec(buildings);
}