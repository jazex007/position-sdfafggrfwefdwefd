#include <iostream>
#include <cmath>
#include <iomanip>
#include <windows.h>
#include <string>
#include <sstream>

const double EARTH_RADIUS = 6371000;

double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

void calculateNewPosition(double lat, double lon, double distance, double heading, double& newLat, double& newLon) {
    double delta = distance / EARTH_RADIUS;
    double brng = toRadians(heading);

    newLat = asin(sin(toRadians(lat)) * cos(delta) + cos(toRadians(lat)) * sin(delta) * cos(brng));
    newLon = toRadians(lon) + atan2(sin(brng) * sin(delta) * cos(toRadians(lat)),
        cos(delta) - sin(toRadians(lat)) * sin(newLat));

    newLat = newLat * 180.0 / M_PI;
    newLon = newLon * 180.0 / M_PI;
}

void calculateMeetingPoint(double lat1, double lon1, double heading1,
    double lat2, double lon2, double heading2,
    double& meetLat, double& meetLon) {
    double h1 = toRadians(heading1);
    double h2 = toRadians(heading2);

    double distance = 100.0;

    double newLat1, newLon1, newLat2, newLon2;

    calculateNewPosition(lat1, lon1, distance, heading1, newLat1, newLon1);
    calculateNewPosition(lat2, lon2, distance, heading2, newLat2, newLon2);

    meetLat = (newLat1 + newLat2) / 2;
    meetLon = (newLon1 + newLon2) / 2;
}

HANDLE initSerial(const std::string& portName) {
    HANDLE hSerial = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error in opening serial port" << std::endl;
        return NULL;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error in getting serial state" << std::endl;
        return NULL;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error in setting serial state" << std::endl;
        return NULL;
    }

    return hSerial;
}

std::string readLine(HANDLE hSerial) {
    char buffer[256];
    DWORD bytesRead;
    std::string result;

    while (true) {
        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            buffer[bytesRead] = '\0';
            result += buffer;

            if (result.find('\n') != std::string::npos) {
                break;
            }
        }
    }

    return result;
}

int main() {
    HANDLE hSerial = initSerial("COM3");
    if (!hSerial) {
        return 1;
    }

    double lat1, lon1, heading1;
    double lat2, lon2, heading2;

    std::cout << "Waiting for GPS data from micro:bit..." << std::endl;

    std::string input = readLine(hSerial);
    std::istringstream ss(input);
    ss >> lat1 >> lon1 >> heading1 >> lat2 >> lon2 >> heading2;

    double meetLat, meetLon;
    calculateMeetingPoint(lat1, lon1, heading1, lat2, lon2, heading2, meetLat, meetLon);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Meeting Point: Latitude = " << meetLat << ", Longitude = " << meetLon << std::endl;

    CloseHandle(hSerial);
    return 0;
}
