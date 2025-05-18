#pragma once

#include <core/enums.hpp>
#include <rules/standart_rules.hpp>
#include <brains/standart_brain.hpp>

class TPlayerProcessBase
{
public:
    TPlayerProcessBase() = default;
    ~TPlayerProcessBase() = default;

    MODEL_COMPONENTS::TPlayerState PlayerState() const;
    void setPlayerState( MODEL_COMPONENTS::TPlayerState _playerState );

    virtual bool isBrainless( ) const;

    virtual void Init();

private:
    MODEL_COMPONENTS::TPlayerState m_playerState = MODEL_COMPONENTS::TPlayerState::UNKNOWN;
};

class TStandartPlayerProcess : public TPlayerProcessBase
{
public: //types
    using THistoryList = std::vector< std::pair< TGameValue<uint8_t>, std::pair<uint32_t, uint32_t > > >;
    friend struct TStandartBrain;
public: //methods
    TStandartPlayerProcess( ) = delete;
    TStandartPlayerProcess( std::function< uint32_t( uint32_t ) > & _randomFunc );
    ~TStandartPlayerProcess( ) = default;
    void Init() override;
    void selectBrain( MODEL_COMPONENTS::TGameBrain _gameBrain );
    bool isBrainless( ) const override
    {
        return m_gameBrain == nullptr;
    }
    THistoryList const & HistoryList( ) const;
    uint32_t AttemptsCount( ) const;
    void setTrueGameValue( TGameValue<uint8_t> const & _gameValue );
    void appendGameValue( TGameValue<uint8_t> const & _gameValue );
    void makeStep( );
    void giveUp( );
    std::function< uint32_t( uint32_t ) > const & GetRandom() const;
    void restoreProcess(
            std::vector<uint8_t> const & secretValue,
            THistoryList const & history
    );
private:
    std::shared_ptr<TStandartBrain> GameBrain_ptr();

private:
    std::function< uint32_t( uint32_t ) > m_randomFunc;
    THistoryList m_historyList{};
    std::shared_ptr<TGameValue<uint8_t>>m_trueGameValue = nullptr;
    std::shared_ptr<TStandartBrain> m_gameBrain = nullptr;

};
