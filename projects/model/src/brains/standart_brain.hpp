#pragma once
#include <brains/brain_interface.hpp>
#include <core/game_value.hpp>
#include <core/enums.hpp>
#include <memory>
#include <map>
class TStandartPlayerProcess;
class TValueNode;

struct TStandartBrain : public IGameBrain
{
    TStandartBrain() = delete;
    TStandartBrain( TStandartPlayerProcess const * _playerProcess );
    ~TStandartBrain() = default;

    std::shared_ptr<TGameValue<uint8_t>> const & PredictedValue() const;

protected:
    TStandartPlayerProcess const *  m_playerProcess_cptr = nullptr;
    std::shared_ptr<TGameValue<uint8_t>> m_predictedValue{};
};

class TDecisionTreeBrain : public TStandartBrain
{
public:
    static std::string DecisionTreePath;

    TDecisionTreeBrain() = delete;
    TDecisionTreeBrain( TStandartPlayerProcess const * _playerProcess );
    ~TDecisionTreeBrain() = default;

    virtual void Init() override;
    virtual void makePredict( ) override;

protected:
    void flipValueCoins(std::vector<uint8_t> & value, bool isFrontSide);

protected:
    std::vector<std::pair<uint8_t, uint8_t>> m_digitCoins{};
    std::shared_ptr<TValueNode >m_gameNode = nullptr;
};

class TAnaliticBrain : public TStandartBrain
{
public:
    TAnaliticBrain() = delete;
    TAnaliticBrain( TStandartPlayerProcess const * _playerProcess );
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
    TStandartRandomBrain( TStandartPlayerProcess const * _playerProcess );
    ~TStandartRandomBrain() = default;

    virtual void makePredict( ) override;
};

class TStandartStupidBrain : public TAnaliticBrain
{
public:
    TStandartStupidBrain() = delete;
    TStandartStupidBrain( TStandartPlayerProcess const * _playerProcess );
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
    TStandartSmartBrain( TStandartPlayerProcess const * _playerProcess );
    ~TStandartSmartBrain() = default;

    virtual void Init( ) override;
    virtual void makePredict( ) override;
protected:
    virtual void handleValuesByHistory() override;
    virtual int32_t chooseBestGameValueOffset() override;
};

std::shared_ptr<TStandartBrain> createStandartBrain( TStandartPlayerProcess const * _playerProcess, MODEL_COMPONENTS::TGameBrain _gameBrain );
