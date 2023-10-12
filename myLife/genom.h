#pragma once

#include <vector>
#include <string>

class Cell;

class Genom
{
    Cell* cell;

    std::vector<uint8_t> genomBytes;
    std::vector<uint8_t>::const_iterator currentCommand;

    inline void moveIterator(uint16_t pos);
    inline void setIterator(uint16_t pos);

    void insertGen(uint16_t pos, uint8_t gen);
    void deleteGen(uint16_t pos);
    //std::list<uint16_t> toBytes(const std::vector<std::unique_ptr<CellAction>>& genom);

    uint64_t mutations;

    inline uint16_t currentPosition() const { return currentCommand - genomBytes.begin(); };

public:
    const static uint8_t maxGenValue = 64;
    
    Genom(const std::vector<uint8_t>& genom) : cell(cell), genomBytes(genom), currentCommand(genomBytes.begin()), mutations(0) {};
    Genom(const Genom& genom) :
        cell(cell),
        genomBytes(genom.genomBytes), 
        currentCommand(genomBytes.begin()), 
        mutations(genom.mutations) {};
    Genom& operator= (const Genom &genom)
    {
        genomBytes = genom.genomBytes;
        currentCommand = genomBytes.begin() + (genom.currentCommand - genom.genomBytes.begin());
        mutations = genom.mutations;
        return *this;
    }

    void mutate();

    std::string toString() const;
    
    void execute();

    friend class Cell;
};

