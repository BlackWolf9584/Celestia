// destination.cpp
//
// Copyright (C) 2001, Chris Laurel <claurel@shatters.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#include <config.h>
#include <algorithm>
#include <celutil/logger.h>
#include <celutil/stringutils.h>
#include <celengine/astro.h>
#include <celengine/parser.h>
#include <celutil/tokenizer.h>
#include "destination.h"

using namespace std;
using celestia::util::GetLogger;

DestinationList* ReadDestinationList(istream& in)
{
    Tokenizer tokenizer(&in);
    Parser parser(&tokenizer);
    auto* destinations = new DestinationList();

    while (tokenizer.nextToken() != Tokenizer::TokenEnd)
    {
        if (tokenizer.getTokenType() != Tokenizer::TokenBeginGroup)
        {
            GetLogger()->error("Error parsing destinations file.\n");
            for_each(destinations->begin(), destinations->end(), [](Destination* dest) { delete dest; });
            delete destinations;
            return nullptr;
        }
        tokenizer.pushBack();

        Value* destValue = parser.readValue();
        if (destValue == nullptr || destValue->getType() != Value::HashType)
        {
            GetLogger()->error("Error parsing destination.\n");
            for_each(destinations->begin(), destinations->end(), [](Destination* dest) { delete dest; });
            delete destinations;
            if (destValue != nullptr)
                delete destValue;
            return nullptr;
        }

        Hash* destParams = destValue->getHash();
        Destination* dest = new Destination();

        if (!destParams->getString("Name", dest->name))
        {
            GetLogger()->warn("Skipping unnamed destination\n");
            delete dest;
        }
        else
        {
            destParams->getString("Target", dest->target);
            destParams->getString("Description", dest->description);
            destParams->getNumber("Distance", dest->distance);

            // Default unit of distance is the light year
            string distanceUnits;
            if (destParams->getString("DistanceUnits", distanceUnits))
            {
                if (!compareIgnoringCase(distanceUnits, "km"))
                    dest->distance = astro::kilometersToLightYears(dest->distance);
                else if (!compareIgnoringCase(distanceUnits, "au"))
                    dest->distance = astro::AUtoLightYears(dest->distance);
            }

            destinations->push_back(dest);
        }

        delete destValue;
    }

    return destinations;
}
