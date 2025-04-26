#include <components/player.hpp>

namespace SERVER_COMPONENTS
{

    TPlayer::TPlayer(uint32_t _id)
    {
        m_id = _id;
    }

    bool TPlayer::isHost(uint32_t _gameId) const
    {
        return m_gamesIds.contains(_gameId) && m_gamesIds.at(_gameId).isHost;
    }

    uint32_t TPlayer::ComputersCount(uint32_t _gameId) const
    {
        if(!m_gamesIds.contains(_gameId))
        {
            return 0;
        }
        return m_gamesIds.at(_gameId).Computers.size();
    }

    void TPlayer::addGame(uint32_t _gameId, bool _host)
    {
        TGameData data;
        data.isHost = _host;
        m_gamesIds[_gameId] = data;
    }
    void TPlayer::setHost(uint32_t _gameId, bool _host)
    {
        m_gamesIds[_gameId].isHost = _host;
    }
    void TPlayer::removeGame(uint32_t _gameId)
    {
        m_gamesIds.erase(_gameId);
    }
    bool TPlayer::gameExistComputer(uint32_t _gameId, uint32_t _computerId) const
    {
        return m_gamesIds.contains(_gameId) && m_gamesIds.at(_gameId).Computers.contains(_computerId);
    }
    void TPlayer::addComputer(uint32_t _gameId, uint32_t _computerId)
    {
        m_gamesIds[_gameId].Computers.emplace(_computerId);
    }
    void TPlayer::removeComputer(uint32_t _gameId, uint32_t _computerId)
    {
        if(m_id == _computerId)
        {
            removeGame(_gameId);
            return;
        }
        m_gamesIds[_gameId].Computers.erase(_computerId);
    }

    uint32_t TPlayer::GamesCount() const
    {
        return m_gamesIds.size();
    }

    bool TPlayer::PlayerInGame(uint32_t _gameId) const
    {
        return m_gamesIds.contains(_gameId);
    }
}
