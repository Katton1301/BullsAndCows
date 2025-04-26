#include<decision_tree_generator.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <random>
#include <algorithm>
#include <list>

TDecisionTreeGenerator::TValuesList TDecisionTreeGenerator::generateAllPossibleValues()
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

TDecisionTreeGenerator::TBCDistribution TDecisionTreeGenerator::distributeValuesByBullsNCows( TValue const & predictedValue, TValuesList const & values )
{
    TDecisionTreeGenerator::TBCDistribution distributeValues;
    for( auto const & value : values)
    {
        auto BC = TStandartRules::Instance().calculateBullsAndCows(predictedValue, value);
        distributeValues[BC].push_back(value);
    }
    return distributeValues;
}

std::set<TDecisionTreeGenerator::TValue> TDecisionTreeGenerator::splitingAlgorithm(TValuesList const & availableValues, TValuesList const & checkingValues)
{
    std::set<TValue> bestValues;
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
    for (auto const & predictedValue : checkingValues)
    {
        TDecisionTreeGenerator::TBCDistribution bcDist;
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
        if(
            std::any_of(availableValues.begin(), availableValues.end(), [&predictedValue](auto const & value){return value == predictedValue;}) ||
            splitList.front() * 3 < availableValues.size() ||
            splitList.size() > 5
        )
        {
            bestValues.emplace(predictedValue);
        }

        if( bestSplit.size() == 0 || lessSplits(splitList,bestSplit) )
        {
            bestSplit = splitList;
            bestValue = predictedValue;
        }
    }
    if(bestValues.size() == 0)
    {
        return std::set<TValue>{bestValue};
    }
    else
    {
        return bestValues;
    }
}

std::shared_ptr<TValueNode> TDecisionTreeGenerator::chooseBestEquivalentValues( TValuesList const & availableValues, TValuesList const & solvedValues )
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

    if(availableValues.size() > 12)
    {
        std::cout << "optimizing " << differentValuesForNStep.size();
        differentValuesForNStep = splitingAlgorithm(availableValues, TValuesList(differentValuesForNStep.begin(), differentValuesForNStep.end()));
        std::cout << " to " << differentValuesForNStep.size() << std::endl;
    }

    std::shared_ptr<TValueNode> mainNode = nullptr;

    for(auto const & value : differentValuesForNStep)
    {
        auto curNode = std::make_shared<TValueNode>(value, 1, availableValues.size(), depth);
        TDecisionTreeGenerator::TBCDistribution bcDist;
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
        if(LogLevel() > 1)
        {
            std::cout
                    << "calc "
                    << TStandartRules::Instance().gameValueToString(value)
                    << " depth "
                    << depth
                    << " "
                    << curNode->Steps()
                    << std::endl;
        }
        if(!mainNode || mainNode->Steps() > curNode->Steps())
        {
            mainNode = curNode;
        }
    }
    return mainNode;
}

void TDecisionTreeGenerator::generatePermutations(std::vector<uint32_t>& current, std::vector<bool>& used, std::vector<std::vector<uint32_t>>& result, uint32_t n)
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

std::vector<std::vector<uint32_t>> TDecisionTreeGenerator::generateAllPositions(uint32_t n)
{
    std::vector<std::vector<uint32_t>> result;
    if (n <= 0) return result;

    std::vector<uint32_t> current;
    std::vector<bool> used(n, false);
    generatePermutations(current, used, result, n);

    return result;
}

std::vector<TDecisionTreeGenerator::TValue> TDecisionTreeGenerator::generateEquivalentValues(TValuesList const & solvedValues)
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

std::vector<std::vector<TDecisionTreeGenerator::TValue>> TDecisionTreeGenerator::generateAllFirstNEquivalentValues(uint32_t N)
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
    if(LogLevel() > 1)
    {
        std::cout << "For " << N << " steps needed " << valuesNSteps.size() << " different values sets" << std::endl;
    }
    return std::vector<TValuesList>(valuesNSteps.begin(), valuesNSteps.end());;
}


std::shared_ptr<TValueNode> TDecisionTreeGenerator::customMinimax(TValuesList const & values, int depth)
{
    std::vector<std::pair<TValue, TDecisionTreeGenerator::TBCDistribution>> distributionForValues;
    for (auto const & predictedValue : values)
    {
        TDecisionTreeGenerator::TBCDistribution bcDist;
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

void TDecisionTreeGenerator::start()
{
    m_values3Steps = generateAllFirstNEquivalentValues(3);
    auto possibleValues = generateAllPossibleValues();
    TValue firstValue{0, 1, 2, 3};
    auto distributionValuesByBC = distributeValuesByBullsNCows(firstValue, possibleValues);
    bool loadJson = true;
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
