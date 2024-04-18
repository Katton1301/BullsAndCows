#pragma once

#include "core/enums.hpp"
#include "rules/standart_rules.hpp"
#include "brains/standart_brain.hpp"
#include <random>

class TGameProcessBase
{
public:
    TGameProcessBase() = default;
    ~TGameProcessBase() = default;

    MODEL_COMPONENTS::TGameStage GameStage() const;
    void setGameStage( MODEL_COMPONENTS::TGameStage _gameStage );

    virtual void Init();

private:
    MODEL_COMPONENTS::TGameStage m_gameStage = MODEL_COMPONENTS::TGameStage::UNKNOWN;
};

class TStandartGameProcess : public TGameProcessBase
{
public: //types
    using THistoryList = std::vector< std::pair< TGameValue<uint8_t>, std::pair<uint32_t, uint32_t > > >;
    friend class TStandartBrain;
public: //methods
    TStandartGameProcess( );
    ~TStandartGameProcess( ) = default;
    void Init() override;
    void selectBrain( MODEL_COMPONENTS::TGameBrain _gameBrain );
    THistoryList const & HistoryList( ) const;
    uint32_t AttemptsCount( ) const;
    void setTrueGameValue( TGameValue<uint8_t> const & _gameValue );
    void appendGameValue( TGameValue<uint8_t> const & _gameValue );
    void makeStep( );
    std::function< uint32_t( uint32_t ) > const & RandomByModulus() const;
private:
    std::shared_ptr<TStandartBrain> GameBrain_ptr();

private:
    std::mt19937 random_generator_;
    std::function< uint32_t( uint32_t ) > m_randomByModulus;
    THistoryList m_historyList{};
    TGameValue<uint8_t> *m_trueGameValue = nullptr;
    std::shared_ptr<TStandartBrain> m_gameBrain = nullptr;
};
