#ifndef __MYSQL_HPP
#define __MYSQL_HPP

#include <mysqlx/xdevapi.h>

#include <string>
#include <vector>

using namespace std;
class DB {
	mysqlx::Session* pSession;
	mysqlx::Schema* pSchema;

public:
	DB(std::string hostname, int port, std::string user, std::string password, std::string schema);
	~DB();
	mysqlx::Table getTable(std::string table) const;


};

extern const char* BuildingColumnumnStr[];
extern const char* SurfaceColumnStr[];
extern const char* MarkerColumnStr[];
extern const char* TableStr[];

class GPS {
public:
	double latitude, longitude, altitude;
	GPS(double lati, double longi, double alti) :
		latitude(lati), longitude(longi), altitude(alti) {}
};

class GIS_DB : public DB{
public:	
	enum class Table {
		BUILDING, SURFACE, MARKER
	};
	static const char* TableToStr(Table t);

	class Building {
	public:
		char buildingId[4];
		wstring buildingName;
		double latitude, longitude, altitude;
		std::string buildingEnName;

		enum class Column {
			BUILDING_ID, BUILDING_NAME, BUILDING_EN_NAME,
			LATITUDE, LONGITUDE, ALTITUDE
		};
		static const char* ColumnToStr(Column col);

	private:
		void _setVar(Column col, const char* pchar);
		void _setVar(Column col, const wchar_t* pwchar);
		void _setVar(Column col, double decimal);
		void _setVar(Column col, std::string str);
		void _setVar(Column col, wstring wstr);
		void _setVar(Column col, const mysqlx::Value&);

	public:
		Building() { varInit(); }
		Building(std::string buildingId);
		template<typename ... Types>
		Building(Types ... args) {
			varInit();
			setVar(args...);
		}

		template <typename T, typename ... Types>
		void setVar(Column col, T val, Types ... args) {
			_setVar(col, val);
			setVar(args...);
		}
		void setVar() {};

		void varInit();
		void setBuildingId(std::string buildingId);
		void setBuildingName(std::wstring buildingName);
		void setBuildingEnName(std::string buildingEnName);
		void setLatitude(double latitude);
		void setLongitude(double longitude);
		void setAltitude(double altitude);
		void setGPSLoc(double lati, double longi, double alti);

		void print();
	};

	class Surface {
	public:
		int surfaceSeq;
		double topLeftLatitude, topLeftLongitude, topLeftAltitude,
			botLeftLatitude, botLeftLongitude, botLeftAltitude,
			botRightLatitude, botRightLongitude, botRightAltitude,
			topRightLatitude, topRightLongitude, topRightAltitude;
		char buildingId[4];

		enum class Column {
			SURFACE_SEQ, TOPLEFT_LATITUDE, TOPLEFT_LONGITUDE, TOPLEFT_ALTITUDE,
			BOTLEFT_LATITUDE, BOTLEFT_LONGITUDE, BOTLEFT_ALTITUDE,
			BOTRIGHT_LATITUDE, BOTRIGHT_LONGITUDE, BOTRIGHT_ALTITUDE,
			TOPRIGHT_LATITUDE, TOPRIGHT_LONGITUDE, TOPRIGHT_ALTITUDE, BUILDING_ID,
			TOPLEFT_GPS, TOPRIGHT_GPS, BOTLEFT_GPS, BOTRIGHT_GPS
		};
		static const char* ColumnToStr(Column col);
		
	private:
		void _setVar(Column col, int integer);
		void _setVar(Column col, double decimal);
		void _setVar(Column col, const char* pchar);
		void _setVar(Column col, std::string str);
		void _setVar(Column col, GPS gps);
		void _setVar(Column col, const mysqlx::Value& val);

	public:
		Surface() { varInit(); }
		Surface(int surfaceSeq, std::string buildingId);
		template<typename ... Types>
		Surface(Types ... args) {
			varInit();
			setVar(args...);
		}

		template<typename T, typename ... Types>
		void setVar(Column col, T val, Types ... args) {
			_setVar(col, val);
			setVar(args...);
				
		}
		void setVar() {}

		void varInit();
		void setSurfaceSeq(int seq) { surfaceSeq = seq; }
		void setTopLeftGPS(double lati, double longi, double alti) { 
			topLeftAltitude = lati; topLeftLongitude = longi; topLeftAltitude = alti;
		}
		void setTopLeftGPS(GPS gps) {
			topLeftLatitude = gps.latitude;
			topLeftLongitude = gps.longitude;
			topLeftAltitude = gps.altitude;
		}
		void setTopRightGPS(double lati, double longi, double alti) {
			topRightLatitude = lati; topRightLongitude = longi; topRightAltitude = alti;
		}
		void setTopRightGPS(GPS gps) {
			topRightLatitude = gps.latitude;
			topRightLongitude = gps.longitude;
			topRightAltitude = gps.altitude;
		}
		void setBotRightGPS(double lati, double longi, double alti) {
			botRightLatitude = lati; botRightLongitude = longi; botRightAltitude = alti;
		}
		void setBotRightGPS(GPS gps) {
			botRightLatitude = gps.latitude;
			botRightLongitude = gps.longitude;
			botRightAltitude = gps.altitude;
		}
		void setBotLeftGPS(double lati, double longi, double alti) {
			botLeftLatitude = lati; botLeftLongitude = longi; botLeftAltitude = alti;
		}
		void setBotLeftGPS(GPS gps) {
			botLeftLatitude = gps.latitude;
			botLeftLongitude = gps.longitude;
			botLeftAltitude = gps.altitude;
		}
		void setTopLeftLati(double lati) { topLeftLatitude = lati; }
		void setTopLeftLongi(double longi) { topLeftLongitude = longi; }
		void setTopLeftAlti(double alti) { topLeftAltitude = alti; }
		void setTopRightLati(double lati) { topRightLatitude = lati; }
		void setTopRightLongi(double longi) { topRightLongitude = longi; }
		void setTopRightAlti(double alti) { topRightAltitude = alti; }
		void setBotRightLati(double lati) { botRightLatitude = lati; }
		void setBotRightLongi(double longi) { botRightLongitude = longi; }
		void setBotRightAlti(double alti) { botRightAltitude = alti; }
		void setBotLeftLati(double lati) { botLeftLatitude = lati; }
		void setBotLeftLongi(double longi) { botLeftLongitude = longi; }
		void setBotLeftAlti(double alti) { botLeftAltitude = alti; }
		void setBuildingId(std::string str);

		void print();
	};

	class Marker {
	public:
		int markerSeq;
		double latitude, longitude, altitude;
		int surfaceSeq;
		char buildingId[4];
		wstring markerName;

		enum class Column {
			MARKER_SEQ, LATITUDE, LONGITUDE, ALTITUDE,
			SURFACE_SEQ, BUILDING_ID, MARKER_NAME, GPS
		};
		static const char* ColumnToStr(Column col);

		template<typename T, typename ... Types>
		void setVar(Column col, T val, Types ... args) {
			_setVar(col, val);
			setVar(args...);
		}
		void setVar() {}

	private:
		void _setVar(Column col, int val);
		void _setVar(Column col, double val);
		void _setVar(Column col, const char* pchar);
		void _setVar(Column col, const wchar_t* pwchar);
		void _setVar(Column col, std::string str);
		void _setVar(Column col, wstring wstr);
		void _setVar(Column col, GPS gps);
		void _setVar(Column col, const mysqlx::Value& val);

	public:
		Marker() { varInit(); }
		Marker(int markerSeq, int surfaceSeq, std::string buildingId);
		template<typename ... Types>
		Marker(Types ... args) {
			varInit();
			setVar(args...);
		}

		void varInit();
		void setMarkerSeq(int seq) { markerSeq = seq; }
		void setGPS(double lati, double longi, double alti) {
			latitude = lati; longitude = longi; altitude = alti;
		}
		void setGPS(GPS gps) {
			latitude = gps.latitude;
			longitude = gps.longitude;
			altitude = gps.altitude;
		}
		void setLatitude(double lati) { latitude = lati; }
		void setLongitude(double longi) { longitude = longi; }
		void setAltitude(double alti) { altitude = alti; }
		void setSurfaceSeq(int seq) { surfaceSeq = seq; }
		void setBuildingId(std::string str);
		void setMarkerName(wstring wstr) { markerName = wstr; }

		void print(); 
	};

	GIS_DB(std::string hostname, int port, std::string user, std::string password, std::string schema) :
		DB(hostname, port, user, password, schema) {}
	
	static void RowToBuilding(const mysqlx::Row& row, Building& building, const vector<Building::Column>& selectedCols);
	static void RowToSurface(const mysqlx::Row& row, Surface& surface, const vector<Surface::Column>& selectedCols);
	static void RowToMarker(const mysqlx::Row& row, Marker& marker, const vector<Marker::Column>& selectedCols);	

	void selectFromBuilding(vector<Building*>& buildings, const mysqlx::string whereCond, int limit = -1);
	void selectFromSurface(vector<Surface*>& surfaces, const mysqlx::string whereCond, int limit = -1);
	void selectFromMarker(vector<Marker*>& markers, const mysqlx::string whereCond, int limit = -1);
};
using BuildingCol = GIS_DB::Building::Column;
using SurfaceCol = GIS_DB::Surface::Column;
using MarkerCol = GIS_DB::Marker::Column;

const vector<BuildingCol> buildingAllSelectList = { BuildingCol::BUILDING_ID, BuildingCol::BUILDING_NAME,
										BuildingCol::LATITUDE, BuildingCol::LONGITUDE,
										BuildingCol::ALTITUDE, BuildingCol::BUILDING_EN_NAME };
const vector<SurfaceCol> SurfaceAllSelectList = {
	SurfaceCol::SURFACE_SEQ, SurfaceCol::TOPLEFT_LATITUDE, SurfaceCol::TOPLEFT_LONGITUDE, SurfaceCol::TOPLEFT_ALTITUDE,
	SurfaceCol::BOTLEFT_LATITUDE, SurfaceCol::BOTLEFT_LONGITUDE, SurfaceCol::BOTLEFT_ALTITUDE,
	SurfaceCol::BOTRIGHT_LATITUDE, SurfaceCol::BOTRIGHT_LONGITUDE, SurfaceCol::BOTRIGHT_ALTITUDE,
	SurfaceCol::TOPRIGHT_LATITUDE, SurfaceCol::TOPRIGHT_LONGITUDE, SurfaceCol::TOPRIGHT_ALTITUDE, SurfaceCol::BUILDING_ID
};

void mysqlTest();
#endif