#pragma once

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <iostream>
#include <chrono>
#include <windows.h>

class Direction;
class Genom;
class Object;
class Food;
class Cell;

class Point
{
    int x_, y_;
public:
    Point(int x, int y);
    Point(const Point& point, const Direction& dir);
    inline int x() const { return x_; };
    inline int y() const { return y_; };
    inline int& x() { return x_; };
    inline int& y() { return y_; };
};

class World
{
public:

    static World world;

    enum ObjectType { EMPTY, WALL, FOOD, CELL };
    static const uint16_t height = 49;
    static const uint16_t width = 211;

private:
    static const std::vector<int8_t> seasons;

    std::vector<std::vector<Object*>> map;

    std::list<Food> foods;
    std::list<Cell> cells;

    uint32_t time;
    bool updateMap;

    void addObjectToField(Object* obj);

    void printInfo();
    inline void printAllMap();

    inline Object* getObject(const Point& point) const
    {
        return map[point.x()][point.y()];
    }

public:
    bool printMap;
    bool logToFile;
    HANDLE consoleHeader;
    clock_t startTime;
    clock_t lastTime;

    uint8_t seasonEnergy;

    static const int8_t maxEnergyOnPoint;
    int8_t energyOnPoint(const Point& point);

    World();
    
    inline ObjectType getObjectType(const Point& point) const;
    
    void eraseField(const Point& point);
    
    Food* addFood(const Point& point, uint8_t energy);
    Cell* addCell(const Genom& genom, const Point& point, uint8_t energy = 100, size_t chainId = 0);
    Cell* getCell(const Point& point) const;
    Food* getFood(const Point& point) const;

    bool moveObject(const Point& from, const Point& to);

    void updatePointOnScreen(const Point& point) const;
    inline uint32_t getTime() const { return time; };
    void start(uint32_t end_time = 0);

};