#include "enums.hpp"
#include <sstream>
#include <cassert>

namespace MODEL_COMPONENTS
{
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

    std::ostream & operator<< ( std::ostream & stream_, TGameBrain _gameBrain )
    {
        switch( _gameBrain )
        {
            case TGameBrain::UNKNOWN    :
                stream_ << "UNKNOWN";
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
