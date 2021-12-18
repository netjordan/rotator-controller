char azimuth_to_direction(int azimuth_in)
{
    if (azimuth_in > 348)
        return {"N"};
    if (azimuth_in > 326)
        return {"NNW"};
    if (azimuth_in > 303)
        return {"NW"};
    if (azimuth_in > 281)
        return {"WNW"};
    if (azimuth_in > 258)
        return {"W"};
    if (azimuth_in > 236)
        return {"WSW"};
    if (azimuth_in > 213)
        return {"SW"};
    if (azimuth_in > 191)
        return {"SSW"};
    if (azimuth_in > 168)
        return {"S"};
    if (azimuth_in > 146)
        return {"SSE"};
    if (azimuth_in > 123)
        return {"SE"};
    if (azimuth_in > 101)
        return {"ESE"};
    if (azimuth_in > 78)
        return {"E"};
    if (azimuth_in > 56)
        return {"ENE"};
    if (azimuth_in > 33)
        return {"NE"};
    if (azimuth_in > 11)
        return {"NNE"};

    return {"N"};
}