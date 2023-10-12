#pragma once

#include <vector>

#include "genom.h"
#include "world.h"

class Direction
{
public:
    Direction(uint8_t num)
    {
        x = 0; y = 0;
        if (num <= 2)
            y = -1;
        else if (num >= 4 && num <= 6)
            y = 1;

        if (num == 0 || num == 7 || num == 6)
            x = -1;
        else if (num >= 2 && num <= 4)
            x = 1;
    }
    
    Direction(short int x, short int y) : x(x), y(y) {};

    short int x;
    short int y;
};

class Object
{
    static uint64_t id_counter;
    uint64_t id;
protected:
    
    Point point;
    World::ObjectType type;

public:
    const static uint8_t energyToDivide = 40;

    Object(World::ObjectType type, const Point& point) : id(id_counter), point(point), type(type)
    {
        ++id_counter;
    };
    virtual ~Object() = 0 {};

    virtual void execute() = 0 {};

    inline World::ObjectType getType()
    {
        return type;
    }

    inline size_t getId()  { return id; }
    inline const Point& getPoint() const { return point; }

    virtual void print() const = 0 {};

    friend class World;
};

class Food : public Object
{
    uint8_t energy;
    
public:
    Food(const Point& point, const uint8_t energy = energyToDivide) : Object(World::FOOD, point), energy(energy), needToFall(true) {};
    ~Food() {};

    bool needToFall;

    inline uint8_t getEnergy()
    {
        return energy;
    }

    void execute() override;

    uint8_t use();

    void print() const override;
};

class Cell : public Object
{
    enum Type{ Carrion = 14, Predator = 4, Herbivorous = 2, Unknown = 15};
    static uint64_t chainIdCounter;

    Genom genom;
    int energy;
    uint64_t chainId;

    uint8_t currentType;
    inline uint8_t getColor() const;
    inline Type getFoodType();

    uint16_t energyFromGenerate;
    uint16_t energyFromFood;
    uint16_t energyFromHunting;

    uint8_t mutationHistoricalProp;
    uint8_t chainHistoricalProp;

public:
    const static uint8_t maxEnergy = 100;
    const static int energyPerStep = -5;

    static bool colorIsFoodType;
    static bool colorIsChain;

    Cell(const Genom& genom, const Point& point, uint8_t energy = 100, uint64_t chain_id = 0);
    ~Cell() {};

    void execute() override;

    inline int getEnergy() const
    {
        return energy;
    }

    void changeEnergy(int en);

    inline Genom& getGenom()
    {
        return genom;
    }
    inline unsigned long getChainId() const
    {
        return chainId;
    }

    void generate();
    bool bite(const Direction& direction);
    bool share(uint8_t energyToShare);
    bool eatFood(const Direction& direction);
    bool divide(uint8_t mutationProp, uint8_t chainProp);
    

    friend void Genom::execute();

    void print() const override;
};

