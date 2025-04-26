#include <catch2/catch_test_macros.hpp>
#include <player_process.hpp>
#include <random>

TEST_CASE("2. Check player process", "[player_process]")
{
    TDecisionTreeBrain::DecisionTreePath = "../bc.json";
    SECTION("2.1. Check all states of player process")
    {
        std::random_device device;
        std::mt19937 random_generator;
        random_generator.seed(device());

        std::function< uint32_t( uint32_t ) > randomByModulus =
            [&random_generator]( uint32_t _modulus )->unsigned int
        {
            std::uniform_int_distribution<uint32_t> range(0, _modulus - 1);
            return range(random_generator);
        };
        auto playerProcess = TStandartPlayerProcess(randomByModulus);
        CHECK( playerProcess.isBrainless() );
        CHECK( playerProcess.PlayerState() == MODEL_COMPONENTS::TPlayerState::UNKNOWN );
        CHECK( playerProcess.HistoryList().size() == 0 );
        CHECK( playerProcess.AttemptsCount() == 0 );

        playerProcess.Init();
        CHECK( playerProcess.isBrainless() );
        CHECK( playerProcess.PlayerState() == MODEL_COMPONENTS::TPlayerState::WAIT_A_NUMBER );
        CHECK( playerProcess.HistoryList().size() == 0 );
        CHECK( playerProcess.AttemptsCount() == 0 );

        std::vector< uint8_t > secretValue{0,1,2,3};
        playerProcess.setTrueGameValue(TGameValue<uint8_t>(secretValue));
        CHECK( playerProcess.isBrainless() );
        CHECK( playerProcess.PlayerState() == MODEL_COMPONENTS::TPlayerState::IN_PROGRESS );
        CHECK( playerProcess.HistoryList().size() == 0 );
        CHECK( playerProcess.AttemptsCount() == 0 );

        std::vector< uint8_t > firstValue{4,5,6,7};
        playerProcess.appendGameValue(TGameValue<uint8_t>(firstValue));
        CHECK( playerProcess.isBrainless() );
        CHECK( playerProcess.PlayerState() == MODEL_COMPONENTS::TPlayerState::IN_PROGRESS );
        CHECK( playerProcess.HistoryList().size() == 1 );
        CHECK( playerProcess.AttemptsCount() == 1 );
        CHECK( playerProcess.HistoryList().back().first.List() == firstValue);
        CHECK( playerProcess.HistoryList().back().second.first == 0);
        CHECK( playerProcess.HistoryList().back().second.second == 0);

        playerProcess.appendGameValue(TGameValue<uint8_t>(secretValue));
        CHECK( playerProcess.isBrainless() );
        CHECK( playerProcess.PlayerState() == MODEL_COMPONENTS::TPlayerState::FINISHED );
        CHECK( playerProcess.HistoryList().size() == 2 );
        CHECK( playerProcess.AttemptsCount() == 2 );
        CHECK( playerProcess.HistoryList().back().first.List() == secretValue);
        CHECK( playerProcess.HistoryList().back().second.first == 4);
        CHECK( playerProcess.HistoryList().back().second.second == 0);
    }

    SECTION("2.2. Check to give up")
    {
        std::random_device device;
        std::mt19937 random_generator;
        random_generator.seed(device());

        std::function< uint32_t( uint32_t ) > randomByModulus =
            [&random_generator]( uint32_t _modulus )->unsigned int
        {
            std::uniform_int_distribution<uint32_t> range(0, _modulus - 1);
            return range(random_generator);
        };
        auto playerProcess = TStandartPlayerProcess(randomByModulus);
        playerProcess.Init();
        std::vector< uint8_t > secretValue{0,1,2,3};
        playerProcess.setTrueGameValue(TGameValue<uint8_t>(secretValue));
        std::vector< uint8_t > firstValue{4,5,6,7};
        playerProcess.appendGameValue(TGameValue<uint8_t>(firstValue));

        playerProcess.giveUp();
        CHECK( playerProcess.isBrainless() );
        CHECK( playerProcess.PlayerState() == MODEL_COMPONENTS::TPlayerState::GAVE_UP );
        CHECK( playerProcess.HistoryList().size() == 1 );
        CHECK( playerProcess.AttemptsCount() == 1 );
    }

    SECTION("2.3. Check process with brain")
    {
        std::random_device device;
        std::mt19937 random_generator;
        random_generator.seed(device());

        std::function< uint32_t( uint32_t ) > randomByModulus =
            [&random_generator]( uint32_t _modulus )->unsigned int
        {
            std::uniform_int_distribution<uint32_t> range(0, _modulus - 1);
            return range(random_generator);
        };
        auto playerProcess = TStandartPlayerProcess(randomByModulus);
        playerProcess.selectBrain(MODEL_COMPONENTS::TGameBrain::BEST);
        playerProcess.Init();
        CHECK( !playerProcess.isBrainless() );
        CHECK( playerProcess.PlayerState() == MODEL_COMPONENTS::TPlayerState::WAIT_A_NUMBER );
        CHECK( playerProcess.HistoryList().size() == 0 );
        CHECK( playerProcess.AttemptsCount() == 0 );
        std::vector< uint8_t > secretValue{0,1,2,3};
        playerProcess.setTrueGameValue(TGameValue<uint8_t>(secretValue));
        CHECK( !playerProcess.isBrainless() );
        CHECK( playerProcess.PlayerState() == MODEL_COMPONENTS::TPlayerState::IN_PROGRESS );
        CHECK( playerProcess.HistoryList().size() == 0 );
        CHECK( playerProcess.AttemptsCount() == 0 );

        playerProcess.makeStep();
        CHECK( !playerProcess.isBrainless() );
        auto isCorrectState =
            playerProcess.PlayerState() == MODEL_COMPONENTS::TPlayerState::IN_PROGRESS ||
            playerProcess.PlayerState() == MODEL_COMPONENTS::TPlayerState::FINISHED
        ;
        CHECK(isCorrectState );
        CHECK( playerProcess.HistoryList().size() == 1 );
        CHECK( playerProcess.AttemptsCount() == 1 );
    }
}
