#ifndef GPS_POINT_H
#define GPS_POINT_H


namespace gps {

struct Point
{
    double latitude, longitude, height;
    unsigned timestamp;


public:
    Point(double latitude, double longitude, double height=0.0);
    double distanceTo(const Point& other);



};

} // namespace gps

#endif // GPS_POINT_H
