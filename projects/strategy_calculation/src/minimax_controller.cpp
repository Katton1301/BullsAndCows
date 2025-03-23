#include<minimax_controller.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <random>

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

std::shared_ptr<TValueNode> TMinimaxController::customMinimax(TValuesList const & values, int depth)
{
    std::map<TValue, TMinimaxController::TBCDistribution> distributionForValues;
    for (auto const & predictedValue : values)
    {
        for (auto const & trueValue : values)
        {
            auto BC = TStandartRules::Instance().calculateBullsAndCows(predictedValue, trueValue);
            distributionForValues[predictedValue][BC].push_back(trueValue);
        }
    }

    double bestStepCount = std::numeric_limits<uint32_t>::max();
    std::vector<std::shared_ptr<TValueNode>> nodesList;

    if(depth > 7)
    {
        return std::make_shared<TValueNode>(TValue(4,10), 1, bestStepCount, depth + 1);
    }

    if (depth == m_cashed_attempt)
    {
        double bestCalcedStep = std::numeric_limits<uint32_t>::max();;
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

        for (auto const & [bc, value] : distribution)
        {
            if (value.size() > 1)
            {
                auto child = customMinimax(value, depth + 1);
                child->recalcSteps();
                middleNode->addChild(bc.first, bc.second, std::move(child));
            }
            else if (bc.first < 4)
            {
                auto childNode = std::make_shared<TValueNode>(value.front(), 1, 1, depth + 1);
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
    std::vector<uint8_t> firstValue{0, 1, 2, 3};
    auto possibleValues = generateAllPossibleValues();
    auto distributionValuesByBC = distributeValuesByBullsNCows(firstValue, possibleValues);

    bool loadJson = true;
    std::shared_ptr<TValueNode> mainNode;
    if (loadJson)
    {
        mainNode = JSON_TOOLS::loadNodeFromJson("./bc.json", firstValue);
    }
    if (!mainNode)
    {
        mainNode = std::make_shared<TValueNode>(firstValue, 1, possibleValues.size(), 1);
    }
    possibleValues.clear();

    for (auto & [bc, valuesSet] : distributionValuesByBC)
    {
        if(LogLevel() > 0)
        {
            std::cout << bc.first << " B " << bc.second << " C " << valuesSet.size() << " values" << std::endl;
        }
        if (!mainNode->ContainBullsNCows(bc.first, bc.second))
        {
            if (
                (bc.first == 4 && bc.second == 0) ||
                (bc.first == 0 && bc.second == 3) ||
                (bc.first == 1 && bc.second == 2) ||
                (bc.first == 2 && bc.second == 1) ||
                (bc.first == 3 && bc.second == 0)
            )
            {
                if (bc.first == 4)
                {
                    mainNode->addChild(bc.first, bc.second, std::make_shared<TValueNode>(firstValue, 0, 1, 2));
                }
                else
                {
                    TValue randomSecondValue;
                    if(bc.first == 1 && bc.second == 2)
                    {
                        randomSecondValue = distributionValuesByBC[{1, 1}][rand() % distributionValuesByBC[{1, 1}].size()];
                    }
                    else
                    {
                        randomSecondValue = distributionValuesByBC[{0, 0}][rand() % distributionValuesByBC[{0, 0}].size()];
                    }
                    randomSecondValue = distributionValuesByBC[{2, 1}][rand() % distributionValuesByBC[{2, 1}].size()];
                    auto secondNode = std::make_shared<TValueNode>(randomSecondValue, 1, valuesSet.size(), 2);
                    for (auto & [bc2, valuesSet2] : distributeValuesByBullsNCows(randomSecondValue, valuesSet))
                    {
                        auto child = customMinimax(valuesSet2, 3);
                        child->recalcSteps();
                        secondNode->addChild(bc2.first, bc2.second, std::move(child));
                    }
                    secondNode->recalcSteps();
                    mainNode->addChild(bc.first, bc.second, std::move(secondNode));
                }
            }
            else
            {
                auto child = customMinimax(valuesSet, 2);
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























