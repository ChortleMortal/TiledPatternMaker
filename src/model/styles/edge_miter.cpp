/*
 * This is code scrapes from the internet to be looked at in the future
 * /

#if 1
double calculateAngle(const QPointF& a, const QPointF& centroid)
{
    return qAtan2(a.y() - centroid.y(), a.x() - centroid.x());
}

bool comparePoints(const QPointF& a, const QPointF& b, const QPointF& centroid)
{
    double angleA = calculateAngle(a, centroid);
    double angleB = calculateAngle(b, centroid);
    return angleA < angleB;
}
#endif



#define MITER
#ifdef MITER

#include <iostream>
#include <cmath>

struct Point {
    double x, y;
};

struct Line {
    Point start, end;
};

Point calculateIntersection(Point A, Point B, Point C, Point D) {
    double a1 = B.y - A.y;
    double b1 = A.x - B.x;
    double c1 = a1 * A.x + b1 * A.y;

    double a2 = D.y - C.y;
    double b2 = C.x - D.x;
    double c2 = a2 * C.x + b2 * C.y;

    double determinant = a1 * b2 - a2 * b1;

    if (determinant == 0) {
        throw std::runtime_error("Lines are parallel");
    } else {
        double x = (b2 * c1 - b1 * c2) / determinant;
        double y = (a1 * c2 - a2 * c1) / determinant;
        return {x, y};
    }
}

Point calculateMiterJoin(Line line1, Line line2, double miterLength) {
    Point intersection = calculateIntersection(line1.start, line1.end, line2.start, line2.end);

    double angle1 = atan2(line1.end.y - line1.start.y, line1.end.x - line1.start.x);
    double angle2 = atan2(line2.end.y - line2.start.y, line2.end.x - line2.start.x);

    double bisectAngle = (angle1 + angle2) / 2.0;

    Point miterPoint = {
        intersection.x + miterLength * cos(bisectAngle),
        intersection.y + miterLength * sin(bisectAngle)
    };

    return miterPoint;
}

int main() {
    Line line1 = {{0, 0}, {1, 1}};
    Line line2 = {{1, 1}, {2, 0}};
    double miterLength = 1.0;

    try {
        Point miterPoint = calculateMiterJoin(line1, line2, miterLength);
        std::cout << "Miter Point: (" << miterPoint.x << ", " << miterPoint.y << ")\n";
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}

#endif
///////////////////////////////////////////////////////////

#define BEVELED
#ifdef BEVELED

#include <iostream>
#include <cmath>
#include <utility>

// Structure to represent a point
struct Point {
    double x, y;
};

// Function to calculate the join point of two beveled lines
Point calculateJoinPoint(const Point& p1, const Point& p2, const Point& p3, double bevelDistance) {
    // Calculate direction vectors
    double dx1 = p2.x - p1.x;
    double dy1 = p2.y - p1.y;
    double dx2 = p3.x - p2.x;
    double dy2 = p3.y - p2.y;

    // Normalize direction vectors
    double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
    double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);
    dx1 /= len1;
    dy1 /= len1;
    dx2 /= len2;
    dy2 /= len2;

    // Calculate the bevel points
    Point bevelPoint1 = { p2.x - bevelDistance * dx1, p2.y - bevelDistance * dy1 };
    Point bevelPoint2 = { p2.x - bevelDistance * dx2, p2.y - bevelDistance * dy2 };

    // Calculate the join point (intersection of the two lines)
    double a1 = dy1;
    double b1 = -dx1;
    double c1 = a1 * bevelPoint1.x + b1 * bevelPoint1.y;

    double a2 = dy2;
    double b2 = -dx2;
    double c2 = a2 * bevelPoint2.x + b2 * bevelPoint2.y;

    double det = a1 * b2 - a2 * b1;
    if (std::abs(det) < 1e-10) {
        // Lines are parallel, return the midpoint as an approximation
        return { (bevelPoint1.x + bevelPoint2.x) / 2, (bevelPoint1.y + bevelPoint2.y) / 2 };
    } else {
        // Calculate the intersection point
        double x = (b2 * c1 - b1 * c2) / det;
        double y = (a1 * c2 - a2 * c1) / det;
        return { x, y };
    }
}

int main() {
    Point p1 = {0, 0};
    Point p2 = {1, 1};
    Point p3 = {2, 0};
    double bevelDistance = 0.1;

    Point joinPoint = calculateJoinPoint(p1, p2, p3, bevelDistance);
    std::cout << "Join Point: (" << joinPoint.x << ", " << joinPoint.y << ")\n";

    return 0;
}

/*
Explanation:
Point Structure: Defines a simple structure to represent a point with x and y coordinates.
calculateJoinPoint Function: Computes the join point of two beveled lines.
Direction Vectors: Calculates the direction vectors for the lines.
Normalization: Normalizes these vectors.
Bevel Points: Computes the bevel points by moving back along the direction vectors by the bevel distance.
Intersection Calculation: Uses the line equations to find the intersection point of the two lines formed by the bevel points.
Main Function: Demonstrates the usage of the calculateJoinPoint function with sample points and bevel distance.

Feel free to adapt this code to fit your specific requirements!
*/
#endif

#define MULTI_BEVEL
#ifdef MULTI_BEVEL
#include <cmath>
#include <vector>
#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;

int main() {
    std::vector<double> angles = {1.0, 2.0, 2.5, 3.0, 5.0}; // list of angles corresponding to your red segments (middle of your walls)
    std::vector<double> wall_len = {0.05, 0.1, 0.02, 0.08, 0.1}; // list of half-width of your walls
    int nb_walls = angles.size();

    for (auto a : angles) {
        plt::plot({0, std::cos(a)}, {0, std::sin(a)}, "r", {{"linewidth", 3}}); // plotting the red segments
    }

    for (int i = 0; i < angles.size(); i++) {
        int j = (i + 1) % nb_walls;
        double angle_n = angles[i]; // get angle Θ_n
        double angle_np = angles[j]; // get the angle Θ_n+1 (do not forget to loop to the first angle when you get to the last angle in the list)
        double wall_n = wall_len[i]; // get the thickness of the n-th wall t_n
        double wall_np = wall_len[j]; // get the thickness of the n+1-th wall t_n+1
        double dif_angle = angle_np - angle_n; // ΔΘ
        double t1 = wall_n / std::sin(dif_angle); // width of the rhombus
        double t2 = wall_np / std::sin(dif_angle); // height of the rhombus

        double x = t2 * std::cos(angle_n) + t1 * std::cos(angle_np); // x coordinates of the intersection point
        double y = t2 * std::sin(angle_n) + t1 * std::sin(angle_np); // y coordinates of the intersection point

        std::vector<double> wall_n_vec = {std::cos(angle_n), std::sin(angle_n)}; // for plotting n-th wall
        std::vector<double> wall_np_vec = {std::cos(angle_np), std::sin(angle_np)}; // for plotting n+1-th wall

        plt::plot({x, wall_n_vec[0] + x}, {y, wall_n_vec[1] + y}, "k"); // plotting the n wall
        plt::plot({x, wall_np_vec[0] + x}, {y, wall_np_vec[1] + y}, "k"); // plotting the n+1 wall
    }

    plt::show();
    return 0;
}
#endif
