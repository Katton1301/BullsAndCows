#include <game_controller.h>

TGameController::TGameController()
{
    std::random_device device;
    random_generator_.seed(device());

    m_randomByModulus =
        [this]( uint32_t _modulus )->unsigned int
    {
        std::uniform_int_distribution<uint32_t> range(0, _modulus - 1);
        return range(random_generator_);
    };
}

TGameController::~TGameController()
{

}

uint32_t TGameController::PlayersCount( ) const
{
    return PlayerList().size() + ComputerList().size();
}

MODEL_COMPONENTS::TGameStage TGameController::GameStage() const
{
    return m_gameStage;
}


uint32_t TGameController::GameStep() const
{
    return m_game_step;
}

uint32_t TGameController::GameStep(uint32_t _processId, bool _isPlayer) const
{
    if(_isPlayer)
    {
        if(!m_player_list.contains(_processId)) { return 0; }
        return PlayerCptrById(_processId)->AttemptsCount();
    }
    else
    {
        if(!m_computer_list.contains(_processId)) { return 0; }
        return ComputerCptrById(_processId)->AttemptsCount();
    }
}

void TGameController::InitGame()
{
    m_game_step = 0;
    m_winnersId.clear();
    m_gameStage = MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER;
    for(auto & [gameId, playerProcess] : m_player_list)
    {
        playerProcess->Init();
    }
    for(auto & [gameId, computerProcess] : m_computer_list)
    {
        computerProcess->Init();
    }
}

TGameController::TError TGameController::StartGame( std::vector<uint8_t> const & secretValue )
{
    if(GameStage() != MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER)
    {
        return TError::GAME_BUSY;
    }
    for(auto & [gameId, playerProcess] : m_player_list)
    {
        playerProcess->setTrueGameValue( TGameValue<uint8_t>(secretValue) );
    }
    for(auto & [gameId, computerProcess] : m_computer_list)
    {
        computerProcess->setTrueGameValue( TGameValue<uint8_t>(secretValue) );
    }
    m_gameStage = MODEL_COMPONENTS::TGameStage::IN_PROGRESS;
    return TError::OK;
}

TGameController::TError TGameController::StartGame( )
{
    auto secretValue = TStandartRules::Instance().GetRandomGameValue(RandomByModulus());
    return StartGame(secretValue.List());
}

bool TGameController::allPlayersCompleteCurrentStep()
{
    bool stepFinished = true;
    if(std::any_of(
            m_player_list.begin(),
            m_player_list.end(),
            [this]( auto const & pair )
            {
                return
                    pair.second->PlayerState() != MODEL_COMPONENTS::TPlayerState::FINISHED &&
                    pair.second->PlayerState() != MODEL_COMPONENTS::TPlayerState::GAVE_UP &&
                    pair.second->AttemptsCount() == GameStep()
                ;
            }
            ))
    {
        stepFinished = false;
    }
    return stepFinished;
}

TGameController::TError TGameController::FinishStep()
{
    if(!allPlayersCompleteCurrentStep())
    {
        return TError::NOT_ALL_PLAYERS_STEPED;
    }
    doComputersStep();
    ++m_game_step;
    if(GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS)
    {
        defineWinners();
    }
    checkFinish();
    return TError::OK;
}

void TGameController::checkFinish()
{
    if(
        std::all_of(
            PlayerList().begin(),
            PlayerList().end(),
            [](auto const & pair)
            {
                return
                    pair.second->PlayerState() == MODEL_COMPONENTS::TPlayerState::FINISHED ||
                    pair.second->PlayerState() == MODEL_COMPONENTS::TPlayerState::GAVE_UP;
            }
        )
        &&
        std::all_of(
            ComputerList().begin(),
            ComputerList().end(),
            [](auto const & pair)
            {
                return
                    pair.second->PlayerState() == MODEL_COMPONENTS::TPlayerState::FINISHED ||
                    pair.second->PlayerState() == MODEL_COMPONENTS::TPlayerState::GAVE_UP;
            }
        )
    )
    {
        m_gameStage = MODEL_COMPONENTS::TGameStage::FINISHED;
    }
}

void TGameController::FinishGame()
{
    for(auto const & [gameId, playerProcess] : PlayerList())
    {
        if(
            playerProcess->PlayerState() != MODEL_COMPONENTS::TPlayerState::FINISHED &&
            playerProcess->PlayerState() != MODEL_COMPONENTS::TPlayerState::GAVE_UP
        )
        {
            playerProcess->giveUp();
        }
    }
    while( GameStage() != MODEL_COMPONENTS::TGameStage::FINISHED )
    {
        FinishStep();
    }
}

TGameController::TError TGameController::DoPlayerStep( uint32_t _processId, std::vector< uint8_t > const & _gameValueList )
{
    if(!m_player_list.contains(_processId))
    {
        return TError::PLAYER_NOT_FOUND;
    }
    auto & playerProcess = PlayerPtrById(_processId);
    if(playerProcess->AttemptsCount() > GameStep())
    {
        return TError::PLAYER_ALREADY_STEP;
    }
    if(playerProcess->PlayerState() != MODEL_COMPONENTS::TPlayerState::IN_PROGRESS)
    {
        return TError::PLAYER_ALREADY_FINISH;
    }
    playerProcess->appendGameValue(TGameValue(_gameValueList));

    if(allPlayersCompleteCurrentStep())
    {
        return FinishStep();
    }
    return TError::OK;
}

TGameController::TError TGameController::PlayerGiveUp( uint32_t _processId )
{
    if(!m_player_list.contains(_processId))
    {
        return TError::PLAYER_NOT_FOUND;
    }
    auto & playerProcess = PlayerPtrById(_processId);
    if(playerProcess->PlayerState() != MODEL_COMPONENTS::TPlayerState::IN_PROGRESS)
    {
        return TError::PLAYER_ALREADY_FINISH;
    }
    auto stepInProgress = isStepInTransitionStage();
    playerProcess->giveUp();
    if(stepInProgress)
    {
        if(allPlayersCompleteCurrentStep())
        {
            return FinishStep();
        }
    }
    else
    {
        checkFinish();
    }
    return TError::OK;
}

std::vector<std::pair<uint32_t, bool>> const & TGameController::WinnersId( ) const
{
    return m_winnersId;
}

void TGameController::defineWinners()
{
    std::vector<std::pair<uint32_t,bool>> winIdList;
    for(auto const & [gameId, playerProcess] : PlayerList())
    {
        if(
            playerProcess->PlayerState() == MODEL_COMPONENTS::TPlayerState::FINISHED &&
            playerProcess->AttemptsCount() == GameStep()
        )
        {
            winIdList.push_back(std::make_pair(gameId, true));
        }
    }
    for(auto const & [gameId, computerProcess] : ComputerList())
    {
        if(
            computerProcess->PlayerState() == MODEL_COMPONENTS::TPlayerState::FINISHED &&
            computerProcess->AttemptsCount() == GameStep()
            )
        {
            winIdList.push_back(std::make_pair(gameId, false));
        }
    }
    if(winIdList.size() > 0)
    {
        m_gameStage = MODEL_COMPONENTS::TGameStage::IN_PROGRESS_WINNER_DEFINED;
        m_winnersId = winIdList;
    }

}

uint32_t TGameController::PlayerPlace( uint32_t _processId, bool _isPlayer ) const
{
    if(
        (_isPlayer && !m_player_list.contains(_processId)) ||
        (!_isPlayer && !m_computer_list.contains(_processId))
    )
    {
        return 0;
    }
    auto const & process = _isPlayer ? PlayerCptrById(_processId) : ComputerCptrById(_processId);
    if(
        process->PlayerState() != MODEL_COMPONENTS::TPlayerState::FINISHED &&
        process->PlayerState() != MODEL_COMPONENTS::TPlayerState::GAVE_UP
    )
    {
        return 0;
    }
    auto step = process->AttemptsCount();
    if(process->PlayerState() == MODEL_COMPONENTS::TPlayerState::GAVE_UP)
    {
        uint32_t place = PlayersCount();
        for(auto const & [gameId, playerProcess] : PlayerList())
        {
            if(
                (gameId != _processId || !_isPlayer) &&
                playerProcess->PlayerState() == MODEL_COMPONENTS::TPlayerState::GAVE_UP &&
                playerProcess->AttemptsCount() >= step
            )
            {
                --place;
            }
        }
        for(auto const & [gameId, computerProcess] : ComputerList())
        {
            if(
                (gameId != _processId || _isPlayer) &&
                computerProcess->PlayerState() == MODEL_COMPONENTS::TPlayerState::GAVE_UP &&
                computerProcess->AttemptsCount() >= step
                )
            {
                --place;
            }
        }
        return place;
    }
    else
    {
        uint32_t place = 1;
        for(auto const & [gameId, playerProcess] : PlayerList())
        {
            if(
                (gameId != _processId || !_isPlayer) &&
                playerProcess->PlayerState() == MODEL_COMPONENTS::TPlayerState::FINISHED &&
                playerProcess->AttemptsCount() < step
            )
            {
                ++place;
            }
        }
        for(auto const & [gameId, computerProcess] : ComputerList())
        {
            if(
                (gameId != _processId || _isPlayer) &&
                computerProcess->PlayerState() == MODEL_COMPONENTS::TPlayerState::FINISHED &&
                computerProcess->AttemptsCount() < step
            )
            {
                ++place;
            }
        }
        return place;
    }
}

TGameController::StepResults TGameController::getProcessStepResults( uint32_t _processId, bool _isPlayer, uint32_t _gameStep ) const
{
    TGameController::StepResults result;
    result.player = _isPlayer;
    if(
        _gameStep > 0 && (
            (_isPlayer && PlayerList().contains(_processId)) ||
            (!_isPlayer && ComputerList().contains(_processId))
        )
    )
    {
        result.processId = _processId;
        auto const & process = _isPlayer ? PlayerCptrById(_processId) : ComputerCptrById(_processId);
        if(_gameStep - 1 < process->HistoryList().size())
        {
            auto const & lastHistoryData = process->HistoryList().at(_gameStep - 1);
            result.gameValueList = lastHistoryData.first.List();
            result.bulls = lastHistoryData.second.first;
            result.cows = lastHistoryData.second.second;
            result.step = _gameStep;
            result.finished = process->AttemptsCount() == _gameStep;
        }
    }
    return result;
}

std::vector<TGameController::StepResults> TGameController::getStepResults( uint32_t _gameStep ) const
{
    std::vector<StepResults> results;
    if(_gameStep == 0)
    {
        return results;
    }
    for(auto const & [gameId, playerProcess] : PlayerList())
    {
        if(_gameStep - 1 < playerProcess->HistoryList().size())
        {
            auto const & lastHistoryData = playerProcess->HistoryList().at(_gameStep - 1);
            results.push_back(
                {
                    gameId,
                    true,
                    playerProcess->AttemptsCount(),
                    lastHistoryData.first.List(),
                    lastHistoryData.second.first,
                    lastHistoryData.second.second,
                    playerProcess->PlayerState() == MODEL_COMPONENTS::TPlayerState::FINISHED
                }
            );
        }
    }
    for(auto const & [gameId, computerProcess] : ComputerList())
    {
        if(_gameStep - 1 < computerProcess->HistoryList().size())
        {
            auto const & lastHistoryData = computerProcess->HistoryList().at(_gameStep - 1);
            results.push_back(
                {
                    gameId,
                    false,
                    computerProcess->AttemptsCount(),
                    lastHistoryData.first.List(),
                    lastHistoryData.second.first,
                    lastHistoryData.second.second,
                    computerProcess->PlayerState() == MODEL_COMPONENTS::TPlayerState::FINISHED
                }
                );
        }
    }
    return results;
}

void TGameController::doComputersStep()
{
    for(auto & [gameId, computerProcess] : m_computer_list)
    {
        if(
            computerProcess->PlayerState() != MODEL_COMPONENTS::TPlayerState::FINISHED &&
            computerProcess->PlayerState() != MODEL_COMPONENTS::TPlayerState::GAVE_UP &&
            computerProcess->AttemptsCount() == GameStep()
        )
        {
            computerProcess->makeStep();
        }
    }
}

std::function< uint32_t( uint32_t ) > const & TGameController::RandomByModulus() const
{
    return m_randomByModulus;
}

TGameController::TError TGameController::addPlayerProcess(uint32_t _processId)
{
    if(GameStage() != MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER)
    {
        return TError::GAME_IN_PROGRESS;
    }
    if(m_player_list.contains(_processId))
    {
        return TError::PLAYER_ALREADY_EXISTS;
    }
    if(PlayerList().size() >= GAME_PROCESS_COUNT_LIMIT)
    {
        return TError::PROCESS_LIMIT_HAS_BEEN_REACHED;
    }
    auto playerProcess = std::make_unique<TStandartPlayerProcess>(m_randomByModulus);
    playerProcess->Init();
    m_player_list.emplace(_processId, std::move(playerProcess));
    return TError::OK;
}

TGameController::TError TGameController::addComputerProcess(uint32_t _processId, MODEL_COMPONENTS::TGameBrain _gameBrain)
{
    if(GameStage() != MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER)
    {
        return TError::GAME_IN_PROGRESS;
    }
    if(m_computer_list.contains(_processId))
    {
        return TError::COMPUTER_ALREADY_EXISTS;
    }
    if(
        static_cast<uint32_t>(_gameBrain) < static_cast<uint32_t>(MODEL_COMPONENTS::TGameBrain::BEGIN) ||
        static_cast<uint32_t>(_gameBrain) >= static_cast<uint32_t>(MODEL_COMPONENTS::TGameBrain::END)
    )
    {
        return TError::UNKNOWN_BRAIN;
    }
    if(ComputerList().size() >= GAME_PROCESS_COUNT_LIMIT)
    {
        return TError::PROCESS_LIMIT_HAS_BEEN_REACHED;
    }
    auto computerProcess = std::make_unique<TStandartPlayerProcess>(m_randomByModulus);
    computerProcess->selectBrain(_gameBrain);
    computerProcess->Init();
    m_computer_list.emplace(_processId, std::move(computerProcess));
    return TError::OK;
}


TGameController::TError TGameController::removePlayerProcess( uint32_t _processId, bool _isPlayer )
{
    if(
        GameStage() != MODEL_COMPONENTS::TGameStage::FINISHED &&
        GameStage() != MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER
    )
    {
        return TError::GAME_IN_PROGRESS;
    }
    if(_isPlayer)
    {
        if(!m_player_list.contains(_processId))
        {
            return TError::PLAYER_NOT_FOUND;
        }
        m_player_list.erase(_processId);
    }
    else
    {
        if(!m_computer_list.contains(_processId))
        {
            return TError::COMPUTER_NOT_FOUND;
        }
        m_computer_list.erase(_processId);
    }
    return TError::OK;
}

TGameController::TError TGameController::switchGameBrain(uint32_t _processId, MODEL_COMPONENTS::TGameBrain _gameBrain)
{
    if(!m_computer_list.contains(_processId))
    {
        return TError::COMPUTER_NOT_FOUND;
    }
    if(
        static_cast<uint32_t>(_gameBrain) < static_cast<uint32_t>(MODEL_COMPONENTS::TGameBrain::BEGIN) ||
        static_cast<uint32_t>(_gameBrain) >= static_cast<uint32_t>(MODEL_COMPONENTS::TGameBrain::END)
        )
    {
        return TError::UNKNOWN_BRAIN;
    }
    ComputerPtrById(_processId)->Init();
    ComputerPtrById(_processId)->selectBrain(_gameBrain);
    return TError::OK;
}

bool TGameController::isStepInTransitionStage( ) const
{
    bool existsPlayerFinishStep = false;
    bool existsPlayerUnfinishStep = false;
    for(auto & [gameId, playerProcess] : m_player_list)
    {
        if(
            playerProcess->PlayerState() == MODEL_COMPONENTS::TPlayerState::IN_PROGRESS &&
            playerProcess->AttemptsCount() == GameStep()
        )
        {
            existsPlayerUnfinishStep = true;
        }
        if(
            playerProcess->AttemptsCount() > GameStep() ||
            (
                playerProcess->PlayerState() == MODEL_COMPONENTS::TPlayerState::GAVE_UP &&
                playerProcess->AttemptsCount() == GameStep()
            )
        )
        {
            existsPlayerFinishStep = true;
        }
    }
    return existsPlayerFinishStep && existsPlayerUnfinishStep;
}
