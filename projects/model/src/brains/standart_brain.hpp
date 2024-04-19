#pragma once
#include "brains/brain_interface.hpp"
#include "core/game_value.hpp"
#include "core/enums.hpp"
#include <memory>
#include <map>
class TStandartGameProcess;

struct TStandartBrain : public IGameBrain
{
    TStandartBrain() = delete;
    TStandartBrain( TStandartGameProcess const * _gameProcess );
    ~TStandartBrain() = default;

    virtual void Init() override;
    TGameValue<uint8_t> const * PredictedValue_cptr() const;


protected:
    void copyPossibleValuesList();

protected:
    TStandartGameProcess const *  m_gameProcess_cptr = nullptr;
    std::vector< TGameValue<uint8_t> > m_possibleValues{};
    TGameValue<uint8_t> * m_predictedValue = nullptr;
};

class TStandartRandomBrain : public TStandartBrain
{
public:
    TStandartRandomBrain() = delete;
    TStandartRandomBrain( TStandartGameProcess const * _gameProcess );
    ~TStandartRandomBrain() = default;

    virtual void makePredict( ) override;
};

class TStandartStupidBrain : public TStandartBrain
{
public:
    TStandartStupidBrain() = delete;
    TStandartStupidBrain( TStandartGameProcess const * _gameProcess );
    ~TStandartStupidBrain() = default;

    virtual void Init( ) override;
    virtual void makePredict( ) override;
protected:
    virtual void handlePredictedValueByIndex( uint32_t history_i );
    void eraseValuesForDigits( std::vector<uint8_t> const & digits );
    void leaveValuesForDigits( std::vector<uint8_t> const & digits );
    int32_t chooseFirstAndSecondGameValueIndex();
    virtual int32_t chooseBestGameValueOffset();
    void calcPriority();

protected:
    std::map< uint8_t, double > m_digitsPriority;

};

class TStandartSmartBrain : public TStandartStupidBrain
{
public:
    TStandartSmartBrain() = delete;
    TStandartSmartBrain( TStandartGameProcess const * _gameProcess );
    ~TStandartSmartBrain() = default;

    virtual void Init( ) override;
    virtual void makePredict( ) override;
protected:
    virtual int32_t chooseBestGameValueOffset() override;
    void checkValuesByHistoryList();
};

std::shared_ptr<TStandartBrain> createStandartBrain( TStandartGameProcess const * _gameProcess, MODEL_COMPONENTS::TGameBrain _gameBrain );
