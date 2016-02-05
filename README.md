# Personal bus stop

An Arduino Yun sketch using the [Tfl API](https://api-portal.tfl.gov.uk/docs) to get bus times at a stop.

Sample of the API response:

    [4,"1.0",1454670198712]
    [1,"134","/Tottenham Ct R",1454670555000]
    [1,"134","/Tottenham Ct R",1454670514000]
    [1,"214","Moorgate",1454670792000]
    ...


## TODO

1. Toggle switch to query bus times southbound (default) or northbound.
2. Output to LCD or TFT display.
