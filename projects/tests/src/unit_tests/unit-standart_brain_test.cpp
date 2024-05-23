#include <catch2/catch.hpp>
#include <game_process.hpp>
#include <rules/standart_game_value_node.hpp>

TEST_CASE("1. Check standart brain", "[standart brain]")
{
    SECTION("1.1. Test standart rules")
    {
        CHECK(TStandartRules::Instance()->NumbersCount() == 10);
        CHECK(TStandartRules::Instance()->ValueSize() == 4);
        std::vector< uint8_t > checkingValue{1,1,3,4};
        CHECK(!TStandartRules::Instance()->isValidGameValueList(checkingValue));
        checkingValue = {10,1,2,3};
        CHECK(!TStandartRules::Instance()->isValidGameValueList(checkingValue));
        checkingValue = {0,1,2,3,4};
        CHECK(!TStandartRules::Instance()->isValidGameValueList(checkingValue));
        checkingValue = {9,8,7,6};
        CHECK(TStandartRules::Instance()->isValidGameValueList(checkingValue));
        checkingValue = {0,1,2,3};
        CHECK(TStandartRules::Instance()->isValidGameValueList(checkingValue));

        std::vector< uint8_t > secondValue{4,5,6,7};
        auto results = TStandartRules::Instance()->calculateBullsAndCows(checkingValue, secondValue);
        CHECK( !TStandartRules::Instance()->isWinResults(results) );
        CHECK( results.first == 0 );
        CHECK(results.second == 0 );

        results = TStandartRules::Instance()->calculateBullsAndCows(checkingValue, checkingValue);
        CHECK( TStandartRules::Instance()->isWinResults(results) );
        CHECK( results.first == 4 );
        CHECK(results.second == 0 );

        checkingValue = {3,5,2,6};
        results = TStandartRules::Instance()->calculateBullsAndCows(checkingValue, secondValue);
        CHECK( !TStandartRules::Instance()->isWinResults(results) );
        CHECK( results.first == 1 );
        CHECK(results.second == 1 );
        CHECK( TStandartRules::Instance()->AllPossibleGameValues().size() == 5040 );

    }

    SECTION("1.2. Test standart brains")
    {

        std::shared_ptr<TStandartGameProcess> game_process = std::make_shared<TStandartGameProcess>();
        REQUIRE( game_process );

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
            game_process->selectBrain(brainType);
            game_process->Init();
            CHECK(game_process->AttemptsCount() == 0);
            CHECK(game_process->GameStage() == MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER);
            game_process->setTrueGameValue(TGameValue<uint8_t>(trueValue));
            CHECK(game_process->GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
            uint32_t attempts = 0;
            while(game_process->GameStage() != MODEL_COMPONENTS::TGameStage::FINISHED && attempts < 5400)
            {
                CHECK(game_process->GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
                CHECK(game_process->HistoryList().size() == attempts);
                game_process->makeStep();
                ++attempts;
            }
            auto winValue  = game_process->HistoryList().back();
            CHECK( TStandartRules::Instance()->isWinResults(winValue.second) );
            auto results = TStandartRules::Instance()->calculateBullsAndCows(winValue.first, trueValue);
            CHECK( TStandartRules::Instance()->isWinResults(results) );
        }
    }
}
