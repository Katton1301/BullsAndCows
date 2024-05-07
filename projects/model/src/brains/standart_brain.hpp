#pragma once
#include "brains/brain_interface.hpp"
#include "core/game_value.hpp"
#include "core/enums.hpp"
#include <memory>
#include <map>
class TStandartGameProcess;
class TValueNode;

struct TStandartBrain : public IGameBrain
{
    TStandartBrain() = delete;
    TStandartBrain( TStandartGameProcess const * _gameProcess );
    ~TStandartBrain() = default;

    TGameValue<uint8_t> const * PredictedValue_cptr() const;

protected:
    TStandartGameProcess const *  m_gameProcess_cptr = nullptr;
    TGameValue<uint8_t> * m_predictedValue = nullptr;
};

class TStorageTreeBrain : public TStandartBrain
{
public:
    TStorageTreeBrain() = delete;
    TStorageTreeBrain( TStandartGameProcess const * _gameProcess );
    ~TStorageTreeBrain() = default;

    virtual void Init() override;
    virtual void makePredict( ) override;

protected:
    void flipValueCoins(std::vector<uint8_t> & value, bool isFrontSide);

protected:
    std::vector<std::pair<uint8_t, uint8_t>> m_digitCoins{};
    TValueNode *m_gameNode = nullptr;
};

class TAnaliticBrain : public TStandartBrain
{
public:
    TAnaliticBrain() = delete;
    TAnaliticBrain( TStandartGameProcess const * _gameProcess );
    ~TAnaliticBrain() = default;

    virtual void Init() override;

protected:
    void copyPossibleValuesList();

protected:
    std::vector< TGameValue<uint8_t> > m_possibleValues{};
};

class TStandartRandomBrain : public TAnaliticBrain
{
public:
    TStandartRandomBrain() = delete;
    TStandartRandomBrain( TStandartGameProcess const * _gameProcess );
    ~TStandartRandomBrain() = default;

    virtual void makePredict( ) override;
};

class TStandartStupidBrain : public TAnaliticBrain
{
public:
    TStandartStupidBrain() = delete;
    TStandartStupidBrain( TStandartGameProcess const * _gameProcess );
    ~TStandartStupidBrain() = default;

    virtual void Init( ) override;
    virtual void makePredict( ) override;
protected:
    virtual void handleValuesByHistory();
    virtual int32_t chooseBestGameValueOffset();
    void eraseValuesForDigits( std::vector<uint8_t> const & digits );
    void leaveValuesForDigits( std::vector<uint8_t> const & digits );
    int32_t chooseFirstAndSecondGameValueIndex();
private:
    void calcPriority();

private:
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
    virtual void handleValuesByHistory() override;
    virtual int32_t chooseBestGameValueOffset() override;
};

std::shared_ptr<TStandartBrain> createStandartBrain( TStandartGameProcess const * _gameProcess, MODEL_COMPONENTS::TGameBrain _gameBrain );
