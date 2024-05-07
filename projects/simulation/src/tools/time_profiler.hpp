#pragma once

#include <iostream>
#include <chrono>

class TTimeProfiler
{
    public  :   //methods
        TTimeProfiler( ) : m_strName( "" )
        {
            init( );
        }

        ~TTimeProfiler( )
        {

        }

        inline void init( )
        {
            reset( );

            return;
        }

        inline void reset( )
        {
            setTimeStart( std::chrono::high_resolution_clock::now( ) );
            setSummaryTime( 0u );
            setCounter( 0u );

            return;
        }


        inline void start( )
        {
            m_timeStart = std::chrono::high_resolution_clock::now( );
            return;
        }

        inline void stop( )
        {
            std::chrono::high_resolution_clock::time_point timeFinish = std::chrono::high_resolution_clock::now( );
            uint64_t uiLength = std::chrono::duration_cast< std::chrono::microseconds >( timeFinish - m_timeStart ).count( );;

            setCounter( Counter( ) + 1u );
            setSummaryTime( SummaryTime( ) + uiLength );

            return;
        }

        inline double calculateAverageDuration( ) const
        {
            if ( Counter( ) == 0 )
                return 0.0;

            return ( static_cast< double >( SummaryTime( ) ) / static_cast< double >( Counter( ) ) );
        }

        inline const std::string& Name( ) const { return m_strName; }
        inline void setName( const std::string& _strName ) { m_strName = _strName; }

        inline uint64_t           Counter( )      const   { return m_uiCounter; }
        inline uint64_t           SummaryTime( )  const   { return m_uiSummaryTime; }

    private :   //methods
        inline const std::chrono::high_resolution_clock::time_point&    TimeStart( )    const   { return m_timeStart; }

        inline void setTimeStart( const std::chrono::high_resolution_clock::time_point& _timeStart )    {   m_timeStart = _timeStart; }
        inline void setSummaryTime( uint64_t _uiSummaryTime )     {   m_uiSummaryTime = _uiSummaryTime; }
        inline void setCounter( uint64_t _uiCounter )             {   m_uiCounter = _uiCounter; }

    private :   //attributes
        std::string m_strName;
        std::chrono::high_resolution_clock::time_point  m_timeStart;
        uint64_t      m_uiSummaryTime;
        uint64_t      m_uiCounter;
};

inline std::ostream& operator<<( std::ostream& streamOutput_, const TTimeProfiler& _profiler )
{
    streamOutput_   << "profiler : " << _profiler.Name( )
                    << "\taverage duration of operation : " << _profiler.calculateAverageDuration( ) << " us.";
    return streamOutput_;
}
