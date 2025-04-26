#include <catch2/catch_test_macros.hpp>
#include <player_process.hpp>
#include <rules/standart_game_value_node.hpp>
#include <random>

TEST_CASE("1. Check standart brain", "[standart brain]")
{
    TDecisionTreeBrain::DecisionTreePath = "../bc.json";
    SECTION("1.1. Test standart rules")
    {
        CHECK(TStandartRules::Instance().NumbersCount() == 10);
        CHECK(TStandartRules::Instance().ValueSize() == 4);
        CHECK(TStandartRules::Instance().AllValuesNumber() == 5040);
        std::vector< uint8_t > checkingValue{1,1,3,4};
        CHECK(!TStandartRules::Instance().isValidGameValueList(checkingValue));
        checkingValue = {10,1,2,3};
        CHECK(!TStandartRules::Instance().isValidGameValueList(checkingValue));
        checkingValue = {0,1,2,3,4};
        CHECK(!TStandartRules::Instance().isValidGameValueList(checkingValue));
        checkingValue = {9,8,7,6};
        CHECK(TStandartRules::Instance().isValidGameValueList(checkingValue));
        checkingValue = {0,1,2,3};
        CHECK(TStandartRules::Instance().isValidGameValueList(checkingValue));

        std::vector< uint8_t > secondValue{4,5,6,7};
        auto results = TStandartRules::Instance().calculateBullsAndCows(checkingValue, secondValue);
        CHECK( !TStandartRules::Instance().isWinResults(results) );
        CHECK( results.first == 0 );
        CHECK(results.second == 0 );

        results = TStandartRules::Instance().calculateBullsAndCows(checkingValue, checkingValue);
        CHECK( TStandartRules::Instance().isWinResults(results) );
        CHECK( results.first == 4 );
        CHECK(results.second == 0 );

        checkingValue = {3,5,2,6};
        results = TStandartRules::Instance().calculateBullsAndCows(checkingValue, secondValue);
        CHECK( !TStandartRules::Instance().isWinResults(results) );
        CHECK( results.first == 1 );
        CHECK(results.second == 1 );
        CHECK( TStandartRules::Instance().AllPossibleGameValues().size() == 5040 );

    }

    SECTION("1.2. Test standart brains")
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
        std::shared_ptr<TStandartPlayerProcess> player_process = std::make_shared<TStandartPlayerProcess>(randomByModulus);
        REQUIRE( player_process );

        std::vector<MODEL_COMPONENTS::TGameBrain> brainTypesVector
        {
            MODEL_COMPONENTS::TGameBrain::RANDOM,
            MODEL_COMPONENTS::TGameBrain::STUPID,
            MODEL_COMPONENTS::TGameBrain::SMART,
            MODEL_COMPONENTS::TGameBrain::BEST
        };
        std::vector<uint8_t> trueValue{0,1,2,3};
        for(auto const & brainType : brainTypesVector)
        {
            player_process->selectBrain(brainType);
            player_process->Init();
            CHECK(player_process->AttemptsCount() == 0);
            CHECK(player_process->PlayerState() == MODEL_COMPONENTS::TPlayerState::WAIT_A_NUMBER);
            player_process->setTrueGameValue(TGameValue<uint8_t>(trueValue));
            CHECK(player_process->PlayerState() == MODEL_COMPONENTS::TPlayerState::IN_PROGRESS);
            uint32_t attempts = 0;
            while(player_process->PlayerState() != MODEL_COMPONENTS::TPlayerState::FINISHED && attempts < 5400)
            {
                CHECK(player_process->PlayerState() == MODEL_COMPONENTS::TPlayerState::IN_PROGRESS);
                CHECK(player_process->HistoryList().size() == attempts);
                player_process->makeStep();
                ++attempts;
            }
            auto winValue  = player_process->HistoryList().back();
            CHECK( TStandartRules::Instance().isWinResults(winValue.second) );
            auto results = TStandartRules::Instance().calculateBullsAndCows(winValue.first, trueValue);
            CHECK( TStandartRules::Instance().isWinResults(results) );
        }
    }
}
