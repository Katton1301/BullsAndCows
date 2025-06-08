#pragma once
#include <core/enums.hpp>
#include <player_process.hpp>
#include <random>

class TGameController
{
public:
    enum
    {
        GAME_PROCESS_COUNT_LIMIT = 10
    };
    enum class TError : uint32_t
    {
        UNKNOWN = 0,
        OK,
        GAME_BUSY,
        PLAYER_ALREADY_STEP,
        PLAYER_ALREADY_FINISH,
        NOT_ALL_PLAYERS_STEPED,
        GAME_IN_PROGRESS,
        PLAYER_ALREADY_EXISTS,
        COMPUTER_ALREADY_EXISTS,
        PLAYER_NOT_FOUND,
        COMPUTER_NOT_FOUND,
        PROCESS_LIMIT_HAS_BEEN_REACHED,
        UNKNOWN_BRAIN,
    };

    TGameController();
    ~TGameController();

    TError restoreGame(
            std::vector<uint8_t> const & secretValue,
            std::unordered_map<
                uint32_t,
                std::tuple<bool, bool, MODEL_COMPONENTS::TGameBrain, TStandartPlayerProcess::THistoryList>
            >const & processData
    );
    void InitGame();
    TError StartGame(std::vector<uint8_t> const & secretValue);
    TError StartGame();
    TError DoPlayerStep( uint32_t _processId, std::vector< uint8_t > const & _gameValueList );
    TError PlayerGiveUp( uint32_t _processId );
    TError FinishStep();
    void FinishGame();
    TError addPlayerProcess(uint32_t _processId);
    TError addComputerProcess(uint32_t _processId, MODEL_COMPONENTS::TGameBrain _gameBrain);
    TError removePlayerProcess( uint32_t _processId, bool _isPlayer );
    TError switchGameBrain( uint32_t _processId, MODEL_COMPONENTS::TGameBrain _gameBrain );
    bool isStepInTransitionStage( ) const;
    uint32_t PlayerPlace( uint32_t _processId, bool _isPlayer ) const;
    std::vector<uint8_t> SecretValue() const;

    std::vector<std::pair<uint32_t, bool>> const & WinnersId( ) const;

    MODEL_COMPONENTS::StepResults getProcessStepResults( uint32_t _processId, bool _isPlayer, uint32_t _gameStep ) const;

    std::vector<MODEL_COMPONENTS::StepResults> getStepResults( uint32_t _gameStep ) const;
    std::vector<MODEL_COMPONENTS::GameResults> getGameResults( ) const;

    uint32_t PlayersCount( ) const;
    uint32_t UnsteppedPlayers( uint32_t _gameStep ) const;
    uint32_t UnsteppedPlayers( ) const;
    MODEL_COMPONENTS::TGameStage GameStage() const;
    uint32_t GameStep() const;
    uint32_t GameStep(uint32_t _processId, bool _isPlayer) const;

private:
    std::unique_ptr<TStandartPlayerProcess> & PlayerPtrById( uint32_t _processID )
    {
        return m_player_list.at(_processID);
    }
    std::unique_ptr<TStandartPlayerProcess> const & PlayerCptrById( uint32_t _processID ) const
    {
        return m_player_list.at(_processID);
    }

    std::unordered_map< uint32_t, std::unique_ptr<TStandartPlayerProcess > > const & PlayerList( ) const
    {
        return m_player_list;
    }

    std::unique_ptr<TStandartPlayerProcess> & ComputerPtrById( uint32_t _processID )
    {
        return m_computer_list.at(_processID);
    }
    std::unique_ptr<TStandartPlayerProcess> const & ComputerCptrById( uint32_t _processID ) const
    {
        return m_computer_list.at(_processID);
    }

    std::unordered_map< uint32_t, std::unique_ptr<TStandartPlayerProcess > > const & ComputerList( ) const
    {
        return m_computer_list;
    }

    std::function< uint32_t( uint32_t ) > const & RandomByModulus() const;

    void doComputersStep();
    void defineWinners();
    void checkFinish();

private:
    std::mt19937 random_generator_;
    std::function< uint32_t( uint32_t ) > m_randomByModulus;
    std::unordered_map< uint32_t, std::unique_ptr<TStandartPlayerProcess> > m_player_list{};
    std::unordered_map< uint32_t, std::unique_ptr<TStandartPlayerProcess> > m_computer_list{};
    MODEL_COMPONENTS::TGameStage m_gameStage = MODEL_COMPONENTS::TGameStage::UNKNOWN;
    std::vector<std::pair<uint32_t, bool>> m_winnersId{};
    uint32_t m_game_step = 0;
    std::vector<uint8_t> m_secretValue;
};
