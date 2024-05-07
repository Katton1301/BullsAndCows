#include <rules/standart_game_value_node.hpp>
#include <set>
#include <random>
#include <algorithm>

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

std::string calculateHashString( std::map< std::pair<uint32_t, uint32_t>, std::vector<std::vector< uint8_t >> > const & valuesByBC )
{
    std::string hashStr(42, '0');
    static std::map< std::pair<uint32_t, uint32_t>, uint32_t> indexByBC
    {
        {{0,0}, 0},
        {{0,1}, 1},
        {{1,0}, 2},
        {{0,2}, 3},
        {{1,1}, 4},
        {{2,0}, 5},
        {{0,3}, 6},
        {{1,2}, 7},
        {{2,1}, 8},
        {{3,0}, 9},
        {{0,4}, 10},
        {{1,3}, 11},
        {{2,2}, 12},
        {{4,0}, 13},
    };
    for(auto const & valueBC : valuesByBC)
    {
        auto bc_index = indexByBC.at(valueBC.first);
        uint32_t valuesCount = valueBC.second.size();
        hashStr.at(bc_index * 3) = '0' + (valuesCount / 100);
        valuesCount %= 100;
        hashStr.at(bc_index * 3 + 1) = '0' + (valuesCount / 10);
        hashStr.at(bc_index * 3 + 2) = '0' + (valuesCount % 10);
    }
    return hashStr;
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
    std::set< std::string > usedHashes;
    for(auto const & valuesAndBC : valuesByBC)
    {
        std::string hashStr = calculateHashString(valuesAndBC.second);
        if(depth > 2 || values.size() < 1000 || !usedHashes.contains(hashStr))
        {
            usedHashes.emplace(hashStr);
            double stepsSumm = 0.0;
            int BCVariations = 0;
            TValueNode *middleNode = new TValueNode( valuesAndBC.first, 1, values.size(), depth );
            bool highDepth = false;

            for(auto const & value : valuesAndBC.second)
            {
                ++BCVariations;
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
                    stepsSumm += middleNode->ChildsAt(value.first.first, value.first.second).front()->Steps();
                }
                else if(value.first.first < 4)
                {
                    TValueNode *childNode = new TValueNode( value.second.front(), 1, 1, depth + 1 );
                    middleNode->addChild(value.first.first, value.first.second, childNode);
                    ++stepsSumm;
                }
                else
                {
                    TValueNode *childNode = new TValueNode( valuesAndBC.first, 0, 1, depth + 1 );
                    middleNode->addChild(value.first.first, value.first.second, childNode);
                }
            }
            if(bestNodes.empty() || (bestStepCount >= stepsSumm / BCVariations && !highDepth))
            {
                if(bestStepCount > stepsSumm / BCVariations)
                {
                    bestStepCount = stepsSumm / BCVariations;
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
                (valueBC.first.first == 0 && valueBC.first.second == 0) ||
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
