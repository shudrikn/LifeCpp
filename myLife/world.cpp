#include <iomanip>
#include <fstream>

#include "world.h"
#include "objects.h"

World World::world;

const int8_t World::maxEnergyOnPoint = 10;
int8_t World::energyOnPoint(const Point& point)
{
    int energy = ((World::height - point.y()) * 10 / World::height);
    return energy;
}

World::World(): 
    map(width, std::vector<Object*>(height, nullptr)),
    time(0), 
    updateMap(true), 
    printMap(true), 
    logToFile(false), 
    consoleHeader(GetStdHandle(STD_OUTPUT_HANDLE)), 
    startTime(0), 
    lastTime(0)
{}

void World::addObjectToField(Object* obj)
{
    auto p = obj->getPoint();
    updatePointOnScreen(p);
    
    map[p.x()][p.y()] = obj;
}

void World::eraseField(const Point& point)
{
    map[point.x()][point.y()] = nullptr;
    updatePointOnScreen(point);

    Point p(point.x(), point.y() - 1);
    auto food = getFood(p);
    if (food)
    {
        food->needToFall = true;
    }
}

bool World::moveObject(const Point& from, const Point& to)
{
    auto from_ = from;
    if (getObjectType(to) == EMPTY)
    {
        auto object = getObject(from_);
        if (object)
        {
            object->point = to;

            addObjectToField(object);
            eraseField(from_);
            return true;
        }
    }

    return false;
}

void World::updatePointOnScreen(const Point& point) const
{
    if (printMap)
    {
        COORD position = { point.x(), point.y() + 1 };
        SetConsoleCursorPosition(consoleHeader, position);

        auto obj = getObject(point);
        if (obj)
            obj->print();
        else
            std::cout << ' ';
    }
}

Food* World::addFood(const Point& point, uint8_t energy)
{
    foods.emplace_back(point, energy);
    addObjectToField(&foods.back());
    return &foods.back();
}
Cell* World::addCell(const Genom& genom, const Point& point, uint8_t energy, size_t chainId)
{
    cells.emplace_back(genom, point, energy, chainId);
    addObjectToField(&cells.back());
    return &cells.back();
}

Cell* World::getCell(const Point& point) const
{
    if (getObjectType(point) == CELL)
        return dynamic_cast<Cell*>(getObject(point));
    return nullptr;
}
Food* World::getFood(const Point& point) const
{
    if (getObjectType(point) == FOOD)
        return dynamic_cast<Food*>(getObject(point));
    return nullptr;
}

World::ObjectType World::getObjectType(const Point& point) const
{
    int x = point.x();
    int y = point.y();

    if (y < 0 || y >= height)
        return WALL;

    if (!map[x][y])
        return EMPTY;

    return map[x][y]->getType();
}


void World::start(uint32_t end_time)
{
    startTime = clock();
    lastTime = clock();
    do
    {
        
        auto cell = cells.begin();
        while (!cells.empty() && cell != cells.end())
        {
            if (cell->getEnergy() > 0)
            {
                cell->execute();
                ++cell;
            }
            else
            {
                cells.erase(cell++);
            }
        }
        auto food = foods.begin();
        while (!foods.empty() && food != foods.end())
        {
            if (food->getEnergy() > 0)
            {
                food->execute();
                ++food;
            }
            else
            {
                foods.erase(food++);
            }
        }

        printAllMap();
        printInfo();

        ++time;
        //seasonEnergy = seasons[(time / 100000) % 4];
    } while (time!=end_time);
}

Point::Point(int x, int y) : x_(x), y_(y)
{
    while (x_ < 0)
        x_ += World::width;

    x_ = x_ % World::width;
}

Point::Point(const Point& point, const Direction& dir)
{
    x_ = point.x_ + dir.x;
    y_ = point.y_ + dir.y;

    while (x_ < 0)
        x_ += World::width;

    x_ = x_ % World::width;
}

void World::printInfo()
{
    if (time % 100 == 0)
    {
        SetConsoleTextAttribute(consoleHeader, 15);

        COORD position = { 0,0 };
        SetConsoleCursorPosition(consoleHeader, position);

        clock_t currentTime = clock();
        double fps = CLK_TCK * 100 / static_cast<double>(currentTime - lastTime);
        lastTime = currentTime;
        double averageFps = CLK_TCK * time / static_cast<double>(currentTime - startTime);

        std::cout << std::setprecision(4) << "Time: " << time << " Cells: " << cells.size() << " Food: " << foods.size() << " Fps: " << fps << " AverageFps: " << averageFps << "                 ";

        //std::ofstream fout;
        //fout.open("log.txt", std::ofstream::out | std::ofstream::app);
        //fout << time << " " << cells.size() << " "<< foods.size() << std::endl;
        //fout.close();
    }
}

void World::printAllMap()
{
    if (time % 100 == 0 || updateMap)
    {
        COORD position = { 0,1 };
        SetConsoleCursorPosition(consoleHeader, position);

        for (size_t j = 0; j < height; ++j)
        {
            for (size_t i = 0; i < width; ++i)
            {
                if (map[i][j])
                {
                    map[i][j]->print();
                }
                else
                    std::cout << " ";
            }
            std::cout << std::endl;
        }
        updateMap = false;
    }
}