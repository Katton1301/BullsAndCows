#pragma once
#include <string>
#include <sstream>
#include <core/enums.hpp>
namespace COMMON_OPERATIONS
{

    template< typename TParam >
    std::string convertToStdString( const TParam& _param )
    {
        std::stringstream streamTransformer;
        streamTransformer << _param;

        const size_t LINE_SIZE = 255;
        char bufferedLine[ LINE_SIZE ];
        std::fill( bufferedLine, bufferedLine + LINE_SIZE, 0 );
        streamTransformer.read( bufferedLine, LINE_SIZE );
        std::string strParameter ( bufferedLine );

        return strParameter;
    }

    template< typename TParam >
    TParam convertFromStdString(
        std::string const & _string,
        TParam const & _id_default,
        TParam _begin_ids_list,
        TParam _end_ids_list
    )
    {
        TParam result_id = _id_default;
        for (   uint32_t id  = static_cast< uint32_t >( _begin_ids_list );
                id != static_cast< uint32_t >( _end_ids_list );
                ++id )
        {
            std::string string_id = "";
            std::stringstream stream_transformer;
            stream_transformer << static_cast< TParam >( id );
            stream_transformer >> string_id;
            if ( string_id == _string )
            {
                result_id = static_cast< TParam >( id );
                break;
            }
        }
        return result_id;
    }

    inline std::string GameBrainToLevelName( MODEL_COMPONENTS::TGameBrain _gameMode )
    {
        switch ( _gameMode ) {
        case MODEL_COMPONENTS::TGameBrain::RANDOM :
            return std::string("Easiest");
            break;
        case MODEL_COMPONENTS::TGameBrain::STUPID :
            return std::string("Easy");
            break;
        case MODEL_COMPONENTS::TGameBrain::SMART :
            return std::string("Medium");
            break;
        case MODEL_COMPONENTS::TGameBrain::BEST :
            return std::string("Hard");
            break;
        default :
            return std::string("UNKNOWN");
            break;
        }
    }
    inline MODEL_COMPONENTS::TGameBrain LevelNameToGameBrain( std::string _levelName )
    {
        if(_levelName == "Easiest")
        {
            return MODEL_COMPONENTS::TGameBrain::RANDOM;
        }
        if(_levelName == "Easy")
        {
            return MODEL_COMPONENTS::TGameBrain::STUPID;
        }
        if(_levelName == "Medium")
        {
            return MODEL_COMPONENTS::TGameBrain::SMART;
        }
        if(_levelName == "Hard")
        {
            return MODEL_COMPONENTS::TGameBrain::BEST;
        }

        return MODEL_COMPONENTS::TGameBrain::NONE;
    }
}


