#ifndef MISSION_CONTROL_UTIL_H_
#define MISSION CONTROL_UTIL_H_

#include <string>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <deque>
#include <iterator>
#include <algorithm>

// 08.10.2018, Peter - Fixed the circle/square collision detection (I was being dumb...)

inline std::deque<std::string> util_string_split(std::string s, const char delimiter) {
	std::deque<std::string> output;
	if (s.size() > 1 && delimiter != NULL) {
		size_t start = 0;
		size_t end = s.find_first_of(delimiter);

		while (end <= std::string::npos) {
			std::string substr = s.substr(start, end - start);
			output.emplace_back(substr);
			if (end == std::string::npos)
				break;

			start = end + 1;
			end = s.find_first_of(delimiter, start);
		}
	}
	else {
		std::cout << "ERROR: INVALID STRING SPLIT" << std::endl;
	}

	return output;
}

/****
*	Stolen from https://stackoverflow.com/questions/17261217/is-there-an-elegant-way-to-transfer-a-subset-of-one-deque-to-another
*	By Peter, 22/08/2018
****/

template <typename ForwardIt>
inline std::deque<std::string> util_extractDeque(ForwardIt from, ForwardIt to){
	using std::make_move_iterator;

	std::deque<std::string> d2(make_move_iterator(from),make_move_iterator(to));
	std::fill(from, to, 0);

	return d2;
}

/**
* Code lifted from: https://www.movable-type.co.uk/scripts/latlong-os-gridref.html
*
* CONVERTS FROM OSGB36 DATUM NOT GPS DATUM (WGS84) - i.e. close but not perfect (and fixable)
*
* Converts latitude/longitude to Ordnance Survey grid reference easting/northing coordinate.
*
* Note formulation implemented here due to Thomas, Redfearn, etc is as published by OS, but is
* inferior to Kruger as used by e.g. Karney 2011.
*
* @param   {LatLon}    point - latitude/longitude.
* @returns {OsGridRef} OS Grid Reference easting/northing.
*
* @example
*   var p = new LatLon(52.65798, 1.71605);
*   var grid = OsGridRef.latLonToOsGrid(p); // grid.toString(): TG 51409 13177
*   // for conversion of (historical) OSGB36 latitude/longitude point:
*   var p = new LatLon(52.65757, 1.71791, LatLon.datum.OSGB36);
*/
inline glm::vec2 util_latLongToOsGrid(glm::vec2 latLong) {

	// if necessary convert to OSGB36 first
	//if (point.datum != LatLon.datum.OSGB36) point = point.convertDatum(LatLon.datum.OSGB36);

	double phi = glm::radians(latLong.x);
	double lambda = glm::radians(latLong.y);

	double a = 6377563.396;
	double b = 6356256.909;              // Airy 1830 major & minor semi-axes
	double F0 = 0.9996012717;                             // NatGrid scale factor on central meridian
	double phi0 = glm::radians(49.0f);
	double lambda0 = glm::radians(-2.0f);  // NatGrid true origin is 49N,2W
	double N0 = -100000;
	double E0 = 400000;                     // northing & easting of true origin, metres
	double e2 = 1 - (b*b) / (a*a);                          // eccentricity squared
	double n = (a - b) / (a + b);
	double n2 = n*n;
	double n3 = n*n*n;         // n, n^2, n^3

	double cosPhi = cos(phi);
	double sinPhi = sin(phi);
	double ν = a*F0 / sqrt(1 - e2*sinPhi*sin(sinPhi));            // nu = transverse radius of curvature
	double rho = a*F0*(1 - e2) / pow(1 - e2*sinPhi*sinPhi, 1.5); // rho = meridional radius of curvature
	double eta2 = ν / rho - 1; //CHECK                                    // eta = ?

	double Ma = (1 + n + (5 / 4)*n2 + (5 / 4)*n3) * (phi - phi0);
	double Mb = (3 * n + 3 * n*n + (21 / 8)*n3) * sin(phi - phi0) * cos(phi + phi0);
	double Mc = ((15 / 8)*n2 + (15 / 8)*n3) * sin(2 * (phi - phi0)) * cos(2 * (phi + phi0));
	double Md = (35 / 24)*n3 * sin(3 * (phi - phi0)) * cos(3 * (phi + phi0));
	double M = b * F0 * (Ma - Mb + Mc - Md);              // meridional arc

	double cos3phi = cosPhi*cosPhi*cosPhi;
	double cos5phi = cos3phi*cosPhi*cosPhi;
	double tan2phi = tan(phi)*tan(phi);
	double tan4phi = tan2phi*tan2phi;

	double I = M + N0;
	double II = (ν / 2)*sinPhi*cosPhi;
	double III = (ν / 24)*sinPhi*cos3phi*(5 - tan2phi + 9 * eta2);
	double IIIA = (ν / 720)*sinPhi*cos5phi*(61 - 58 * tan2phi + tan4phi);
	double IV = ν*cosPhi;
	double V = (ν / 6)*cos3phi*(ν / rho - tan2phi);
	double VI = (ν / 120) * cos5phi * (5 - 18 * tan2phi + tan4phi + 14 * eta2 - 58 * tan2phi*eta2);

	double deltaLambda = lambda - lambda0;
	double deltaLambda2 = deltaLambda*deltaLambda, deltaLambda3 = deltaLambda2*deltaLambda, deltaLambda4 = deltaLambda3*deltaLambda, deltaLambda5 = deltaLambda4*deltaLambda, deltaLambda6 = deltaLambda5*deltaLambda;

	double N = I + II*deltaLambda2 + III*deltaLambda4 + IIIA*deltaLambda6;
	double E = E0 + IV*deltaLambda + V*deltaLambda3 + VI*deltaLambda5;

	//N = Number(N.toFixed(3)); // round to mm precision
	//E = Number(E.toFixed(3));

	return glm::vec2(E, N); // gets truncated to SW corner of 1m grid square
};


/**
* Converts ‘this’ numeric grid reference to standard OS grid reference.
*
* @param   {number} [digits=10] - Precision of returned grid reference (10 digits = metres);
*   digits=0 will return grid reference in numeric format.
* @returns {string} This grid reference in standard format.
*
* @example
*   var ref = new OsGridRef(651409, 313177).toString(); // TG 51409 13177
*/
inline std::string util_EastingNorthingToGridLetter(glm::vec2 digits, bool numericalPrecision = false) {

	double e = digits.x;
	double n = digits.y;
	//Fudge for glm sometimes having strange behaviour for zeros
	if (e == 0) {
		e = 0.001;
	}
	if (n == 0) {
		n = 0.001;
	}

	// get the 100km-grid indices
	double e100k = floor(e / 100000), n100k = floor(n / 100000);

	//if (e100k<0 || e100k>6 || n100k<0 || n100k>12) return '';

	// translate those into numeric equivalents of the grid letters
	double l1 = (19 - n100k) - int(19 - n100k) % 5 + floor((e100k + 10) / 5);
	double l2 = int(19 - n100k) * 5 % 25 + int(e100k) % 5;

	// compensate for skipped 'I' and build grid letter-pairs
	if (l1 > 7) l1++;
	if (l2 > 7) l2++;
	char letter1 = char(int(l1 + int('A')));
	char letter2 = char(int(l2 + int('A')));
	std::string letterPair;
	letterPair.push_back(letter1);
	letterPair.push_back(letter2);

	if (numericalPrecision) {
		// strip 10km-grid indices from easting & northing
		e = floor((int(e) % 100000)) / 10000;
		n = floor((int(n) % 100000)) / 10000;
		return letterPair + std::to_string(int(e)) + std::to_string(int(n));
	}
	else {
		return letterPair;
	}
};

inline std::string util_get_JSON_Field(std::deque<std::string> json, std::string field) {
	if (json.size() > 0) {
		for (unsigned int i = 0; i < json.size(); i++) {
			if (json[i].size() > 0) {
				std::size_t found = json[i].find(field);
				if (found != std::string::npos) { //if this value claims to exist within this section of the array
					std::deque <std::string> split = util_string_split(json[i], ':'); //split at the colon
					if (split.size() == 2) {
						std::string result = split[1];
						if (result.size() > 0) {
							if (result[0] == '"') {
								result = result.substr(1, std::string::npos);
							}
							if (result.size() > 0) {
								if (result[result.length() - 1] == '"') {
									result = result.substr(0, result.size() - 1);
								}
								return result;
							}
							else {
								return "";
							}
						}
					}
					else {
						std::cout << "ERROR INVALID JSON FIELD '" << field << "'" << std::endl;
						return "";
					}
				}
			}
		}
	}
	else {
		std::cout << "ERROR INVALID JSON ARRAY" << std::endl;
		return "";
	}
	std::cout << "ERROR DID NOT FIND JSON FIELD" << std::endl;
	return "";
}

/********************************************************************
*	utility function: is circle within rect
*	Determines if a circle is contained (either partially or
*	completely) within a rectangle
*	Written by Peter Naylor 02/10/2018
*********************************************************************/
inline bool util_isCircleWithinRect(glm::vec2 circleCentre,float radius, glm::vec2 topLeftCorner,glm::vec2 bottomRightCorner) {
	// First, is the origin contained within the square?
	//std::cout << "X: " << circleCentre.x << ">" << topLeftCorner.x << "; " << circleCentre.x << "<" << bottomRightCorner.x << std::endl;
	//std::cout << "Y: " << circleCentre.y << ">" << topLeftCorner.y << "; " << circleCentre.y << "<" << bottomRightCorner.y << std::endl;
	if ((circleCentre.x > topLeftCorner.x && circleCentre.x < bottomRightCorner.x) || (circleCentre.x < topLeftCorner.x && circleCentre.x > bottomRightCorner.x)) { // We check both to be idiot-proof
		if ((circleCentre.y > topLeftCorner.y && circleCentre.y < bottomRightCorner.y) || (circleCentre.y < topLeftCorner.y && circleCentre.y > bottomRightCorner.y)) {
			return true;
		}
	}
	// Still not within the rectangle? Check to see if the circle intersects any sides of the rectangle
	// Start with the vertical sides
	// is the inside of the square root positive?
	float sqrtContents = pow(radius,2) - pow((topLeftCorner.x - circleCentre.x),2);
	if (sqrtContents >= 0) { //it intercepts somewhere! But where?
		float yIntersectSqrt = sqrt(sqrtContents);
		float yIntersect1 = circleCentre.y + yIntersectSqrt;
		if ((yIntersect1 < topLeftCorner.y && yIntersect1 > bottomRightCorner.y) || (yIntersect1 > topLeftCorner.y && yIntersect1 < bottomRightCorner.y)) {
			return true; //It's within the extent of the line segment
		}
		else { // try the negative solution
			float yIntersectSqrt = sqrt(sqrtContents);
			float yIntersect2 = circleCentre.y - yIntersectSqrt;
			if ((yIntersect2 < topLeftCorner.y && yIntersect2 > bottomRightCorner.y) || (yIntersect2 > topLeftCorner.y && yIntersect2 < bottomRightCorner.y)) {
				return true; //It's within the extent of the line segment
			}
		}
	}
	// Still here? We'll try the other vertical side...
	sqrtContents = pow(radius, 2) - pow((bottomRightCorner.x - circleCentre.x), 2);
	if (sqrtContents >= 0) { //it intercepts somewhere! But where?
		float yIntersectSqrt = sqrt(sqrtContents);
		float yIntersect1 = circleCentre.y + yIntersectSqrt;
		if ((yIntersect1 < topLeftCorner.y && yIntersect1 > bottomRightCorner.y) || (yIntersect1 > topLeftCorner.y && yIntersect1 < bottomRightCorner.y)) {
			return true; //It's within the extent of the line segment
		}
		else { // try the negative solution
			float yIntersectSqrt = sqrt(sqrtContents);
			float yIntersect2 = circleCentre.y - yIntersectSqrt;
			if ((yIntersect2 < topLeftCorner.y && yIntersect2 > bottomRightCorner.y) || (yIntersect2 > topLeftCorner.y && yIntersect2 < bottomRightCorner.y)) {
				return true; //It's within the extent of the line segment
			}
		}
	}

	//Horizontal sides!
	sqrtContents = pow(radius, 2) - pow((topLeftCorner.y - circleCentre.y), 2);
	if (sqrtContents >= 0) { //it intercepts somewhere! But where?
		float xIntersectSqrt = sqrt(sqrtContents);
		float xIntersect1 = circleCentre.x + xIntersectSqrt;
		if ((xIntersect1 < topLeftCorner.x && xIntersect1 > bottomRightCorner.x) || (xIntersect1 > topLeftCorner.x && xIntersect1 < bottomRightCorner.x)) {
			return true; //It's within the extent of the line segment
		}
		else { // try the negative solution
			float xIntersectSqrt = sqrt(sqrtContents);
			float xIntersect2 = circleCentre.x - xIntersectSqrt;
			if ((xIntersect2 < topLeftCorner.x && xIntersect2 > bottomRightCorner.x) || (xIntersect2 > topLeftCorner.x && xIntersect2 < bottomRightCorner.x)) {
				return true; //It's within the extent of the line segment
			}
		}
	}
	// Really still here? We'll try the other horizontal side...
	sqrtContents = pow(radius, 2) - pow((bottomRightCorner.y - circleCentre.y), 2);
	if (sqrtContents >= 0) { //it intercepts somewhere! But where?
		float xIntersectSqrt = sqrt(sqrtContents);
		float xIntersect1 = circleCentre.x + xIntersectSqrt;
		if ((xIntersect1 < topLeftCorner.x && xIntersect1 > bottomRightCorner.x) || (xIntersect1 > topLeftCorner.x && xIntersect1 < bottomRightCorner.x)) {
			return true; //It's within the extent of the line segment
		}
		else { // try the negative solution
			float xIntersectSqrt = sqrt(sqrtContents);
			float xIntersect2 = circleCentre.x - xIntersectSqrt;
			if ((xIntersect2 < topLeftCorner.x && xIntersect2 > bottomRightCorner.x) || (xIntersect2 > topLeftCorner.x && xIntersect2 < bottomRightCorner.x)) {
				return true; //It's within the extent of the line segment
			}
		}
	}
	return false; // NOPE!
}

/********************************************************************
*	utility function: is rect within circle
*	Determines if a rectangle is contained (either partially or
*	completely) within a circle
*	Written by Peter Naylor 02/10/2018
*********************************************************************/
inline bool util_isRectWithinCircle(glm::vec2 circleCentre, float radius, glm::vec2 topLeftCorner, glm::vec2 bottomRightCorner) {
	// First check if any vertices are within the circle:
	glm::vec2 topLeftRelPos(topLeftCorner.x - circleCentre.x, topLeftCorner.y - circleCentre.y);
	if (pow(topLeftRelPos.x, 2) + pow(topLeftRelPos.y, 2) < pow(radius, 2)) {
		return true;
	}
	glm::vec2 bottomRightRelPos(bottomRightCorner.x - circleCentre.x, bottomRightCorner.y - circleCentre.y);
	if (pow(bottomRightRelPos.x, 2) + pow(bottomRightRelPos.y, 2) < pow(radius, 2)) {
		return true;
	}
	if (pow(topLeftRelPos.x, 2) + pow(bottomRightRelPos.y, 2) < pow(radius, 2)) {
		return true;
	}
	if (pow(bottomRightRelPos.x, 2) + pow(topLeftRelPos.y, 2) < pow(radius, 2)) {
		return true;
	}
	if (util_isCircleWithinRect(circleCentre, radius, topLeftCorner, bottomRightCorner)) {
		return true;
	}
	return false;
}


#endif // !MISSION_CONTROL_UTIL_H_
