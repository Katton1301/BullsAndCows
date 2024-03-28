#pragma once

#include "enums.hpp"
#include "game_rules.hpp"
#include "game_brain.hpp"
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
    std::function< uint32_t( uint32_t ) > const & RandomByModulus();
private:
    std::shared_ptr<IGameBrain> GameBrain_ptr();

private:
    std::mt19937 random_generator_;
    std::function< uint32_t( uint32_t ) > m_randomByModulus;
    THistoryList m_historyList{};
    TGameValue<uint8_t> *m_trueGameValue = nullptr;
    std::shared_ptr<IGameBrain> m_gameBrain = nullptr;
};
