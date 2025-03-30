#include<minimax_controller.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <random>
#include <algorithm>
#include <list>

TMinimaxController::TMinimaxController(int32_t _attempt)
{
    std::stringstream stream;
    stream << "calcs_" << _attempt << ".txt";
    m_filename = stream.str();
    loadCashNumbers();
}

TMinimaxController::TValuesList TMinimaxController::generateAllPossibleValues()
{
    TValuesList possibleValues;
    TValue possibleValue(4, 0);
    int32_t pos = 0;
    while (pos >= 0)
    {
        if (TStandartRules::Instance().isValidGameValue(TGameValue(possibleValue)))
        {
            possibleValues.push_back(possibleValue);
        }

        pos = 3;
        ++possibleValue[pos];

        while (possibleValue[pos] == 10)
        {
            possibleValue[pos] = 0;
            --pos;
            if (pos < 0) break;
            ++possibleValue[pos];
        }
    }
    return possibleValues;
}

TMinimaxController::TBCDistribution TMinimaxController::distributeValuesByBullsNCows( TValue const & predictedValue, TValuesList const & values )
{
    TMinimaxController::TBCDistribution distributeValues;
    for( auto const & value : values)
    {
        auto BC = TStandartRules::Instance().calculateBullsAndCows(predictedValue, value);
        distributeValues[BC].push_back(value);
    }
    return distributeValues;
}

void TMinimaxController::loadCashNumbers()
{
    m_numbersCash.clear();
    std::ifstream calced_numbers_in(m_filename, std::ios_base::in);
    uint32_t intValue;
    while (calced_numbers_in >> intValue)
    {
        std::vector<uint8_t> value(4 , 0);
        double isteps;
        int pos = 3;
        while(pos >= 0)
        {
            value[pos] = intValue % 10;
            intValue/= 10;
            --pos;
        }
        calced_numbers_in >> isteps;
        m_numbersCash[value] = isteps;
    }
}

void TMinimaxController::saveNumber(TValue const & number, double steps)
{
    if (!m_numbersCash.contains(number))
    {
        m_numbersCash.emplace(number, steps);
    }
    else
    {
        if(m_numbersCash.at(number) > steps)
        {
            m_numbersCash[number] = steps;
        }
    }
    if(LogLevel() > 0)
    {
        std::cout << "save: ";
        copy(number.begin(),number.end(), std::ostream_iterator<uint32_t>(std::cout, ""));
        std::cout << " " << steps << std::endl;
    }

    std::ofstream calced_numbers_out(m_filename, std::ios::app);
    copy(number.begin(),number.end(), std::ostream_iterator<uint32_t>(calced_numbers_out, ""));
    calced_numbers_out << " " << steps << std::endl;
    calced_numbers_out.close();
}

TMinimaxController::TValue TMinimaxController::splitingAlgorithm(TValuesList const & availableValues, TValuesList const & checkingValues)
{
    std::vector<uint32_t> bestSplit;
    TValue bestValue;
    auto lessSplits = [](std::vector<uint32_t> const & lSplit, std::vector<uint32_t> rSplit)->bool
    {
      for(uint32_t i = 0; i < lSplit.size() && i < rSplit.size(); ++i)
      {
          if(lSplit[i] < rSplit[i])
          {
              return true;
          }
          if(lSplit[i] > rSplit[i])
          {
              return false;
          }
      }
      return lSplit.size() < rSplit.size();
    };

    bool isBestValueMayWin = false;
    for (auto const & predictedValue : checkingValues)
    {
        TMinimaxController::TBCDistribution bcDist;
        std::vector<uint32_t> splitList;
        for (auto const & trueValue : availableValues)
        {
            auto BC = TStandartRules::Instance().calculateBullsAndCows(predictedValue, trueValue);
            bcDist[BC].push_back(trueValue);
        }
        for( auto const & [bc,valueDist] : bcDist)
        {
            splitList.push_back(valueDist.size());
        }
        std::sort(splitList.begin(), splitList.end(), std::greater<>());
        std::cout
                << "split "
                << static_cast<uint32_t>(predictedValue[0])
                << static_cast<uint32_t>(predictedValue[1])
                << static_cast<uint32_t>(predictedValue[2])
                << static_cast<uint32_t>(predictedValue[3])
                << " ";
        for(auto split : splitList)
        {
            std::cout << split << " ";
        }
        std::cout << std::endl;
        if(
            (
                bestSplit.size() == 0 ||
                lessSplits(splitList,bestSplit) ||
                (
                    splitList == bestSplit &&
                    !isBestValueMayWin &&
                    std::any_of(availableValues.begin(), availableValues.end(), [&predictedValue](auto const & value){return value == predictedValue;})
                )
            ) &&
            (
                splitList.front() > 1 ||
                std::any_of(availableValues.begin(), availableValues.end(), [&predictedValue](auto const & value){return value == predictedValue;})
            )
        )
        {
            isBestValueMayWin = std::any_of(availableValues.begin(), availableValues.end(), [&predictedValue](auto const & value){return value == predictedValue;});
            bestSplit = std::move(splitList);
            bestValue = std::move(predictedValue);
        }
    }
    std::cout
            << "choosed "
            << static_cast<uint32_t>(bestValue[0])
            << static_cast<uint32_t>(bestValue[1])
            << static_cast<uint32_t>(bestValue[2])
            << static_cast<uint32_t>(bestValue[3])
            << " ";
    for(auto split : bestSplit)
    {
        std::cout << split << " ";
    }
    std::cout << std::endl;
    return bestValue;
}

std::shared_ptr<TValueNode> TMinimaxController::chooseBestEquivalentValues( TValuesList const & availableValues, TValuesList const & solvedValues )
{
    int depth = solvedValues.size() + 1;
    std::set<TValue> differentValuesForNStep;

    for(auto const & values : Values3Steps())
    {
        uint32_t i = 0;
        for(; i < solvedValues.size(); ++i)
        {
            if(values[i] != solvedValues[i])
            {
                break;
            }
        }
        if(i == solvedValues.size())
        {
            differentValuesForNStep.emplace(values[i]);
        }
    }

    /*
    if(depth > 2)
    {
        differentValuesForNStep = {splitingAlgorithm(availableValues, TValuesList(differentValuesForNStep.begin(), differentValuesForNStep.end()))};
    }
    */
    std::shared_ptr<TValueNode> mainNode = nullptr;

    for(auto const & value : differentValuesForNStep)
    {
        auto curNode = std::make_shared<TValueNode>(value, 1, availableValues.size(), depth);
        TMinimaxController::TBCDistribution bcDist;
        std::vector<uint32_t> splitList;
        for (auto const & trueValue : availableValues)
        {
            auto BC = TStandartRules::Instance().calculateBullsAndCows(value, trueValue);
            bcDist[BC].push_back(trueValue);
        }

        for (auto const & [bc, valueDist] : bcDist)
        {
            if (valueDist.size() > 1)
            {
                if(depth < 3)
                {
                    auto newSolvedValues = solvedValues;
                    newSolvedValues.push_back(value);
                    auto child = chooseBestEquivalentValues(valueDist, newSolvedValues);
                    child->recalcSteps();
                    curNode->addChild(bc.first, bc.second, std::move(child));
                }
                else
                {
                    auto child = customMinimax(valueDist, depth + 1);
                    child->recalcSteps();
                    curNode->addChild(bc.first, bc.second, std::move(child));
                }
            }
            else if (bc.first < 4)
            {
                auto childNode = std::make_shared<TValueNode>(valueDist.front(), 1, 1, depth + 1);
                curNode->addChild(bc.first, bc.second, std::move(childNode));
            }
            else
            {
                auto childNode = std::make_shared<TValueNode>(value, 0, 1, depth + 1);
                curNode->addChild(bc.first, bc.second, std::move(childNode));
            }
        }
        curNode->recalcSteps();
        std::cout
                << "calc "
                << static_cast<uint32_t>(value[0])
                << static_cast<uint32_t>(value[1])
                << static_cast<uint32_t>(value[2])
                << static_cast<uint32_t>(value[3])
                << " depth "
                << depth
                << " "
                << curNode->Steps()
                << std::endl;
        if(!mainNode || mainNode->Steps() > curNode->Steps())
        {
            mainNode = curNode;
        }
    }

    if (depth == m_cashed_attempt)
    {
        saveNumber(mainNode->Value(), mainNode->Steps());
    }
    return mainNode;
}

void TMinimaxController::generatePermutations(std::vector<uint32_t>& current, std::vector<bool>& used, std::vector<std::vector<uint32_t>>& result, uint32_t n)
{
    if (current.size() == n) {
        result.push_back(current);
        return;
    }

    for (uint32_t i = 0; i < n; ++i) {
        if (!used[i]) {
            used[i] = true;
            current.push_back(i);
            generatePermutations(current, used, result, n);
            current.pop_back();
            used[i] = false;
        }
    }
}

std::vector<std::vector<uint32_t>> TMinimaxController::generateAllPositions(uint32_t n)
{
    std::vector<std::vector<uint32_t>> result;
    if (n <= 0) return result;

    std::vector<uint32_t> current;
    std::vector<bool> used(n, false);
    generatePermutations(current, used, result, n);

    return result;
}

std::vector<TMinimaxController::TValue> TMinimaxController::generateEquivalentValues(TValuesList const & solvedValues)
{
    auto possiblePermutations = generateAllPositions(4);
    auto possibleValues = generateAllPossibleValues();
    TValuesList valuesSet(solvedValues.begin(), solvedValues.end());

    std::set<uint8_t> newDigits;
    for(uint8_t digit = 0; digit < 10; ++digit)
    {
        newDigits.emplace(digit);
    }
    for(auto const & value : solvedValues)
    {
        for(auto digit : value)
        {
            if(newDigits.contains(digit))
            {
                newDigits.erase(digit);
            }
        }
    }
    for(auto const & value : possibleValues)
    {
        bool isEquivalentFind = false;
        for(auto const & perm : possiblePermutations)
        {
            TValue trans(10,10);
            TValuesList solvedPermValues;
            bool isCorrectTrans = true;
            for(auto const & solvedValue : solvedValues)
            {
                TValue solvedPermValue(solvedValue.size(),0);
                for(uint32_t i = 0; i < solvedValue.size(); ++i)
                {
                    solvedPermValue[i] = solvedValue[perm[i]];
                    if(trans[solvedValue[i]] != 10 && trans[solvedValue[i]] != solvedPermValue[i])
                    {
                        isCorrectTrans = false;
                        break;
                    }
                    trans[solvedValue[i]] = solvedPermValue[i];
                }
                if(!isCorrectTrans)
                {
                    break;
                }
                solvedPermValues.push_back(solvedPermValue);
            }
            if(!isCorrectTrans)
            {
                continue;
            }
            for(auto const & compareValue : valuesSet)
            {
                TValue permValue(perm.size(),0);
                for(uint32_t i = 0; i < perm.size(); ++i)
                {
                    permValue[i] = compareValue[perm[i]];
                }
                isCorrectTrans = true;
                for(uint32_t i = 0; i < value.size(); ++i)
                {
                    if(trans[value[i]] != permValue[i] && (trans[value[i]] != 10 || !newDigits.contains(permValue[i])))
                    {
                        isCorrectTrans = false;
                        break;
                    }
                }
                if(isCorrectTrans)
                {
                    isEquivalentFind = true;
                    break;
                }
            }
            if(isEquivalentFind)
            {
                break;
            }
        }
        if(!isEquivalentFind)
        {
            valuesSet.push_back(value);
        }
    }
    std::erase_if(
        valuesSet,
        [&solvedValues](auto const & value)
        {
            return std::any_of(solvedValues.begin(),solvedValues.end(), [&value](auto const & solvedValue){return solvedValue == value;});
        }
    );
    return valuesSet;
}

std::vector<std::vector<TMinimaxController::TValue>> TMinimaxController::generateAllFirstNEquivalentValues(uint32_t N)
{
    std::list<TValuesList> valuesNSteps;
    if(N == 0)
    {
        return std::vector<TValuesList>(valuesNSteps.begin(), valuesNSteps.end());
    }
    //optimize first step. all possible values on first step are equivalents
    TValue firstValue{0, 1, 2, 3};
    TValuesList solvedValues{firstValue};
    valuesNSteps.push_back(solvedValues);
    while(valuesNSteps.front().size() < N)
    {
        decltype(valuesNSteps) tmpValuesNSeps;
        while(valuesNSteps.size())
        {
            auto solvedValues = valuesNSteps.front();
            valuesNSteps.pop_front();
            for(auto const & eqValue : generateEquivalentValues(solvedValues))
            {
                auto newSolvedValues = solvedValues;
                newSolvedValues.push_back(eqValue);
                tmpValuesNSeps.push_back(std::move(newSolvedValues));
            }
        }
        valuesNSteps = std::move(tmpValuesNSeps);
    }

    std::cout << "For " << N << " steps needed " << valuesNSteps.size() << " different values sets" << std::endl;
    return std::vector<TValuesList>(valuesNSteps.begin(), valuesNSteps.end());;
}


std::shared_ptr<TValueNode> TMinimaxController::customMinimax(TValuesList const & values, int depth)
{
    std::vector<std::pair<TValue, TMinimaxController::TBCDistribution>> distributionForValues;
    for (auto const & predictedValue : values)
    {
        TMinimaxController::TBCDistribution bcDist;
        for (auto const & trueValue : values)
        {
            auto BC = TStandartRules::Instance().calculateBullsAndCows(predictedValue, trueValue);
            bcDist[BC].push_back(trueValue);
        }
        distributionForValues.push_back(std::make_pair(predictedValue,bcDist));
    }

    double bestStepCount = std::numeric_limits<uint32_t>::max();
    std::vector<std::shared_ptr<TValueNode>> nodesList;

    if(depth > 7) //this way definitely not best
    {
        return std::make_shared<TValueNode>(TValue(4,10), 1, bestStepCount, depth + 1);
    }

    if (depth == m_cashed_attempt)
    {
        double bestCalcedStep = std::numeric_limits<uint32_t>::max();
        std::vector<uint8_t> bestCalcedValue;
        for (auto const & calcedValue : CashNumbers())
        {
            if (bestCalcedValue.empty() || bestCalcedStep > calcedValue.second)
            {
                bestCalcedStep = calcedValue.second;
                bestCalcedValue = calcedValue.first;
            }
        }
    }
    for (auto const & [predValue, distribution] : distributionForValues)
    {

        auto middleNode = std::make_shared<TValueNode>(predValue, 1, values.size(), depth);

        for (auto const & [bc, valueDist] : distribution)
        {
            if (valueDist.size() > 1)
            {
                auto child = customMinimax(valueDist, depth + 1);
                child->recalcSteps();
                middleNode->addChild(bc.first, bc.second, std::move(child));
            }
            else if (bc.first < 4)
            {
                auto childNode = std::make_shared<TValueNode>(valueDist.front(), 1, 1, depth + 1);
                middleNode->addChild(bc.first, bc.second, std::move(childNode));
            }
            else
            {
                auto childNode = std::make_shared<TValueNode>(predValue, 0, 1, depth + 1);
                middleNode->addChild(bc.first, bc.second, std::move(childNode));
            }
        }
        middleNode->recalcSteps();

        if (depth == m_cashed_attempt)
        {
            saveNumber(predValue, middleNode->Steps());
        }

        if (nodesList.empty() || (bestStepCount >= middleNode->Steps()))
        {
            if (bestStepCount > middleNode->Steps()) {
                bestStepCount = middleNode->Steps();
                nodesList.clear();
            }
            nodesList.push_back(std::move(middleNode));
        }
    }

    assert(!nodesList.empty());
    uint32_t chosen_offset = rand() % nodesList.size();
    return nodesList[chosen_offset];
}

void TMinimaxController::start()
{
    m_values3Steps = generateAllFirstNEquivalentValues(3);
    auto possibleValues = generateAllPossibleValues();
    TValue firstValue{0, 1, 2, 3};
    auto distributionValuesByBC = distributeValuesByBullsNCows(firstValue, possibleValues);
    bool loadJson = false;
    std::shared_ptr<TValueNode> mainNode = nullptr;
    if (loadJson)
    {
        mainNode = JSON_TOOLS::loadNodeFromJson("./bc.json", firstValue);
    }
    if (!mainNode)
    {
        mainNode = std::make_shared<TValueNode>(firstValue, 1, possibleValues.size(), 1);
    }

    for (auto & [bc, valuesSet] : distributionValuesByBC)
    {
        if(LogLevel() > 0)
        {
            std::cout << bc.first << " B " << bc.second << " C " << valuesSet.size() << " values" << std::endl;
        }
        if (!mainNode->ContainBullsNCows(bc.first, bc.second))
        {
            if ( (bc.first == 4 && bc.second == 0) )
            {
                mainNode->addChild(bc.first, bc.second, std::make_shared<TValueNode>(firstValue, 0, 1, 2));
            }
            else
            {
                auto child = chooseBestEquivalentValues(valuesSet, TValuesList{firstValue});
                child->recalcSteps();
                mainNode->addChild(bc.first, bc.second, std::move(child));
            }
        }
    }
    mainNode->recalcSteps();

    if(LogLevel() > 0)
    {
        std::cout << mainNode->Steps() << std::endl;
    }
    JSON_TOOLS::writeToJson("./bc.json", mainNode);

    if(LogLevel() >= 0)
    {
        std::cout << "calculation successful" << std::endl;
    }
}























