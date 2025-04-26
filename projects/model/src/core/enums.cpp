#include <core/enums.hpp>
#include <sstream>
#include <cassert>

namespace MODEL_COMPONENTS
{

    std::ostream& operator<<( std::ostream& stream_, TPlayerState _playerState )
    {
        switch( _playerState )
        {
        case TPlayerState::UNKNOWN    :
            stream_ << "UNKNOWN";
            break;

        case TPlayerState::WAIT_A_NUMBER  :
            stream_ << "WAIT_A_NUMBER";
            break;

        case TPlayerState::IN_PROGRESS  :
            stream_ << "IN_PROGRESS";
            break;

        case TPlayerState::GAVE_UP  :
            stream_ << "GAVE_UP";
            break;

        case TPlayerState::FINISHED  :
            stream_ << "FINISHED";
            break;

        default :
            stream_ << "[Error] identifier of player state is wrong : " << static_cast< int32_t >( _playerState ) << std::endl;
            assert( false && " incorrect player state identifier" );
            break;
        }
        return stream_;
    }

    std::ostream & operator<< ( std::ostream & stream_, TGameStage _gameStage )
    {
        switch( _gameStage )
        {
            case TGameStage::UNKNOWN    :
                stream_ << "UNKNOWN";
                break;

            case TGameStage::WAIT_A_NUMBER  :
                stream_ << "WAIT_A_NUMBER";
                break;

            case TGameStage::IN_PROGRESS  :
                stream_ << "IN_PROGRESS";
                break;

            case TGameStage::IN_PROGRESS_WINNER_DEFINED  :
                stream_ << "IN_PROGRESS_WINNER_DEFINED";
                break;

            case TGameStage::FINISHED  :
                stream_ << "FINISHED";
                break;

            default :
                stream_ << "[Error] identifier of game stage is wrong : " << static_cast< int32_t >( _gameStage ) << std::endl;
                assert( false && " incorrect game stage identifier" );
                break;
        }
        return stream_;
    }

    std::string gameStageToString(TGameStage _gameStage)
    {
        return (std::stringstream() << _gameStage).str();
    }

    std::ostream & operator<< ( std::ostream & stream_, TGameBrain _gameBrain )
    {
        switch( _gameBrain )
        {
            case TGameBrain::NONE    :
                stream_ << "NONE";
                break;

            case TGameBrain::RANDOM  :
                stream_ << "RANDOM";
                break;

            case TGameBrain::STUPID  :
                stream_ << "STUPID";
                break;

            case TGameBrain::SMART  :
                stream_ << "SMART";
                break;

            case TGameBrain::BEST  :
                stream_ << "BEST";
                break;

            default :
                stream_ << "[Error] identifier of game brain is wrong : " << static_cast< int32_t >( _gameBrain ) << std::endl;
                assert( false && " incorrect game brain identifier" );
                break;
        }
        return stream_;
    }
}
