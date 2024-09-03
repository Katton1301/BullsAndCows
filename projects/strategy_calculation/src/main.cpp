#include <rules/standart_game_value_node.hpp>
#include <set>
#include <random>
#include <algorithm>
#include <fstream>


std::map<std::vector< uint8_t >, double> loadNumbers()
{
    std::ifstream calcedNumbers_in;
    calcedNumbers_in.open("calces.txt");
    std::map<std::vector< uint8_t >, double> values;
    uint32_t numberValues;
    calcedNumbers_in >> numberValues;
    for(uint32_t i = 0; i < numberValues; ++i)
    {
        uint32_t intValue;
        std::vector< uint8_t > value;
        double isteps;
        for(int j = 0; j < 4; ++j)
        {
            calcedNumbers_in >> intValue;
            value.push_back(intValue);
        }
        calcedNumbers_in >> isteps;
        values.emplace(value, isteps);
    }
    calcedNumbers_in.close();
    return values;
}

void saveNumber(std::vector< uint8_t > number, double steps )
{
    auto values = loadNumbers();
    if(!values.contains(number))
    {
        values.emplace(number, steps);
    }

    std::cout << "save: "
            << static_cast<uint32_t>(number[0])
            << static_cast<uint32_t>(number[1])
            << static_cast<uint32_t>(number[2])
            << static_cast<uint32_t>(number[3])
            << " " << steps << std::endl;

    std::ofstream calcedNumbers_out;
    calcedNumbers_out.open("calces.txt");
    calcedNumbers_out << values.size() << std::endl;
    for(auto const & item : values)
    {
        for(auto intValue : item.first)
        {
            calcedNumbers_out << static_cast<uint32_t>(intValue) << " ";
        }
        calcedNumbers_out << item.second << std::endl;
    }
    calcedNumbers_out.close();
}

std::map< std::pair<uint32_t, uint32_t>, std::vector<std::vector< uint8_t >> > calculateBCForValue
    (std::vector< uint8_t > predictedValue, std::vector<std::vector< uint8_t >> & values)
{
    std::map< std::pair<uint32_t, uint32_t>, std::vector<std::vector< uint8_t >> > valuesByBC;
    while(values.size() > 0)
    {
        auto BC = TStandartRules::Instance()->calculateBullsAndCows( predictedValue, values.back() );
        if(!valuesByBC.contains(BC))
        {
            valuesByBC.emplace(BC, std::vector<std::vector< uint8_t >>() );
        }
        valuesByBC.at(BC).push_back(values.back());
        values.pop_back();
    }
    return valuesByBC;
}

std::vector<TValueNode *> handleValues( std::vector<std::vector< uint8_t >> const & values, int depth)
{
    std::map< std::vector< uint8_t >, std::map< std::pair<uint32_t, uint32_t>, std::vector<std::vector< uint8_t >> > > valuesByBC;
    for(auto const & predictedValue : values)
    {
        valuesByBC.emplace(predictedValue, std::map< std::pair<uint32_t, uint32_t>, std::vector<std::vector< uint8_t >> >());
        for( auto const & trueValue : values )
        {
            auto BC = TStandartRules::Instance()->calculateBullsAndCows( predictedValue, trueValue );
            if(!valuesByBC.at(predictedValue).contains(BC))
            {
                valuesByBC.at(predictedValue).emplace(BC, std::vector<std::vector< uint8_t >>());
            }
            valuesByBC.at(predictedValue).at(BC).push_back(trueValue);
        }
    }

    double bestStepCount = 100000000;
    std::vector<TValueNode *> bestNodes;

    if(depth == 2)
    {
        auto calcedValues = loadNumbers();
        double bestCalcedStep = 100000000;
        std::vector< uint8_t > bestCalcedValue;
        for(auto const & calcedValue : calcedValues)
        {

            if(bestCalcedValue.size() == 0)
            {
                bestCalcedStep = calcedValue.second;
                bestCalcedValue = calcedValue.first;
            }
            else
            {
                std::vector< uint8_t > deletedValue = calcedValue.first;
                if( bestCalcedStep <= calcedValue.second )
                {
                    bestCalcedStep = calcedValue.second;
                    deletedValue = bestCalcedValue;
                    bestCalcedValue = calcedValue.first;
                }
                auto it = std::find_if(
                            valuesByBC.begin(),
                            valuesByBC.end(),
                            [deletedValue](auto const & valuesAndBC)
                            {
                                return valuesAndBC.first == deletedValue;
                            }
                        );
                if(it != valuesByBC.end())
                {
                    valuesByBC.erase(it);
                }
            }
        }
    }

    int n = 0;
    for(auto const & valuesAndBC : valuesByBC)
    {
        TValueNode *middleNode = new TValueNode( valuesAndBC.first, 1, values.size(), depth );
        bool highDepth = false;

        for(auto const & value : valuesAndBC.second)
        {
            if(value.second.size() > 1)
            {
                if(depth > 6)
                {
                    highDepth = true;
                    break;
                }
                for( auto & child : handleValues(value.second, depth + 1))
                {
                    child->recalcSteps();
                    middleNode->addChild(value.first.first, value.first.second, child);
                }
            }
            else if(value.first.first < 4)
            {
                TValueNode *childNode = new TValueNode( value.second.front(), 1, 1, depth + 1 );
                middleNode->addChild(value.first.first, value.first.second, childNode);
            }
            else
            {
                TValueNode *childNode = new TValueNode( valuesAndBC.first, 0, 1, depth + 1 );
                middleNode->addChild(value.first.first, value.first.second, childNode);
            }
        }
        middleNode->recalcSteps();

        if(depth == 2)
        {
            saveNumber(valuesAndBC.first, middleNode->Steps());
        }

        if(bestNodes.empty() || (bestStepCount >= middleNode->Steps() && !highDepth))
        {
            if(bestStepCount > middleNode->Steps())
            {
                bestStepCount = middleNode->Steps();
                for(auto & child : bestNodes)
                {
                    delete child;
                }
                bestNodes.clear();
            }
            bestNodes.push_back(middleNode);
        }
        else
        {
            delete middleNode;
        }
    }
    /* for minimize json file */
    if(bestNodes.size() > 0)
    {
        uint32_t chosen_offset = rand() % bestNodes.size();
        TValueNode *chosenValue = bestNodes.at(chosen_offset);
        bestNodes.erase(bestNodes.begin() + chosen_offset);
        for(auto & child : bestNodes)
        {
            delete child;
        }
        bestNodes.clear();
        bestNodes.push_back(chosenValue);
    }
    /*-----------------------*/
    return bestNodes;
}

int main ( )
{
    std::vector< uint8_t > possibleValue(4, 0);
    std::vector<std::vector< uint8_t >> possibleValues;
    int32_t pos = 0;
    while( pos >= 0 )
    {
        if(TStandartRules::Instance()->isValidGameValue(TGameValue(possibleValue)))
        {
            possibleValues.push_back(possibleValue);
        }

        pos = 3;
        ++possibleValue.at(pos);

        while(possibleValue.at(pos) == 10)
        {
            possibleValue.at(pos) = 0;
            --pos;
            if(pos < 0)
            {
                break;
            }
            ++possibleValue.at(pos);
        }
    }
    srand( time(0) );
    auto rng = std::default_random_engine {};
    std::shuffle(possibleValues.begin(), possibleValues.end(), rng);

    std::vector< uint8_t > firstValue{0,1,2,3};
    auto valuesByBC = calculateBCForValue(firstValue, possibleValues);

    bool loadJson = true;
    TValueNode *mainNode = nullptr;
    if(loadJson)
    {
        mainNode = loadNodeFromJson("./", firstValue);
    }
    if(!mainNode)
    {
        mainNode = new TValueNode(firstValue, 1, possibleValues.size(), 1);
    }

    auto randomSecondValue = valuesByBC.at({0,0}).at(rand() % valuesByBC.at({0,0}).size());
    for(auto & valueBC : valuesByBC)
    {
        std::cout << valueBC.first.first << " B " << valueBC.first.second << " C " << valueBC.second.size() << " values" << std::endl;
        if(!mainNode->ContainBullsNCows(valueBC.first.first, valueBC.first.second))
        {
            if(
                (valueBC.first.first == 4 && valueBC.first.second == 0) ||
                (valueBC.first.first == 0 && valueBC.first.second == 3) ||
                (valueBC.first.first == 1 && valueBC.first.second == 2) ||
                (valueBC.first.first == 2 && valueBC.first.second == 1) ||
                (valueBC.first.first == 3 && valueBC.first.second == 0)
            )
            {
                if(valueBC.first.first == 4)
                {

                    TValueNode *secondNode = new TValueNode(firstValue, 0, 1, 2);
                    mainNode->addChild(valueBC.first.first,valueBC.first.second, secondNode);
                }
                else
                {
                    TValueNode *secondNode = new TValueNode(randomSecondValue, 1, valueBC.second.size(), 2);
                    auto secondBCValue = calculateBCForValue(randomSecondValue, valueBC.second);
                    for(auto const & secondBC : secondBCValue)
                    {
                        for( auto & child : handleValues(secondBC.second, 3))
                        {
                            child->recalcSteps();
                            secondNode->addChild(secondBC.first.first,secondBC.first.second, child);
                        }
                    }
                    secondNode->recalcSteps();
                    mainNode->addChild(valueBC.first.first,valueBC.first.second, secondNode);
                }
            }
            else
            {
                for( auto & child : handleValues(valueBC.second, 2))
                {
                    child->recalcSteps();
                    mainNode->addChild(valueBC.first.first,valueBC.first.second, child);
                }
            }
        }
    }
    mainNode->recalcSteps();
    std::cout << mainNode->Steps() << std::endl;
    writeToJson("./", *mainNode);
    std::cout << "calculation succesfull" << std::endl;
    return 0;
}
