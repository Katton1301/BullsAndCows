#include <components/enums.hpp>

namespace SERVER_COMPONENTS
{
    std::ostream& operator<<( std::ostream& stream_, TServerState id )
    {
        switch( id )
        {
        case TServerState::UNKNOWN  :
            stream_ << "TServerState::UNKNOWN";
            break;

        case TServerState::WAIT_REGISTRATION  :
            stream_ << "TServerState::WAIT_REGISTRATION";
            break;

        case TServerState::READY_TO_WORK  :
            stream_ << "TServerState::READY_TO_WORK";
            break;

        default :
            std::cout << "< Wrong server state identifier : " << static_cast< int >( id ) << " >";
            assert( false && "Wrong server state identifier" );

            break;
        }
        return stream_;
    }

    std::ostream& operator<<( std::ostream& stream_, TCommand id )
    {
        switch( id )
        {
        case TCommand::UNKNOWN  :
            stream_ << "TCommand::UNKNOWN";
            break;

        case TCommand::REGISTER_SERVER  :
            stream_ << "TCommand::REGISTER_SERVER";
            break;

        case TCommand::SERVER_INFO  :
            stream_ << "TCommand::SERVER_INFO";
            break;

        case TCommand::DISCONNECT_SERVER  :
            stream_ << "TCommand::DISCONNECT_SERVER";
            break;

        case TCommand::CREATE_GAME  :
            stream_ << "TCommand::CREATE_GAME";
            break;

        case TCommand::ADD_PLAYER  :
            stream_ << "TCommand::ADD_PLAYER";
            break;

        case TCommand::ADD_COMPUTER  :
            stream_ << "TCommand::ADD_COMPUTER";
            break;

        case TCommand::START_GAME  :
            stream_ << "TCommand::START_GAME";
            break;

        case TCommand::PLAYER_STEP  :
            stream_ << "TCommand::PLAYER_STEP";
            break;

        case TCommand::COMPUTER_STEP  :
            stream_ << "TCommand::COMPUTER_STEP";
            break;

        case TCommand::SWITCH_BRAIN  :
            stream_ << "TCommand::SWITCH_BRAIN";
            break;

        case TCommand::PLAYER_GIVE_UP  :
            stream_ << "TCommand::PLAYER_GIVE_UP";
            break;

        case TCommand::STEP_INFO  :
            stream_ << "TCommand::STEP_INFO";
            break;

        case TCommand::GIVE_RIGHTS  :
            stream_ << "TCommand::GIVE_RIGHTS";
            break;

        case TCommand::LEAVE_FROM_GAME  :
            stream_ << "TCommand::LEAVE_FROM_GAME";
            break;

        case TCommand::FINISH_GAME  :
            stream_ << "TCommand::FINISH_GAME";
            break;

        case TCommand::GAME_RESULT  :
            stream_ << "TCommand::GAME_RESULT";
            break;
        case TCommand::REMOVE_GAME  :
            stream_ << "TCommand::REMOVE_GAME";
            break;
        case TCommand::RESTORE_GAME  :
            stream_ << "TCommand::RESTORE_GAME";
            break;

        default :
            std::cout << "< Wrong command identifier : " << static_cast< int >( id ) << " >";
            assert( false && "Wrong command identifier" );

            break;
        }
        return stream_;
    }

    std::ostream& operator<<( std::ostream& stream_, TResult id )
    {
        switch( id )
        {
        case TResult::UNKNOWN  :
            stream_ << "TResult::UNKNOWN";
            break;

        case TResult::ERROR_PARSE_MESSAGE  :
            stream_ << "TResult::ERROR_PARSE_MESSAGE";
            break;

        case TResult::ERROR_SERVER_NOT_REGISTRED  :
            stream_ << "TResult::ERROR_SERVER_NOT_REGISTRED";
            break;

        case TResult::ERROR_SERVER_ALREADY_REGISTRED  :
            stream_ << "TResult::ERROR_SERVER_ALREADY_REGISTRED";
            break;

        case TResult::ERROR_BAD_SERVER_ID  :
            stream_ << "TResult::ERROR_BAD_SERVER_ID";
            break;

        case TResult::ERROR_MESSAGE_FOR_ANOTHER_SERVER  :
            stream_ << "TResult::ERROR_MESSAGE_FOR_ANOTHER_SERVER";
            break;

        case TResult::ERROR_UNKNOWN_COMMAND  :
            stream_ << "TResult::ERROR_UNKNOWN_COMMAND";
            break;

        case TResult::ERROR_GAMES_LIMIT_REACHED  :
            stream_ << "TResult::ERROR_GAMES_LIMIT_REACHED";
            break;

        case TResult::ERROR_PROCESS_LIMIT_REACHED  :
            stream_ << "TResult::ERROR_PROCESS_LIMIT_REACHED";
            break;

        case TResult::ERROR_UNKNOWN_BRAIN  :
            stream_ << "TResult::ERROR_UNKNOWN_BRAIN";
            break;

        case TResult::ERROR_PLAYER_NOT_FOUND  :
            stream_ << "TResult::ERROR_PLAYER_NOT_FOUND";
            break;

        case TResult::ERROR_GAME_NOT_EXISTS  :
            stream_ << "TResult::ERROR_GAME_NOT_EXISTS";
            break;

        case TResult::ERROR_COMPUTER_NOT_FOUND  :
            stream_ << "TResult::ERROR_COMPUTER_NOT_FOUND";
            break;

        case TResult::ERROR_GAME_ALREADY_EXISTS  :
            stream_ << "TResult::ERROR_GAME_ALREADY_EXISTS";
            break;

        case TResult::ERROR_INVALID_SECRET_VALUE  :
            stream_ << "TResult::ERROR_INVALID_SECRET_VALUE";
            break;

        case TResult::ERROR_INVALID_GAME_VALUE  :
            stream_ << "TResult::ERROR_INVALID_GAME_VALUE";
            break;

        case TResult::ERROR_GAME_ALREADY_FINISHED  :
            stream_ << "TResult::ERROR_GAME_ALREADY_FINISHED";
            break;

        case TResult::ERROR_PLAYER_ALREADY_FINISHED  :
            stream_ << "TResult::ERROR_PLAYER_ALREADY_FINISHED";
            break;

        case TResult::ERROR_PLAYER_ALREADY_EXISTS  :
            stream_ << "TResult::ERROR_PLAYER_ALREADY_EXISTS";
            break;

        case TResult::ERROR_COMPUTER_ALREADY_EXISTS  :
            stream_ << "TResult::ERROR_COMPUTER_ALREADY_EXISTS";
            break;

        case TResult::ERROR_NOT_ALL_PLAYERS_STEPED  :
            stream_ << "TResult::ERROR_NOT_ALL_PLAYERS_STEPED";
            break;

        case TResult::ERROR_GAME_IN_PROGRESS  :
            stream_ << "TResult::ERROR_GAME_IN_PROGRESS";
            break;

        case TResult::ERROR_PLAYER_ALREADY_MADE_STEP  :
            stream_ << "TResult::ERROR_PLAYER_ALREADY_MADE_STEP";
            break;

        case TResult::ERROR_PLAYER_HAVE_UNDELETED_COMPUTERS  :
            stream_ << "TResult::ERROR_PLAYER_HAVE_UNDELETED_COMPUTERS";
            break;

        case TResult::ERROR_PLAYER_DOESNT_HAVE_RIGHTS  :
            stream_ << "TResult::ERROR_PLAYER_DOESNT_HAVE_RIGHTS";
            break;

        case TResult::ERROR_NOT_ALL_PLAYERS_LEAVE_FROM_GAME  :
            stream_ << "TResult::ERROR_NOT_ALL_PLAYERS_LEAVE_FROM_GAME";
            break;

        case TResult::ERROR_BRAIN_NOT_CHANGING_IN_PROGRESS  :
            stream_ << "TResult::ERROR_BRAIN_NOT_CHANGING_IN_PROGRESS";
            break;

        case TResult::ERROR_INVALID_GAME_STEP  :
            stream_ << "TResult::ERROR_INVALID_GAME_STEP";
            break;

        case TResult::COMPLETE  :
            stream_ << "TCommand::COMPLETE";
            break;

        default :
            std::cout << "< Wrong result identifier : " << static_cast< int >( id ) << " >";
            assert( false && "Wrong result identifier" );

            break;
        }
        return stream_;
    }

    std::ostream& operator<<( std::ostream& stream_, TResultData const & result )
    {
        stream_ << "Server: " << result.ServerId << std::endl;
        stream_ << "Correlation Id: " << result.CorrelationId << std::endl;
        stream_ << "Result: " << result.Result << std::endl;
        stream_ << "Game Stage: " << result.GameStage << std::endl;
        stream_ << "Game: " << result.GameId << std::endl;
        stream_ << "Player: " << result.PlayerId << std::endl;
        stream_ << "Players: " << result.Players << std::endl;
        stream_ << "Unstepped Players: " << result.UnsteppedPlayers << std::endl;
        for(auto const & step : result.Steps)
        {
            if(step.player)
            {
                stream_ << "Player ";
            }
            else
            {
                stream_ << "Computer ";
            }
            stream_ << "Id: " << step.processId << std::endl;
            stream_ << "Bulls: " << step.bulls << std::endl;
            stream_ << "Cows: " << step.cows << std::endl;
            stream_ << "Step: " << step.step << std::endl;
            stream_
                << "Game Value: "
                << step.gameValueList[0]
                << step.gameValueList[1]
                << step.gameValueList[2]
                << step.gameValueList[3]
                << std::endl;
        }
        stream_ << "Game Ids: ";
        for( auto id : result.GameIds)
        {
            stream_ << id << " ";
        }
        stream_ << std::endl;
        return stream_;
    }

    std::ostream& operator<<( std::ostream& stream_, TRequestData const & request )
    {
        stream_ << "Command: " << request.Command << std::endl;
        stream_ << "Correlation Id: " << request.CorrelationId << std::endl;
        stream_ << "Player: " << request.PlayerId << std::endl;
        stream_ << "Game: " << request.GameId << std::endl;
        stream_ << "Computer: " << request.ComputerId << std::endl;
        if(request.GameBrain != MODEL_COMPONENTS::TGameBrain::NONE)
        {
            stream_ << "Game Brain: " << request.GameBrain << std::endl;
        }
        stream_ << "Value: " << std::endl;
        for(auto digit : request.GameValue)
        {
            stream_ << static_cast<uint32_t>(digit) << " ";
        }
        if(request.History.size() > 0)
        {
            stream_ << "History:" << std::endl;
            for(auto const & history : request.History)
            {
                stream_ << "History Step:" << std::endl;
                stream_ << ((history.player) ? "  player id:" : "computer id:") << history.processId << std::endl;
                stream_ << "Step: " << history.step << std::endl;
                stream_ << "Bulls: " << history.bulls << std::endl;
                stream_ << "Cows: " << history.cows << std::endl;
                stream_ << "Game Value: " << history.cows << std::endl;
                for(auto digit : history.gameValueList)
                {
                    stream_ << static_cast<uint32_t>(digit) << " ";
                }
                if(history.finished)
                {
                    stream_ << "Last step" << std::endl;
                }
            }
        }
        if(request.BrainsMap.size() > 0)
        {
            stream_ << "Brains:" << std::endl;
            for(auto const & [id, data] : request.BrainsMap)
            {
                stream_ << "Brain: " << std::endl;
                stream_ << "Id: " << id << std::endl;
                stream_ << "Owner id: " << data.first << std::endl;
                stream_ << "Game Brain: " << data.second << std::endl;
            }
        }
        stream_ << std::endl;
        return stream_;
    }
}
