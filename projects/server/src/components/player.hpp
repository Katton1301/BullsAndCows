#pragma once
#include <components/enums.hpp>
#include <set>

namespace SERVER_COMPONENTS
{
    enum
    {
        PLAYER_GAMES_LIMIT = 10,
    };
    struct TGameData
    {
        bool isHost = false;
        std::set<uint32_t> Computers;
    };

    class TPlayer
    {
    public:
        TPlayer() = delete;
        TPlayer(uint32_t _id);
        ~TPlayer() = default;

        bool isHost(uint32_t _gameId) const;
        uint32_t GamesCount() const;
        bool PlayerInGame(uint32_t _gameId) const;
        uint32_t ComputersCount(uint32_t _gameId) const;

        void addGame(uint32_t _gameId, bool _host);
        void setHost(uint32_t _gameId, bool _host);
        void removeGame(uint32_t _gameId);
        bool gameExistComputer(uint32_t _gameId, uint32_t _computerId) const;
        void addComputer(uint32_t _gameId, uint32_t _computerId);
        void removeComputer(uint32_t _gameId, uint32_t _computerId);

    private:
        uint32_t m_id = 0;
        std::map<uint32_t, TGameData> m_gamesIds{};
    };
}
