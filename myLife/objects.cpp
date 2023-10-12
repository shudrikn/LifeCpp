#include "objects.h"

uint64_t Object::id_counter = 0;
uint64_t Cell::chainIdCounter = 1;

bool Cell::colorIsFoodType = false;
bool Cell::colorIsChain = false;

void Food::execute()
{
    if (needToFall)
    {
        if (!World::world.moveObject(point, Point(point.x(), point.y() + 1)))
        {
            needToFall = false;
        }
    }
}

Cell::Cell(const Genom& genom, const Point& point, uint8_t energy, uint64_t chain_id) :
    Object(World::CELL, point), 
    genom(genom), 
    energy(energy),
    chainId(chain_id),
    currentType(Unknown),
    energyFromGenerate(0),
    energyFromFood(0),
    energyFromHunting(0),
    mutationHistoricalProp(20),
    chainHistoricalProp(2)
{
    this->genom.cell = this;
}

void Cell::changeEnergy(int en)
{
    if (en > 9 && chainId)
    {
        uint8_t energyToShare = en / 9;

        share(energyToShare);
    }

    energy += en;

    if (energy <= 0)
    {
        World::world.addFood(point, energyToDivide);
    }

    if (energy > Cell::maxEnergy)
    {
        if (!divide(mutationHistoricalProp, chainHistoricalProp))
            energy = Cell::maxEnergy;
    }
}

void Cell::execute()
{
    changeEnergy(energyPerStep);

    if (energy <= 0)
    {
        return;
    }

    if (rand() % 100 == 1)
        genom.mutate();

    genom.execute();
}

void Cell::generate()
{
    int8_t en = World::world.energyOnPoint(point);

    changeEnergy(en);
    energyFromGenerate += en;
};

bool Cell::bite(const Direction& direction)
{
    Point selectedPoint(point, direction);

    auto enemy = World::world.getCell(selectedPoint);
    if (enemy)
    {
        short int en;
        en = min(enemy->energy, energy); // сила укуса - не больше, чем сила самой клетки
                
     
        changeEnergy(en);
        enemy->changeEnergy(en * -1);


        energyFromHunting += en;

        return true;
    }
    return false;
}

bool Cell::share(uint8_t energyToShare)
{
    bool result = false;

    if (!chainId)
        return result;

    for (uint8_t d = 0; d < 8; ++d)
    {
        Direction dir(d);
        Point selectedPoint(point, dir);
        if (energy > energyToShare)
        {
            auto neighbour = World::world.getCell(selectedPoint);
            if (neighbour)
            {
                bool chain = getChainId() == neighbour->getChainId();
                if (chain)
                {
                    neighbour->changeEnergy(energyToShare);
                    energy -= energyToShare;

                    result = true;
                }
            }
        }
    }
    return result;
}

bool Cell::eatFood(const Direction& direction)
{
    Point selectedPoint(point, direction);
    auto food = World::world.getFood(selectedPoint);

    if (food)
    {
        auto en = food->use();
        changeEnergy(en);
        energyFromFood += en;

        return true;
    }
    return false;
}

bool Cell::divide(uint8_t mutationProp, uint8_t chainProp)
{
    if (energy > energyToDivide + 1)
    {
        for (uint8_t i = 0; i < 4; ++i)
        {
            Point pointToClone(point, rand()%8);

            if (World::world.getObjectType(pointToClone) == World::EMPTY)
            {
                energy -= energyToDivide;
                energy = energy / 2;
                bool mutation = mutationProp >= (rand() % 100);
                auto newGenom = genom;
                if (mutation)
                {
                    newGenom.mutate();
                }
                bool chain = chainProp >= (rand() % 100);
                uint64_t newChainId = chainId;
                if (chain)
                {
                    newChainId = chainIdCounter;
                    ++chainIdCounter;

                    // если уже многоклеточное - есть вероятность стать одноклеточным
                    if (chainId)
                    {
                        bool solo = 50 >= (rand() % 100);
                        if (solo)
                        {
                            newChainId = 0;
                        }
                    }
                }

                auto newCell = World::world.addCell(newGenom, pointToClone, energy, newChainId);
                newCell->chainHistoricalProp = chainProp;
                newCell->mutationHistoricalProp = mutationProp;

                return true;
            }
        }
    }
    return false;
}

Cell::Type Cell::getFoodType()
{
    if (energyFromFood > energyFromHunting)
        if (energyFromFood > energyFromGenerate)
            return Carrion;
        else
            return Herbivorous;
    else
        if (energyFromHunting > energyFromGenerate)
            return Predator;
        else
            return Herbivorous;
}

uint8_t Food::use()
{
    uint8_t e = energy;
    energy = 0;
    World::world.eraseField(point);
    return e;
}

void Food::print() const
{
    SetConsoleTextAttribute(World::world.consoleHeader, 8);
    std::cout << "*";
}

uint8_t Cell::getColor() const
{
    if (colorIsFoodType)
        return currentType;
    else if (colorIsChain)
        if (chainId == 0)
            return 0xF;
        else
            return 0xE - chainId % 13;
    else
        return 0xF;
}

void Cell::print() const
{
    SetConsoleTextAttribute(World::world.consoleHeader, getColor());
    std::cout << static_cast<char>(4);
}