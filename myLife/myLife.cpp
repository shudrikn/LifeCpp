#include <ctime>

#include "world.h"
#include "genom.h"
#include "objects.h"

int main()
{
    srand(time(0));
    World::world = World();
    World::world.printMap = false;
    //Cell::colorIsFoodType = true;
    Cell::colorIsChain = true;
    Genom genom(std::vector<uint8_t>(50, 2));
    
    for (int i = 0; i < 100; ++i)
    {
        try
        {
            Point randomPoint(rand() % World::width, rand() % World::height);
            if (World::world.getObjectType(randomPoint) == World::EMPTY)
                World::world.addCell(genom, randomPoint);
        } 
        catch (std::runtime_error err)
        {}
    }
    for (int i = 0; i < 0; ++i)
    {
        try
        {
            Point randomPoint(rand() % World::width, rand() % World::height);
            if (World::world.getObjectType(randomPoint) == World::EMPTY)
                World::world.addFood(randomPoint, 50);
        }
        catch (std::runtime_error err)
        {}
    }

    World::world.start();
}