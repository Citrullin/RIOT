# Do the following to calibrate your HX711

1. Start with "HX711_PARAM_OFFSET 0" and "HX711_PARAM_SCALE 1" in hx711_params.h
2. Flash your microcontroller with these values
3. Check the returned values in your terminal and use it for the offset.
4. Flash again with the new offset
5. Place a reference weight on your connected load cell.
6. Get the average value and use HX711_PARAM_SCALE = AVG_VALUE / REFERENCE_WEIGHT
7. You should different reference values. For example 1g, 2g, 5g, 10g, 20g. 
Depending on the accuracy you need.
