#include <events/event_processor.hpp>
#include <data_storage.hpp>
#include <components/commands.hpp>

SERVER_COMPONENTS::TResult ConvertGameErrorToServerResult( TGameController::TError error )
{
    SERVER_COMPONENTS::TResult result = SERVER_COMPONENTS::TResult::UNKNOWN;
    switch (error)
    {
    case TGameController::TError::GAME_BUSY:
        result = SERVER_COMPONENTS::TResult::ERROR_GAME_IN_PROGRESS;
        break;
    case TGameController::TError::PLAYER_ALREADY_STEP:
        result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_ALREADY_MADE_STEP;
        break;
    case TGameController::TError::PLAYER_ALREADY_FINISH:
        result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_ALREADY_FINISHED;
        break;
    case TGameController::TError::NOT_ALL_PLAYERS_STEPED:
        result = SERVER_COMPONENTS::TResult::ERROR_NOT_ALL_PLAYERS_STEPED;
        break;
    case TGameController::TError::GAME_IN_PROGRESS:
        result = SERVER_COMPONENTS::TResult::ERROR_GAME_IN_PROGRESS;
        break;
    case TGameController::TError::PLAYER_ALREADY_EXISTS:
        result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_ALREADY_EXISTS;
        break;
    case TGameController::TError::COMPUTER_ALREADY_EXISTS:
        result = SERVER_COMPONENTS::TResult::ERROR_COMPUTER_ALREADY_EXISTS;
        break;
    case TGameController::TError::PLAYER_NOT_FOUND:
        result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
        break;
    case TGameController::TError::COMPUTER_NOT_FOUND:
        result = SERVER_COMPONENTS::TResult::ERROR_COMPUTER_NOT_FOUND;
        break;
    case TGameController::TError::PROCESS_LIMIT_HAS_BEEN_REACHED:
        result = SERVER_COMPONENTS::TResult::ERROR_PROCESS_LIMIT_REACHED;
        break;
    case TGameController::TError::UNKNOWN_BRAIN:
        result = SERVER_COMPONENTS::TResult::ERROR_UNKNOWN_BRAIN;
        break;
    default:
        break;
    }
    return result;
}


TEventProcessor::TEventProcessor(
        TEventManager& manager
#if defined(KAFKA_SERVER)
        , RdKafka::Producer* kafka_producer
        , std::string const & producer_topic
#endif
    )
    : m_manager(manager)
#if defined(KAFKA_SERVER)
    , m_kafka_producer(kafka_producer)
    , m_producer_topic(producer_topic)
#endif
    , m_workers()
    , m_serverState(SERVER_COMPONENTS::TServerState::WAIT_REGISTRATION)
    , m_serverId(0)
    , m_running(false)
{}

void TEventProcessor::start(int thread_count)
{
    m_running = true;
    for (int i = 0; i < thread_count; ++i)
    {
        m_workers.emplace_back([this]() { processEvents(); });
    }
}

void TEventProcessor::stop()
{
    m_running = false;
    m_manager.stop();
    for (auto& worker : m_workers)
    {
        if (worker.joinable()) worker.join();
    }
}

void TEventProcessor::processEvents()
{
    while (m_running)
    {
        auto event = m_manager.getNextEvent();
        if (!event) continue;
        std::cout << "Process: " << event->getData() << std::endl;
        auto requestData = SERVER_COMPONENTS::ParseKafkaMessage(event->getData());

        SERVER_COMPONENTS::TResultData result;
        result.CorrelationId = requestData.CorrelationId;
        if(requestData.CorrelationId == "" || requestData.Command == SERVER_COMPONENTS::TCommand::UNKNOWN)
        {
            result.Result = SERVER_COMPONENTS::TResult::ERROR_PARSE_MESSAGE;
        }
        else
        {
            switch(ServerState())
            {
            case SERVER_COMPONENTS::TServerState::WAIT_REGISTRATION:
                result = registerServer(requestData);
                break;
            case SERVER_COMPONENTS::TServerState::READY_TO_WORK:
                result = handleRequest(requestData);
                break;
            default:
                break;
            }
        }

        if(result.Result != SERVER_COMPONENTS::TResult::UNKNOWN)
        {
            auto resultMessage = SERVER_COMPONENTS::SerializeResultToKafkaMessage(result);
#if defined(KAFKA_SERVER)
            sendToKafka(resultMessage);
#else
            std::cout << "Sent to Kafka: " << resultMessage << std::endl;
#endif
            m_manager.setEventResponse(event->getId(), resultMessage);
            //m_manager.removeEvent(event->getId());
        }
    }
}


SERVER_COMPONENTS::TServerState TEventProcessor::ServerState( ) const
{
    return m_serverState;
}

uint32_t TEventProcessor::ServerId() const
{
    return m_serverId;
}

SERVER_COMPONENTS::TResultData TEventProcessor::registerServer( SERVER_COMPONENTS::TRequestData const & request )
{
    SERVER_COMPONENTS::TResultData result;
    result.CorrelationId = request.CorrelationId;
    if(request.Command != SERVER_COMPONENTS::TCommand::REGISTER_SERVER)
    {
        result.Result = SERVER_COMPONENTS::TResult::ERROR_SERVER_NOT_REGISTRED;
        return result;
    }
    if(request.ServerId == 0)
    {
        result.Result = SERVER_COMPONENTS::TResult::ERROR_BAD_SERVER_ID;
        return result;
    }
    m_serverId = request.ServerId;
    m_serverState = SERVER_COMPONENTS::TServerState::READY_TO_WORK;
    result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
    result.ServerId = ServerId();
    return result;
}

SERVER_COMPONENTS::TResultData TEventProcessor::handleRequest( SERVER_COMPONENTS::TRequestData const & request )
{
    if(request.ServerId != ServerId())
    {
        SERVER_COMPONENTS::TResultData result;
        result.CorrelationId = request.CorrelationId;
        result.ServerId = ServerId();
        result.Result = SERVER_COMPONENTS::TResult::ERROR_MESSAGE_FOR_ANOTHER_SERVER;
        return result;
    }
    if(
        static_cast<uint32_t>(request.Command) >= static_cast<uint32_t>(SERVER_COMPONENTS::TCommand::SERVER_COMMAND_BEGIN) &&
        static_cast<uint32_t>(request.Command) < static_cast<uint32_t>(SERVER_COMPONENTS::TCommand::SERVER_COMMAND_END)
    )
    {
        return handleServerCommand(request);
    }
    else
    {
        return handleGameCommand(request);
    }

}

SERVER_COMPONENTS::TResultData TEventProcessor::handleServerCommand( SERVER_COMPONENTS::TRequestData const & request )
{
    SERVER_COMPONENTS::TResultData result;
    result.CorrelationId = request.CorrelationId;
    result.ServerId = ServerId();
    switch(request.Command)
    {
    case SERVER_COMPONENTS::TCommand::SERVER_INFO:
        result.GameIds = TDataStorage::Instance().getAllGameIds();
        result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        break;
    case SERVER_COMPONENTS::TCommand::REGISTER_SERVER:
        result.Result = SERVER_COMPONENTS::TResult::ERROR_SERVER_ALREADY_REGISTRED;
        break;
    case SERVER_COMPONENTS::TCommand::DISCONNECT_SERVER:
        TDataStorage::Instance().clearStorage();
        m_serverId = 0;
        m_serverState = SERVER_COMPONENTS::TServerState::WAIT_REGISTRATION;
        result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        break;
    default:
        result.Result = SERVER_COMPONENTS::TResult::ERROR_UNKNOWN_COMMAND;
        break;
    }
    return result;
}

SERVER_COMPONENTS::TResultData TEventProcessor::handleGameCommand( SERVER_COMPONENTS::TRequestData const & request )
{
    SERVER_COMPONENTS::TResultData result;
    result.CorrelationId = request.CorrelationId;
    result.ServerId = ServerId();
    auto success = TDataStorage::Instance().lockForProcessing(request.GameId, request.PlayerId);
    if (!success)
    {
        std::cout << "Game " << request.GameId << " or Player " << request.PlayerId << " is busy now" << std::endl;
        return result;
    }
    try
    {
        result.GameId = request.GameId;
        result.PlayerId = request.PlayerId;
        switch (request.Command)
        {
        case SERVER_COMPONENTS::TCommand::CREATE_GAME:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                TDataStorage::Instance().createPlayer(request.PlayerId);
            }
            auto & player = TDataStorage::Instance().getPlayer(request.PlayerId);
            if(player->GamesCount() >= SERVER_COMPONENTS::PLAYER_GAMES_LIMIT)
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAMES_LIMIT_REACHED;
                break;
            }
            if(TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_ALREADY_EXISTS;
                break;
            }
            TDataStorage::Instance().createGame(request.GameId);
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            if(request.ComputerId == 0)
            {
                auto error = game->addPlayerProcess(request.PlayerId);
                if(error != TGameController::TError::OK)
                {
                    TDataStorage::Instance().removeGame(request.GameId);
                    result.Result = ConvertGameErrorToServerResult(error);
                    break;
                }
                game->InitGame();
                player->addGame(request.GameId, true);
            }
            else
            {
                auto error = game->addComputerProcess(request.ComputerId, request.GameBrain);
                if(error != TGameController::TError::OK)
                {
                    TDataStorage::Instance().removeGame(request.GameId);
                    result.Result = ConvertGameErrorToServerResult(error);
                    break;
                }
                game->InitGame();
                player->addGame(request.GameId, true);
                player->addComputer(request.GameId, request.ComputerId);
            }
            result.GameStage = game->GameStage();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::ADD_PLAYER:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                TDataStorage::Instance().createPlayer(request.PlayerId);
            }
            auto & player = TDataStorage::Instance().getPlayer(request.PlayerId);
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            auto error = game->addPlayerProcess(request.PlayerId);
            if(error != TGameController::TError::OK)
            {
                result.Result = ConvertGameErrorToServerResult(error);
                break;
            }
            game->InitGame();
            player->addGame(request.GameId, false);
            result.GameStage = game->GameStage();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::ADD_COMPUTER:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            auto & player = TDataStorage::Instance().getPlayer(request.PlayerId);
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            if(!player->isHost(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_DOESNT_HAVE_RIGHTS;
                break;
            }
            if(request.ComputerId == 0)
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_COMPUTER_NOT_FOUND;
                break;
            }
            auto error = game->addComputerProcess(request.ComputerId, request.GameBrain);
            if(error != TGameController::TError::OK)
            {
                result.Result = ConvertGameErrorToServerResult(error);
                break;
            }
            game->InitGame();
            player->addComputer(request.GameId, request.ComputerId);
            result.GameStage = game->GameStage();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::START_GAME:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            auto & player = TDataStorage::Instance().getPlayer(request.PlayerId);
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            if(!player->isHost(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_DOESNT_HAVE_RIGHTS;
                break;
            }
            if(request.GameValue.size() > 0)
            {
                if(!TStandartRules::Instance().isValidGameValue(request.GameValue))
                {
                    result.SecretValue = request.GameValue;
                    result.Result = SERVER_COMPONENTS::TResult::ERROR_INVALID_SECRET_VALUE;
                    break;
                }
                auto error = game->StartGame(request.GameValue);
                if(error != TGameController::TError::OK)
                {
                    result.Result = ConvertGameErrorToServerResult(error);
                    break;
                }
            }
            else
            {
                auto error = game->StartGame();
                if(error != TGameController::TError::OK)
                {
                    result.Result = ConvertGameErrorToServerResult(error);
                    break;
                }
            }
            result.SecretValue = game->SecretValue();
            result.GameStage = game->GameStage();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::PLAYER_STEP:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            if(request.GameValue.size() == 0 || !TStandartRules::Instance().isValidGameValue(request.GameValue))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_INVALID_GAME_VALUE;
                break;
            }
            auto currentStep = game->GameStep(request.PlayerId, true);
            if(game->GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED)
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_ALREADY_FINISHED;
                break;
            }
            if(currentStep > game->GameStep())
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_ALREADY_MADE_STEP;
                break;
            }
            auto error = game->DoPlayerStep(request.PlayerId, request.GameValue);
            if(error != TGameController::TError::OK)
            {
                result.Result = ConvertGameErrorToServerResult(error);
                break;
            }
            result.Players = game->PlayersCount();
            result.UnsteppedPlayers = game->UnsteppedPlayers(currentStep);
            result.Steps = game->getStepResults(currentStep + 1);
            result.GameStage = game->GameStage();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::COMPUTER_STEP:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            if(request.ComputerId == 0 )
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_COMPUTER_NOT_FOUND;
                break;
            }
            auto currentStep = game->GameStep(request.ComputerId, false);
            auto error = game->FinishStep();
            if(error != TGameController::TError::OK)
            {
                result.Result = ConvertGameErrorToServerResult(error);
                break;
            }
            result.Players = game->PlayersCount();
            result.UnsteppedPlayers = game->UnsteppedPlayers(currentStep);
            result.Steps = game->getStepResults(currentStep + 1);
            result.GameStage = game->GameStage();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::SWITCH_BRAIN:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            auto & player = TDataStorage::Instance().getPlayer(request.PlayerId);
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            if(!player->isHost(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_DOESNT_HAVE_RIGHTS;
                break;
            }
            if(request.ComputerId == 0)
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_COMPUTER_NOT_FOUND;
                break;
            }
            if(game->GameStage() != MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER)
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_BRAIN_NOT_CHANGING_IN_PROGRESS;
                break;
            }
            auto error = game->switchGameBrain(request.ComputerId, request.GameBrain);
            if(error != TGameController::TError::OK)
            {
                result.Result = ConvertGameErrorToServerResult(error);
                break;
            }
            game->InitGame();
            result.GameStage = game->GameStage();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::PLAYER_GIVE_UP:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            auto currentStep = game->GameStep(request.PlayerId, true);
            auto error = game->PlayerGiveUp(request.PlayerId);
            if(error != TGameController::TError::OK)
            {
                result.Result = ConvertGameErrorToServerResult(error);
                break;
            }
            result.Players = game->PlayersCount();
            result.UnsteppedPlayers = game->UnsteppedPlayers(currentStep);
            result.Steps = game->getStepResults(currentStep + 1);
            result.GameStage = game->GameStage();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::STEP_INFO:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            bool isPlayer = request.ComputerId == 0;
            uint32_t id = isPlayer ? request.PlayerId : request.ComputerId;
            if(request.Step == 0 || request.Step > game->GameStep(id, isPlayer))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_INVALID_GAME_STEP;
                break;
            }
            auto stepResults = game->getProcessStepResults(id, isPlayer, request.Step);
            result.Players = game->PlayersCount();
            result.UnsteppedPlayers = game->UnsteppedPlayers(request.Step - 1);
            result.Steps.push_back(stepResults);
            result.GameStage = game->GameStage();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::GIVE_RIGHTS:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            auto & player = TDataStorage::Instance().getPlayer(request.PlayerId);
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            player->setHost(request.GameId, true);
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::LEAVE_FROM_GAME:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            auto & player = TDataStorage::Instance().getPlayer(request.PlayerId);
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            if(request.ComputerId > 0)
            {
                if(!player->gameExistComputer(request.GameId, request.ComputerId))
                {
                    result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_DOESNT_HAVE_RIGHTS;
                    break;
                }
                auto error = game->removePlayerProcess(request.ComputerId, false);
                if(error != TGameController::TError::OK)
                {
                    result.Result = ConvertGameErrorToServerResult(error);
                    break;
                }
                player->removeComputer(request.GameId, request.ComputerId);
            }
            else
            {
                if(player->ComputersCount(request.GameId) > 0)
                {
                    result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_HAVE_UNDELETED_COMPUTERS;
                    break;
                }
                auto error = game->removePlayerProcess(request.PlayerId, true);
                if(error != TGameController::TError::OK)
                {
                    result.Result = ConvertGameErrorToServerResult(error);
                    break;
                }
                player->removeGame(request.GameId);
            }
            if(game->PlayersCount() == 0)
            {
                TDataStorage::Instance().removeGame(request.GameId);
                result.GameStage = MODEL_COMPONENTS::TGameStage::UNKNOWN;
            }
            else
            {
                result.GameStage = game->GameStage();
            }
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::FINISH_GAME:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            auto & player = TDataStorage::Instance().getPlayer(request.PlayerId);
            if(!player->isHost(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_DOESNT_HAVE_RIGHTS;
                break;
            }
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            game->FinishGame();
            uint32_t lastStep = game->GameStep();
            result.Steps = game->getStepResults(lastStep);
            result.GameStage = game->GameStage();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::GAME_RESULT:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            if(game->GameStage() != MODEL_COMPONENTS::TGameStage::FINISHED)
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_IN_PROGRESS;
                break;
            }
            result.GameResults = game->getGameResults();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::REMOVE_GAME:
        {
            if(!TDataStorage::Instance().isPlayerExists(request.PlayerId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }
            auto & player = TDataStorage::Instance().getPlayer(request.PlayerId);
            if(!TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_NOT_EXISTS;
                break;
            }
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            if(game->PlayersCount() > 1)
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_NOT_ALL_PLAYERS_LEAVE_FROM_GAME;
                break;
            }
            auto error = game->removePlayerProcess(request.PlayerId, true);
            if(error != TGameController::TError::OK)
            {
                result.Result = ConvertGameErrorToServerResult(error);
                break;
            }
            player->removeGame(request.GameId);
            TDataStorage::Instance().removeGame(request.GameId);
            result.GameStage = MODEL_COMPONENTS::TGameStage::UNKNOWN;
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        case SERVER_COMPONENTS::TCommand::RESTORE_GAME:
        {
            if(TDataStorage::Instance().isGameExists(request.GameId))
            {
                result.Result = SERVER_COMPONENTS::TResult::ERROR_GAME_ALREADY_EXISTS;
                break;
            }
            TDataStorage::Instance().createGame(request.GameId);
            auto & game = TDataStorage::Instance().getGame(request.GameId);
            if(request.GameValue.size() == 0|| !TStandartRules::Instance().isValidGameValue(request.GameValue))
            {
                TDataStorage::Instance().removeGame(request.GameId);
                result.Result = SERVER_COMPONENTS::TResult::ERROR_INVALID_GAME_VALUE;
                break;
            }
            auto secret_value = request.GameValue;
            std::unordered_map<
                    uint32_t,
                    std::tuple<bool, MODEL_COMPONENTS::TGameBrain, TStandartPlayerProcess::THistoryList>
                >  restoreData;

            if(request.Players.size() == 0)
            {
                TDataStorage::Instance().removeGame(request.GameId);
                result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                break;
            }

            for( auto [id, is_host] : request.Players)
            {
                restoreData.try_emplace(
                    id,
                    std::make_tuple(true, MODEL_COMPONENTS::TGameBrain::NONE, TStandartPlayerProcess::THistoryList{})
                );
                if(!TDataStorage::Instance().isPlayerExists(id))
                {
                    TDataStorage::Instance().createPlayer(id);
                }
                auto & player = TDataStorage::Instance().getPlayer(id);
                if(player->GamesCount() >= SERVER_COMPONENTS::PLAYER_GAMES_LIMIT)
                {
                    TDataStorage::Instance().removeGame(request.GameId);
                    result.Result = SERVER_COMPONENTS::TResult::ERROR_GAMES_LIMIT_REACHED;
                    break;
                }
                if(!player->PlayerInGame(request.GameId))
                {
                    player->addGame(request.GameId, is_host);
                }
            }
            for( auto const & [id, brainData] : request.BrainsMap)
            {
                auto brain = brainData.second;
                auto player_id = brainData.first;
                restoreData.try_emplace(
                    id,
                    std::make_tuple(false, brain, TStandartPlayerProcess::THistoryList{})
                );
                if(!TDataStorage::Instance().isPlayerExists(player_id))
                {
                    TDataStorage::Instance().createPlayer(player_id);
                }
                auto & player = TDataStorage::Instance().getPlayer(player_id);
                player->addComputer(request.GameId, id);
            }

            for( auto const & item : request.History)
            {
                if(!restoreData.contains(item.processId))
                {
                    TDataStorage::Instance().removeGame(request.GameId);
                    result.Result = SERVER_COMPONENTS::TResult::ERROR_PLAYER_NOT_FOUND;
                    break;
                }
                auto & history = std::get<2>(restoreData[item.processId]);
                auto emptyData = std::make_pair(TGameValue<uint8_t>({0,0,0,0}), std::make_pair(0,0));
                if(history.size() < item.step)
                {
                    history.resize(item.step, emptyData);
                }
                std::pair<uint32_t, uint32_t> bc = std::make_pair(item.bulls, item.cows);
                auto gameValue = TGameValue<uint8_t>(item.gameValueList);
                history[item.step - 1] = {gameValue, bc};
            }
            game->restoreGame(secret_value, restoreData);
            result.GameStage = game->GameStage();
            result.Result = SERVER_COMPONENTS::TResult::COMPLETE;
        }
        break;
        default:
            result.Result = SERVER_COMPONENTS::TResult::ERROR_UNKNOWN_COMMAND;
            break;
        }
        TDataStorage::Instance().unlockAfterProcessing(request.GameId, request.PlayerId);
    }
    catch (...)
    {
        TDataStorage::Instance().unlockAfterProcessing(request.GameId, request.PlayerId);
        throw;
    }
    return result;
}

#if defined(KAFKA_SERVER)
void TEventProcessor::sendToKafka(const std::string& message)
{
    if (!m_kafka_producer) 
    {
        std::cerr << "Kafka producer is not initialized" << std::endl;
        return;
    }

    RdKafka::ErrorCode resp = m_kafka_producer->produce(
        m_producer_topic,
        RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<char*>(message.c_str()),
        message.size(),
        nullptr,
        0,
        0,
        nullptr
    );

    if (resp != RdKafka::ERR_NO_ERROR)
    {
        std::cerr << "Kafka produce failed: " << RdKafka::err2str(resp) << std::endl;
    }
    else
    {
        std::cout << "Successfully sent message to Kafka: " << message << std::endl;
    }

    m_kafka_producer->poll(0);
}
#endif
