#include <catch2/catch_test_macros.hpp>
#include <game_controller.h>

TEST_CASE("3. Check game controller", "[standart brain]")
{
    TDecisionTreeBrain::DecisionTreePath = "../bc.json";
    SECTION("3.1. Check game controller with one player")
    {
        TGameController gameController;
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::UNKNOWN);
        CHECK(gameController.GameStep() == 0);

        gameController.InitGame();
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER);
        CHECK(gameController.GameStep() == 0);
        uint32_t playerId = 1;
        auto error = gameController.addPlayerProcess(playerId);
        CHECK( error == TGameController::TError::OK );

        error = gameController.addPlayerProcess(playerId);
        CHECK( error == TGameController::TError::PLAYER_ALREADY_EXISTS );

        std::vector< uint8_t > secretValue{0,1,2,3};
        error = gameController.StartGame(secretValue);
        CHECK( error == TGameController::TError::OK );
        CHECK( secretValue == gameController.SecretValue() );
        error = gameController.StartGame(secretValue);
        CHECK( error == TGameController::TError::GAME_BUSY );
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
        CHECK(gameController.GameStep(playerId, true) == 0);
        CHECK(gameController.GameStep() == 0);
        CHECK(gameController.PlayerPlace(playerId, true) == 0);
        CHECK(gameController.getStepResults(1).size() == 0);
        CHECK(gameController.UnsteppedPlayers() == 1);

        std::vector< uint8_t > firstValue{4,5,6,7};
        error = gameController.DoPlayerStep(playerId, firstValue);
        CHECK( error == TGameController::TError::OK );
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
        CHECK(!gameController.isStepInTransitionStage());
        CHECK(gameController.GameStep(playerId, true) == 1);
        CHECK(gameController.GameStep(playerId, false) == 0);
        CHECK(gameController.GameStep(0, true) == 0);
        CHECK(gameController.GameStep() == 1);
        CHECK(gameController.WinnersId().size() == 0);
        CHECK(gameController.UnsteppedPlayers(0) == 0);

        auto results = gameController.getStepResults(1);
        CHECK(results.size() == 1);
        auto playerResult = results.back();
        CHECK(playerResult.processId == playerId);
        CHECK(playerResult.player);
        CHECK(playerResult.step == 1);
        CHECK(playerResult.gameValueList == firstValue);
        CHECK(playerResult.bulls == 0);
        CHECK(playerResult.cows == 0);
        CHECK(!playerResult.finished);

        error = gameController.DoPlayerStep(playerId, secretValue);
        CHECK( error == TGameController::TError::OK );
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED);
        CHECK(!gameController.isStepInTransitionStage());
        CHECK(gameController.GameStep(playerId, true) == 2);
        CHECK(gameController.GameStep() == 2);
        CHECK(gameController.WinnersId().size() == 1);
        CHECK(gameController.PlayerPlace(playerId, true) == 1);
        results = gameController.getStepResults(2);
        CHECK(results.size() == 1);
        playerResult = results.back();
        CHECK(playerResult.processId == playerId);
        CHECK(playerResult.step == 2);
        CHECK(playerResult.gameValueList == secretValue);
        CHECK(playerResult.bulls == 4);
        CHECK(playerResult.cows == 0);
        CHECK(playerResult.finished);


        gameController.InitGame();
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::WAIT_A_NUMBER);
        CHECK(gameController.GameStep(playerId, true) == 0);
        CHECK(gameController.GameStep() == 0);
        error = gameController.StartGame(secretValue);
        CHECK( error == TGameController::TError::OK );
        CHECK( secretValue == gameController.SecretValue() );

        error = gameController.PlayerGiveUp(playerId);
        CHECK( error == TGameController::TError::OK );
        error = gameController.PlayerGiveUp(0);
        CHECK( error == TGameController::TError::PLAYER_NOT_FOUND );
        error = gameController.PlayerGiveUp(playerId);
        CHECK( error == TGameController::TError::PLAYER_ALREADY_FINISH );
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED);
        CHECK(gameController.GameStep(playerId, true) == 0);
        CHECK(gameController.GameStep() == 0);
    }

    SECTION("3.2. Check game controller with one computer")
    {
        TGameController gameController;

        gameController.InitGame();

        uint32_t computerId = 1;
        auto error = gameController.addComputerProcess(computerId, MODEL_COMPONENTS::TGameBrain::BEST);
        CHECK( error == TGameController::TError::OK );
        error = gameController.addComputerProcess(computerId, MODEL_COMPONENTS::TGameBrain::BEST);
        CHECK( error == TGameController::TError::COMPUTER_ALREADY_EXISTS );

        std::vector< uint8_t > secretValue{0,1,2,3};
        error = gameController.StartGame(secretValue);
        CHECK( error == TGameController::TError::OK );
        CHECK( secretValue == gameController.SecretValue() );
        CHECK(gameController.PlayerPlace(computerId, false) == 0);

        error = gameController.FinishStep();
        CHECK( error == TGameController::TError::OK );
        CHECK(!gameController.isStepInTransitionStage());
        CHECK(gameController.GameStep(computerId, false) == 1);
        CHECK(gameController.GameStep(computerId, true) == 0);
        CHECK(gameController.GameStep() == 1);
        CHECK(gameController.UnsteppedPlayers() == 0);

        CHECK(gameController.getStepResults(0).size() == 0);
        auto results = gameController.getStepResults(1);
        CHECK(results.size() == 1);
        auto playerResult = results.back();
        CHECK(playerResult.processId == computerId);
        CHECK(!playerResult.player);
        CHECK(playerResult.step == 1);


        gameController.FinishGame();
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED);
        uint32_t lastStep = gameController.GameStep();
        CHECK(gameController.PlayerPlace(computerId, false) == 1);
        results = gameController.getStepResults(lastStep);
        CHECK(results.size() == 1);
        playerResult = results.back();
        CHECK(playerResult.processId == computerId);
        CHECK(playerResult.step == lastStep);
        CHECK(playerResult.bulls == 4);
        CHECK(playerResult.cows == 0);
        CHECK(playerResult.finished);
    }

    SECTION("3.3. Check game controller with multiplie players")
    {
        TGameController gameController;

        gameController.InitGame();

        uint32_t playerId1 = 1;
        auto error = gameController.addPlayerProcess(playerId1);
        CHECK( error == TGameController::TError::OK );
        uint32_t playerId2 = 2;
        error = gameController.addPlayerProcess(playerId2);
        CHECK( error == TGameController::TError::OK );

        std::vector< uint8_t > secretValue{0,1,2,3};
        error = gameController.StartGame(secretValue);
        CHECK( error == TGameController::TError::OK );
        CHECK( secretValue == gameController.SecretValue() );
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
        CHECK(gameController.GameStep(playerId1, true) == 0);
        CHECK(gameController.GameStep(playerId2, true) == 0);
        CHECK(gameController.PlayerPlace(playerId1, true) == 0);
        CHECK(gameController.PlayerPlace(playerId2, true) == 0);
        CHECK(gameController.GameStep() == 0);

        std::vector< uint8_t > firstValue1{4,5,6,7};
        error = gameController.DoPlayerStep(playerId1, firstValue1);
        CHECK( error == TGameController::TError::OK );
        error = gameController.DoPlayerStep(playerId1, firstValue1);
        CHECK( error == TGameController::TError::PLAYER_ALREADY_STEP );
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
        CHECK(gameController.isStepInTransitionStage());
        CHECK(gameController.GameStep(playerId1, true) == 1);
        CHECK(gameController.GameStep(playerId2, true) == 0);
        CHECK(gameController.GameStep() == 0);
        CHECK(gameController.WinnersId().size() == 0);
        CHECK(gameController.UnsteppedPlayers() == 1);


        std::vector< uint8_t > firstValue2{5,6,7,8};
        error = gameController.DoPlayerStep(playerId2, firstValue2);
        CHECK( error == TGameController::TError::OK );
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
        CHECK(!gameController.isStepInTransitionStage());
        CHECK(gameController.GameStep(playerId1, true) == 1);
        CHECK(gameController.GameStep(playerId2, true) == 1);
        CHECK(gameController.GameStep() == 1);
        CHECK(gameController.WinnersId().size() == 0);

        auto results = gameController.getStepResults(1);
        CHECK(results.size() == 2);

        auto res1_it = std::find_if(results.begin(), results.end(), [playerId1](auto const & result ){ return result.processId == playerId1; });
        REQUIRE(res1_it != results.end());
        CHECK(res1_it->processId == playerId1);
        CHECK(res1_it->player);
        CHECK(res1_it->step == 1);
        CHECK(res1_it->gameValueList == firstValue1);
        CHECK(res1_it->bulls == 0);
        CHECK(res1_it->cows == 0);
        CHECK(!res1_it->finished);

        auto res2_it = std::find_if(results.begin(), results.end(), [playerId2](auto const & result ){ return result.processId == playerId2; });
        REQUIRE(res2_it != results.end());
        CHECK(res2_it->processId == playerId2);
        CHECK(res2_it->player);
        CHECK(res2_it->step == 1);
        CHECK(res2_it->gameValueList == firstValue2);
        CHECK(res2_it->bulls == 0);
        CHECK(res2_it->cows == 0);
        CHECK(!res2_it->finished);

        error = gameController.DoPlayerStep(playerId1, secretValue);
        CHECK( error == TGameController::TError::OK );
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
        CHECK(gameController.isStepInTransitionStage());
        CHECK(gameController.GameStep(playerId1, true) == 2);
        CHECK(gameController.GameStep(playerId2, true) == 1);
        CHECK(gameController.GameStep() == 1);
        CHECK(gameController.WinnersId().size() == 0);
        CHECK(gameController.UnsteppedPlayers() == 1);

        std::vector< uint8_t > secondValue2{6,7,8,9};
        error = gameController.DoPlayerStep(playerId2, secondValue2);
        CHECK( error == TGameController::TError::OK );
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS_WINNER_DEFINED);
        CHECK(!gameController.isStepInTransitionStage());
        CHECK(gameController.GameStep(playerId1,true) == 2);
        CHECK(gameController.GameStep(playerId2,true) == 2);
        CHECK(gameController.PlayerPlace(playerId1, true) == 1);
        CHECK(gameController.PlayerPlace(playerId2, true) == 0);
        CHECK(gameController.GameStep() == 2);
        CHECK(gameController.WinnersId().size() == 1);

        results = gameController.getStepResults(2);
        CHECK(results.size() == 2);

        res1_it = std::find_if(results.begin(), results.end(), [playerId1](auto const & result ){ return result.processId == playerId1; });
        REQUIRE(res1_it != results.end());
        CHECK(res1_it->processId == playerId1);
        CHECK(res1_it->player);
        CHECK(res1_it->step == 2);
        CHECK(res1_it->gameValueList == secretValue);
        CHECK(res1_it->bulls == 4);
        CHECK(res1_it->cows == 0);
        CHECK(res1_it->finished);

        res2_it = std::find_if(results.begin(), results.end(), [playerId2](auto const & result ){ return result.processId == playerId2; });
        REQUIRE(res2_it != results.end());
        CHECK(res2_it->processId == playerId2);
        CHECK(res2_it->player);
        CHECK(res2_it->step == 2);
        CHECK(res2_it->gameValueList == secondValue2);
        CHECK(res2_it->bulls == 0);
        CHECK(res2_it->cows == 0);
        CHECK(!res2_it->finished);

        error = gameController.PlayerGiveUp(playerId2);
        CHECK( error == TGameController::TError::OK );
        CHECK(gameController.PlayerPlace(playerId1, true) == 1);
        CHECK(gameController.PlayerPlace(playerId2, true) == 2);
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED);
        CHECK(gameController.GameStep(playerId2,true) == 2);
        CHECK(gameController.GameStep() == 2);
    }

    SECTION("3.4. Check game controller with players and computers")
    {
        TGameController gameController;

        gameController.InitGame();

        uint32_t playerId1 = 1;
        auto error = gameController.addPlayerProcess(playerId1);
        CHECK( error == TGameController::TError::OK );
        uint32_t playerId2 = 2;
        error = gameController.addPlayerProcess(playerId2);
        CHECK( error == TGameController::TError::OK );
        uint32_t computerId = 3;
        error = gameController.addComputerProcess(computerId, MODEL_COMPONENTS::TGameBrain::BEST);
        CHECK( error == TGameController::TError::OK );

        std::vector< uint8_t > secretValue{0,1,2,3};
        error = gameController.StartGame(secretValue);
        CHECK( error == TGameController::TError::OK );
        CHECK( secretValue == gameController.SecretValue() );

        std::vector< uint8_t > firstValue{4,5,6,7};
        error = gameController.DoPlayerStep(playerId1, firstValue);
        CHECK( error == TGameController::TError::OK );
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
        CHECK(gameController.isStepInTransitionStage());
        CHECK(gameController.GameStep() == 0);
        CHECK(gameController.WinnersId().size() == 0);
        CHECK(gameController.UnsteppedPlayers() == 1);


        error = gameController.PlayerGiveUp(playerId2);
        CHECK( error == TGameController::TError::OK );
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::IN_PROGRESS);
        CHECK(!gameController.isStepInTransitionStage());
        CHECK(gameController.GameStep() == 1);
        CHECK(gameController.PlayerPlace(playerId1, true) == 0);
        CHECK(gameController.PlayerPlace(playerId2, true) == 3);
        CHECK(gameController.UnsteppedPlayers(0) == 0);

        auto results = gameController.getStepResults(1);
        CHECK(results.size() == 2);

        auto res1_it = std::find_if(results.begin(), results.end(), [playerId1](auto const & result ){ return result.processId == playerId1; });
        REQUIRE(res1_it != results.end());
        CHECK(res1_it->processId == playerId1);
        CHECK(res1_it->step == 1);
        CHECK(res1_it->gameValueList == firstValue);
        CHECK(res1_it->bulls == 0);
        CHECK(res1_it->cows == 0);
        CHECK(!res1_it->finished);

        auto res2_it = std::find_if(results.begin(), results.end(), [playerId2](auto const & result ){ return result.processId == playerId2; });
        REQUIRE(res2_it == results.end());


        auto res3_it = std::find_if(results.begin(), results.end(), [computerId](auto const & result ){ return result.processId == computerId; });
        REQUIRE(res3_it != results.end());
        CHECK(res3_it->processId == computerId);
        CHECK(res3_it->step == 1);

        gameController.FinishGame();
        CHECK(gameController.GameStage() == MODEL_COMPONENTS::TGameStage::FINISHED);
        CHECK(gameController.PlayerPlace(playerId1, true) == 3);
        CHECK(gameController.PlayerPlace(playerId2, true) == 2);
        CHECK(gameController.PlayerPlace(playerId2, false) == 0);

        uint32_t lastStep = gameController.GameStep();
        results = gameController.getStepResults(lastStep);
        CHECK(results.size() == 1);
        auto compResult = results.back();
        CHECK(compResult.processId == computerId);
        CHECK(!compResult.player);
        CHECK(compResult.step == lastStep);
        CHECK(compResult.bulls == 4);
        CHECK(compResult.cows == 0);
        CHECK(compResult.finished);
    }
}
