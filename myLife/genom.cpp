#include <sstream>

#include "genom.h"
#include "objects.h"

inline void Genom::moveIterator(uint16_t pos)
{
    auto newPos = currentPosition();
    newPos += pos;

    setIterator(newPos);
}

inline void Genom::setIterator(uint16_t pos)
{
    currentCommand = genomBytes.begin() + (pos % genomBytes.size());
}

void Genom::insertGen(uint16_t pos, uint8_t gen)
{
    uint16_t currentPos = currentCommand - genomBytes.begin();
    genomBytes.insert(genomBytes.begin() + pos, gen);

    if (pos > currentPos)
        currentCommand = genomBytes.begin() + currentPos;
    else
        currentCommand = genomBytes.begin() + currentPos + 1;
}

void Genom::deleteGen(uint16_t pos)
{
    uint16_t currentPos = currentCommand - genomBytes.begin();
    genomBytes.erase(genomBytes.begin() + pos);

    if (pos >= currentPos)
        currentCommand = genomBytes.begin() + currentPos;
    else
        currentCommand = genomBytes.begin() + currentPos - 1;

    if (currentCommand == genomBytes.end())
        currentCommand = genomBytes.begin();
}

void Genom::execute()
{
    bool finish = true;
    int i = 0;
    do
    {
        uint8_t command = *currentCommand % 14;

        // осмотреться (направление) -> (пусто, еда, стена, часть многоклеточного, другая клетка)
        if (command == 1)
        {
            finish = false;

            moveIterator(1);
            Direction direction = *currentCommand % 8;
            Point selectedPoint(cell->point, direction);
            World::ObjectType findedType = World::world.getObjectType(selectedPoint);

            switch (findedType)
            {
            case World::ObjectType::EMPTY:
                moveIterator(1);
                break;
            case World::ObjectType::FOOD:
                moveIterator(2);
                break;
            case World::ObjectType::WALL:
                moveIterator(3);
                break;
            case World::ObjectType::CELL:
                auto neighbour = World::world.getCell(selectedPoint);
                if (neighbour)
                {
                    bool chain = cell->getChainId() == neighbour->getChainId();
                    if (chain)
                        moveIterator(4);
                    else
                        moveIterator(5);
                }
                break;
            }
        }
        // фотосинтез
        else if (command == 2)
        {
            cell->generate();

            moveIterator(1);
            finish = true;
        }
        // двигаться (направление) -> (неудачно, удачно)
        else if (command == 3)
        {
            finish = false;

            moveIterator(1);
            Direction direction = *currentCommand % 8;

            if (cell->getChainId() == 0)
            {
                Point selectedPoint(cell->point, direction);

                if (World::world.moveObject(cell->point, selectedPoint))
                {
                    moveIterator(1);
                    finish = true;
                }
            }

            moveIterator(1);
        }
        // кусать (направление) -> (неудачно, удачно)
        else if (command == 4)
        {
            finish = false;

            moveIterator(1);
            Direction direction = *currentCommand % 8;

            if (cell->bite(direction))
            {
                moveIterator(1);
                finish = true;
            }

            moveIterator(1);
        }
        // съесть еду (направление) -> (неудачно, удачно)
        else if (command == 5)
        {
            finish = false;

            moveIterator(1);
            Direction direction = *currentCommand % 8;

            if (cell->eatFood(direction))
            {
                moveIterator(1);
                finish = true;
            }

            moveIterator(1);
        }
        //делиться (вероятность мутации, вероятность создать новый многоклеточный организм) -> (неудачно, удачно)
        else if (command == 6)
        {
            finish = false;

            moveIterator(1);
            uint8_t mutationProp = (static_cast<double>(*currentCommand) / maxGenValue);
            moveIterator(1);
            uint8_t chainProp = (static_cast<double>(*currentCommand) / maxGenValue);

            if (cell->divide(mutationProp, chainProp))
            {
                moveIterator(1);
                finish = true;
            }

            moveIterator(1);
        }
        // узнать свой уровень энергии () -> (низкий, средний, высокий)
        else if (command == 7)
        {
            if (cell->getEnergy() < Cell::maxEnergy / 3 )
                moveIterator(1);
            else if (cell->getEnergy() < (2 * Cell::maxEnergy) / 3)
                moveIterator(2);
            else
                moveIterator(3);

            finish = false;
        }
        // узнать сколько клетка получит энергии фотосинтезом () -> (низкий, средний, высокий)
        else if (*currentCommand == 8)
        {
            auto energyOnPoint = World::world.energyOnPoint(cell->getPoint());

            if (energyOnPoint < World::maxEnergyOnPoint / 3)
                moveIterator(1);
            else if (energyOnPoint < (2 * World::maxEnergyOnPoint) / 3)
                moveIterator(2);
            else
                moveIterator(3);

            finish = false;
        }
        // поделиться энергией внутри многоклеточного (сколько дать)
        else if (command == 9)
        {
            moveIterator(1);
            uint8_t energy = ((static_cast<double>(*currentCommand) / maxGenValue) * Cell::maxEnergy) / 8;

            if (cell->share(energy))
                moveIterator(1);

            moveIterator(1);
            finish = false;
        }
        // многоклеточный ли я () -> (нет, да)
        else if (command == 10)
        {
            if (cell->getChainId())
                moveIterator(1);

            moveIterator(1);
            finish = false;
        }
        // окружён ли я () -> (нет, да)
        else if (command == 11)
        {
            for (uint8_t d = 0; d < 8; ++d)
            {
                Direction dir(d);
                if (World::world.getObjectType(Point(cell->getPoint(), dir)) == World::EMPTY)
                {
                    moveIterator(1);
                    break;
                }
            }

            moveIterator(1);
            finish = false;
        }
        // относительный переход
        else if (command == 12)
        {
            moveIterator(1);
            uint16_t offset = *currentCommand;
            offset <<= 8;
            moveIterator(1);
            offset += *currentCommand;
            moveIterator(offset);
            finish = false;
        }
        //переход по абсолютному адресу
        else if (command == 13)
        {
            moveIterator(1);
            uint16_t position = *currentCommand;
            position <<= 8;
            moveIterator(1);
            position += *currentCommand;
            setIterator(position);
            finish = false;
        }
        else
        {
            moveIterator(1);
            finish = false;
        }

        ++i;

    } while (!finish && i < 10);
}

void Genom::mutate()
{
    uint8_t operation = rand() % 100;
    uint16_t position = rand() % genomBytes.size();

    if (operation < 98)
        genomBytes[position] = rand() % maxGenValue;
    else if (operation == 98)
        deleteGen(position);
    else
        insertGen(position, rand() % maxGenValue);
   
    ++mutations;
}

std::string Genom::toString() const
{
    std::stringstream result;
    result << currentPosition() << " : ";
    for (int k = 0; k < genomBytes.size(); ++k)
        result << static_cast<short int>(genomBytes[k]) << " ";
    result << "                    ";

    return result.str();
}