#pragma once
#include <unordered_map>
#include <set>
#include <mutex>
#include <memory>
#include <string>
#include <components/player.hpp>
#include <game_controller.h>

class TDataStorage
{
public:
    TDataStorage(const TDataStorage&) = delete;
    TDataStorage& operator=(const TDataStorage&) = delete;

    static TDataStorage& Instance() {
        static TDataStorage instance;
        return instance;
    }

    void clearStorage()
    {
        std::lock_guard<std::mutex> lock1(m_processing_mutex);
        std::lock_guard<std::mutex> lock2(m_gamemutex);
        std::lock_guard<std::mutex> lock3(m_playermutex);
        m_games.clear();
        m_players.clear();
        m_locked_games.clear();
        m_locked_players.clear();
    }

    bool isGameExists(uint32_t _gameId)
    {
        std::lock_guard<std::mutex> lock(m_gamemutex);
        return m_games.contains(_gameId);
    }

    void createGame( uint32_t _gameId )
    {
        std::lock_guard<std::mutex> lock(m_gamemutex);
        m_games.emplace(_gameId, std::make_unique<TGameController>());
        m_games[_gameId]->InitGame();
    }

    std::unique_ptr<TGameController> & getGame(uint32_t _gameId)
    {
        std::lock_guard<std::mutex> lock(m_gamemutex);
        return m_games[_gameId];
    }

    void removeGame(uint32_t _gameId)
    {
        std::lock_guard<std::mutex> lock(m_gamemutex);
        m_games.erase(_gameId);
    }

    bool isPlayerExists(uint32_t _playerId)
    {
        std::lock_guard<std::mutex> lock(m_playermutex);
        return m_players.contains(_playerId);
    }

    void createPlayer( uint32_t _playerId )
    {
        std::lock_guard<std::mutex> lock(m_playermutex);
        m_players.emplace(_playerId, std::make_unique<SERVER_COMPONENTS::TPlayer>(_playerId));
    }

    std::unique_ptr<SERVER_COMPONENTS::TPlayer> & getPlayer(uint32_t _playerId)
    {
        std::lock_guard<std::mutex> lock(m_playermutex);
        return m_players[_playerId];
    }

    std::vector<uint32_t> getAllGameIds()
    {
        std::vector<uint32_t> gameIds;
        for( auto const & [id, game] : m_games)
        {
            gameIds.push_back(id);
        }
        return gameIds;
    }

    void removePlayer(uint32_t _playerId)
    {
        std::lock_guard<std::mutex> lock(m_playermutex);
        m_players.erase(_playerId);
    }

    bool lockForProcessing( uint32_t _gameId, uint32_t _playerId )
    {
        std::unique_lock<std::mutex> lock(m_processing_mutex);

        if (m_locked_games.contains(_gameId))
        {
            return false;
        }
        if (m_locked_players.contains(_playerId))
        {
            return false;
        }

        m_locked_games.insert(_gameId);
        m_locked_players.insert(_playerId);

        return true;
    }

    void unlockAfterProcessing(uint32_t _gameId, uint32_t _playerId)
    {
        std::lock_guard<std::mutex> lock(m_processing_mutex);
        m_locked_games.erase(_gameId);
        m_locked_players.erase(_playerId);
    }

private:
    TDataStorage() = default;

    std::unordered_map<uint32_t, std::unique_ptr<TGameController> > m_games{};
    std::mutex m_gamemutex;
    std::unordered_map<uint32_t, std::unique_ptr<SERVER_COMPONENTS::TPlayer>> m_players{};
    std::mutex m_playermutex;

    std::set<uint32_t> m_locked_games{};
    std::set<uint32_t> m_locked_players{};
    std::mutex m_processing_mutex;
};
